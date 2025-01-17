#include "http_session.h"
#include <boost/algorithm/string.hpp>
#include <iostream>

HttpSession::HttpSession(tcp::socket&& socket, std::string doc_root, std::shared_ptr<ApiHandler> api_handler)
    : socket_(std::move(socket))
    , doc_root_(std::move(doc_root))
    , api_handler_(std::move(api_handler)) {
}

void HttpSession::start() {
    http::async_read(socket_, buffer_, request_,
        [self = shared_from_this()](beast::error_code ec, std::size_t) {
            if (!ec) {
                self->handle_request();
            }
        });
}

void HttpSession::handle_request() {
    auto const bad_request = [this](beast::string_view why) {
        http::response<http::string_body> res{http::status::bad_request, request_.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(request_.keep_alive());
        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    };

    if (request_.method() != http::verb::get &&
        request_.method() != http::verb::post &&
        request_.method() != http::verb::options) {
        return send_response(bad_request("Unknown HTTP-method"));
    }

    if (request_.method() == http::verb::options) {
        http::response<http::string_body> res{http::status::no_content, request_.version()};
        add_cors_headers(res);
        return send_response(std::move(res));
    }

    if (request_.target().empty() || request_.target()[0] != '/' ||
        request_.target().find("..") != beast::string_view::npos) {
        return send_response(bad_request("Illegal request-target"));
    }

    if (handle_api_request()) {
        return;
    }

    std::string path = doc_root_;
    path.append(request_.target().data(), request_.target().size());
    if (request_.target().back() == '/') {
        path.append("index.html");
    }

    if (fs::exists(path)) {
        send_file(path);
    } else {
        http::response<http::string_body> res{http::status::not_found, request_.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(request_.keep_alive());
        res.body() = "File not found\r\n";
        res.prepare_payload();
        add_cors_headers(res);
        send_response(std::move(res));
    }
}

bool HttpSession::handle_api_request() {
    std::string target(request_.target());
    
    if (!boost::starts_with(target, "/api/")) {
        return false;
    }
    
    http::response<http::string_body> res;
    
    if (target == "/api/temperature/current") {
        res = api_handler_->handleCurrentTemperature();
    }
    else if (boost::starts_with(target, "/api/temperature/history")) {
        std::string type, start, end;
        std::string query = target.substr(target.find('?') + 1);
        std::vector<std::string> params;
        boost::split(params, query, boost::is_any_of("&"));
        
        for (const auto& param : params) {
            std::vector<std::string> kv;
            boost::split(kv, param, boost::is_any_of("="));
            if (kv.size() == 2) {
                if (kv[0] == "type") type = kv[1];
                else if (kv[0] == "start") start = kv[1];
                else if (kv[0] == "end") end = kv[1];
            }
        }
        
        if (type.empty() || start.empty() || end.empty()) {
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"error": "Missing required parameters"})";
        } else {
            res = api_handler_->handleTemperatureHistory(type, start, end);
        }
    }
    else {
        return false;
    }
    
    add_cors_headers(res);
    send_response(std::move(res));
    return true;
}

void HttpSession::send_response(http::response<http::string_body>&& msg) {
    auto sp = std::make_shared<http::response<http::string_body>>(std::move(msg));
    
    http::async_write(socket_, *sp,
        [self = shared_from_this(), sp](beast::error_code ec, std::size_t) {
            if (ec) {
                std::cerr << "Error writing response: " << ec.message() << std::endl;
            }
            self->socket_.shutdown(tcp::socket::shutdown_send, ec);
        });
}

void HttpSession::send_file(const std::string& path) {
    http::response<http::file_body> res;
    res.version(11);
    res.result(http::status::ok);
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    
    auto const ext = fs::path(path).extension().string();
    if (ext == ".html") res.set(http::field::content_type, "text/html");
    else if (ext == ".js") res.set(http::field::content_type, "application/javascript");
    else if (ext == ".css") res.set(http::field::content_type, "text/css");
    else if (ext == ".json") res.set(http::field::content_type, "application/json");
    else res.set(http::field::content_type, "application/octet-stream");
    
    add_cors_headers(res);
    
    beast::error_code ec;
    http::file_body::value_type body;
    body.open(path.c_str(), beast::file_mode::scan, ec);
    
    if (ec) {
        http::response<http::string_body> err_res{http::status::internal_server_error, 11};
        err_res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        err_res.set(http::field::content_type, "text/plain");
        err_res.body() = "Failed to open file\r\n";
        err_res.prepare_payload();
        add_cors_headers(err_res);
        return send_response(std::move(err_res));
    }
    
    auto const size = body.size();
    
    res.body() = std::move(body);
    res.content_length(size);
    res.keep_alive(true);
    
    auto sp = std::make_shared<http::response<http::file_body>>(std::move(res));
    http::async_write(socket_, *sp,
        [self = shared_from_this(), sp](beast::error_code ec, std::size_t) {
            if (ec) {
                std::cerr << "Error writing file: " << ec.message() << std::endl;
            }
            self->socket_.shutdown(tcp::socket::shutdown_send, ec);
        });
} 