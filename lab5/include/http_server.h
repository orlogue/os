#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <string>
#include <memory>
#include "api_handler.h"

namespace beast = boost::beast;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;

class HttpServer {
public:
    HttpServer(const std::string& address, unsigned short port, 
              const std::string& doc_root, std::shared_ptr<DbManager> db_manager);
    
    void start();
    void stop();

private:
    void do_accept(tcp::acceptor& acceptor);
    
    boost::asio::io_context io_context_;
    std::string address_;
    unsigned short port_;
    std::string doc_root_;
    std::shared_ptr<ApiHandler> api_handler_;
}; 