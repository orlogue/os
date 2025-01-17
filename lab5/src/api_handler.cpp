#include "api_handler.h"
#include <boost/beast/http.hpp>
#include <boost/json.hpp>
#include <sstream>
#include <iomanip>

namespace http = boost::beast::http;
namespace json = boost::json;

ApiHandler::ApiHandler(std::shared_ptr<DbManager> dbManager) 
    : db_manager_(dbManager) {}

std::string ApiHandler::getFormattedTime(time_t timestamp) {
    std::stringstream ss;
    ss << std::put_time(std::localtime(&timestamp), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

http::response<http::string_body> ApiHandler::handleCurrentTemperature() {
    http::response<http::string_body> res;
    res.version(11);
    res.set(http::field::content_type, "application/json");
    
    try {
        double temp = db_manager_->getCurrentTemperature();
        json::object obj;
        obj["temperature"] = temp;
        obj["timestamp"] = std::time(nullptr);
        res.body() = json::serialize(obj);
        res.result(http::status::ok);
    } catch (const std::exception& e) {
        json::object obj;
        obj["error"] = e.what();
        res.body() = json::serialize(obj);
        res.result(http::status::internal_server_error);
    }
    
    res.prepare_payload();
    return res;
}

http::response<http::string_body> ApiHandler::handleTemperatureHistory(
    const std::string& type, const std::string& start, const std::string& end) {
    
    http::response<http::string_body> res;
    res.version(11);
    res.set(http::field::content_type, "application/json");
    
    try {
        time_t start_time = std::stoll(start);
        time_t end_time = std::stoll(end);
        
        auto records = db_manager_->getTemperatures(type, start_time, end_time);
        
        json::array data;
        for (const auto& record : records) {
            json::object entry;
            entry["timestamp"] = record.timestamp;
            entry["temperature"] = record.temperature;
            data.push_back(entry);
        }
        
        json::object obj;
        obj["data"] = data;
        res.body() = json::serialize(obj);
        res.result(http::status::ok);
    } catch (const std::exception& e) {
        json::object obj;
        obj["error"] = e.what();
        res.body() = json::serialize(obj);
        res.result(http::status::internal_server_error);
    }
    
    res.prepare_payload();
    return res;
} 