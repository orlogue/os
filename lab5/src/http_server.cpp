#include "http_server.h"
#include "http_session.h"
#include "db_manager.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>
#include <thread>
#include <iostream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace fs = boost::filesystem;
using tcp = boost::asio::ip::tcp;

HttpServer::HttpServer(const std::string& address, unsigned short port, 
                     const std::string& doc_root, std::shared_ptr<DbManager> db_manager)
    : address_(address)
    , port_(port)
    , doc_root_(doc_root)
    , api_handler_(std::make_shared<ApiHandler>(db_manager)) {
}

void HttpServer::start() {
    try {
        tcp::acceptor acceptor(io_context_, tcp::endpoint(boost::asio::ip::make_address(address_), port_));
        
        do_accept(acceptor);
        
        io_context_.run();
    } catch (const std::exception& e) {
        std::cerr << "Error in server: " << e.what() << std::endl;
        throw;
    }
}

void HttpServer::stop() {
    boost::asio::post(io_context_, [this]() {
        io_context_.stop();
    });
}

void HttpServer::do_accept(tcp::acceptor& acceptor) {
    acceptor.async_accept(
        [this, &acceptor](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::make_shared<HttpSession>(
                    std::move(socket),
                    doc_root_,
                    api_handler_
                )->start();
                
                do_accept(acceptor);
            } else if (ec != boost::asio::error::operation_aborted) {
                std::cerr << "Accept error: " << ec.message() << std::endl;
            }
        });
} 