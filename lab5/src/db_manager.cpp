#include "db_manager.h"
#include <stdexcept>
#include <sstream>

DbManager::DbManager(const std::string& path) : dbPath(path), db(nullptr) {
    int rc = sqlite3_open(path.c_str(), &db);
    if (rc) {
        throw std::runtime_error("Can't open database: " + std::string(sqlite3_errmsg(db)));
    }
}

DbManager::~DbManager() {
    if (db) {
        sqlite3_close(db);
    }
}

void DbManager::createTables() {
    const char* sql = R"(
        DROP TABLE IF EXISTS temperatures;
        CREATE TABLE temperatures (
            timestamp INTEGER NOT NULL,
            temperature REAL NOT NULL,
            type TEXT NOT NULL,
            PRIMARY KEY (timestamp, type)
        );
        CREATE INDEX IF NOT EXISTS idx_temperatures_timestamp ON temperatures(timestamp);
        CREATE INDEX IF NOT EXISTS idx_temperatures_type ON temperatures(type);
    )";
    
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::string error = errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error("SQL error: " + error);
    }
}

void DbManager::insertTemperature(time_t timestamp, double temperature, const std::string& type) {
    const char* sql = "INSERT OR REPLACE INTO temperatures (timestamp, temperature, type) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }
    
    sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(timestamp));
    sqlite3_bind_double(stmt, 2, temperature);
    sqlite3_bind_text(stmt, 3, type.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        throw std::runtime_error("Failed to insert temperature: " + std::string(sqlite3_errmsg(db)));
    }

    if (type == "raw") {
        const char* hourly_sql = R"(
            INSERT OR REPLACE INTO temperatures (timestamp, temperature, type)
            SELECT 
                ? - (? % 3600),
                AVG(temperature),
                'hourly'
            FROM temperatures
            WHERE type = 'raw'
                AND timestamp >= ? - (? % 3600)
                AND timestamp < ? - (? % 3600) + 3600
        )";

        rc = sqlite3_prepare_v2(db, hourly_sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare hourly statement: " + std::string(sqlite3_errmsg(db)));
        }

        for (int i = 1; i <= 6; i += 2) {
            sqlite3_bind_int64(stmt, i, static_cast<sqlite3_int64>(timestamp));
            sqlite3_bind_int64(stmt, i + 1, static_cast<sqlite3_int64>(timestamp));
        }

        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        const char* daily_sql = R"(
            INSERT OR REPLACE INTO temperatures (timestamp, temperature, type)
            SELECT 
                ? - (? % 86400),
                AVG(temperature),
                'daily'
            FROM temperatures
            WHERE type = 'hourly'
                AND timestamp >= ? - (? % 86400)
                AND timestamp < ? - (? % 86400) + 86400
        )";

        rc = sqlite3_prepare_v2(db, daily_sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare daily statement: " + std::string(sqlite3_errmsg(db)));
        }

        for (int i = 1; i <= 6; i += 2) {
            sqlite3_bind_int64(stmt, i, static_cast<sqlite3_int64>(timestamp));
            sqlite3_bind_int64(stmt, i + 1, static_cast<sqlite3_int64>(timestamp));
        }

        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

double DbManager::getCurrentTemperature() {
    const char* sql = "SELECT temperature FROM temperatures WHERE type = 'raw' ORDER BY timestamp DESC LIMIT 1";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("No temperature data available");
    }
    
    double temperature = sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);
    
    return temperature;
}

std::vector<TemperatureRecord> DbManager::getTemperatures(const std::string& type, time_t start, time_t end) {
    const char* sql = "SELECT timestamp, temperature FROM temperatures "
                     "WHERE type = ? AND timestamp >= ? AND timestamp <= ? "
                     "ORDER BY timestamp ASC";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }
    
    sqlite3_bind_text(stmt, 1, type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(start));
    sqlite3_bind_int64(stmt, 3, static_cast<sqlite3_int64>(end));
    
    std::vector<TemperatureRecord> records;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TemperatureRecord record;
        record.timestamp = static_cast<time_t>(sqlite3_column_int64(stmt, 0));
        record.temperature = sqlite3_column_double(stmt, 1);
        records.push_back(record);
    }
    
    sqlite3_finalize(stmt);
    return records;
} 