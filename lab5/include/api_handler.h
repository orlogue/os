#pragma once

#include "db_manager.h"
#include <boost/beast/http.hpp>
#include <memory>
#include <string>

namespace http = boost::beast::http;

class ApiHandler {
public:
    explicit ApiHandler(std::shared_ptr<DbManager> dbManager);

    http::response<http::string_body> handleCurrentTemperature();
    http::response<http::string_body> handleTemperatureHistory(const std::string& type, 
                                                             const std::string& start,
                                                             const std::string& end);
    
private:
    std::string getFormattedTime(time_t timestamp);
    std::shared_ptr<DbManager> db_manager_;
}; 