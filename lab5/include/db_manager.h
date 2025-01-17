#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <sqlite3.h>
#include <memory>

struct TemperatureRecord {
    time_t timestamp;
    double temperature;
};

class DbManager {
public:
    explicit DbManager(const std::string& dbPath);
    ~DbManager();

    void createTables();
    void insertTemperature(time_t timestamp, double temperature, const std::string& type);
    double getCurrentTemperature();
    std::vector<TemperatureRecord> getTemperatures(const std::string& type, time_t start, time_t end);

private:
    sqlite3* db;
    std::string dbPath;
}; 