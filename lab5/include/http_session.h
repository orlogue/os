#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/filesystem.hpp>
#include <memory>
#include <string>
#include "api_handler.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace fs = boost::filesystem;
using tcp = boost::asio::ip::tcp;

class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
    HttpSession(tcp::socket&& socket, std::string doc_root, std::shared_ptr<ApiHandler> api_handler);
    void start();

private:
    void handle_request();
    bool handle_api_request();
    void send_file(const std::string& path);
    void send_response(http::response<http::string_body>&& msg);
    
    template<typename Body>
    void add_cors_headers(http::response<Body>& res) {
        res.set(http::field::access_control_allow_origin, "*");
        res.set(http::field::access_control_allow_methods, "GET, POST, OPTIONS");
        res.set(http::field::access_control_allow_headers, "Content-Type");
    }

    tcp::socket socket_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> request_;
    std::string doc_root_;
    std::shared_ptr<ApiHandler> api_handler_;
}; 