#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <ctime>
#include <filesystem>
#include <numeric>
#include <map>
#include <sstream>
#include "serial_port.h"

namespace fs = std::filesystem;

struct TempReading {
    time_t timestamp;
    double temperature;
};

class TemperatureMonitor {
private:
    fs::path logs_dir;
    fs::path raw_log_path;
    fs::path hourly_log_path;
    fs::path daily_log_path;
    std::unique_ptr<SerialPort> serialPort;
    std::vector<TempReading> hourly_readings;
    std::map<time_t, std::vector<double>> daily_readings;

    std::string getFormattedTime(time_t timestamp) {
        char buffer[26];
        struct tm* timeinfo = localtime(&timestamp);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        return std::string(buffer);
    }

    void writeToRawLog(const TempReading& reading) {
        std::vector<TempReading> readings;
        
        std::ifstream inFile(raw_log_path);
        time_t timestamp;
        double temp;
        while (inFile >> timestamp >> temp) {
            readings.push_back({timestamp, temp});
        }
        inFile.close();

        readings.push_back(reading);

        time_t cutoff = reading.timestamp - 24*60*60;
        std::vector<TempReading> filtered_readings;
        for (const auto& r : readings) {
            if (r.timestamp >= cutoff) {
                filtered_readings.push_back(r);
            }
        }

        std::ofstream outFile(raw_log_path, std::ios::trunc);
        for (const auto& r : filtered_readings) {
            outFile << r.timestamp << " " << r.temperature << std::endl;
        }
    }

    void processHourlyAverage(const TempReading& reading) {
        hourly_readings.push_back(reading);
        
        struct tm* timeinfo = localtime(&reading.timestamp);
        int current_hour = timeinfo->tm_hour;
        
        if (!hourly_readings.empty()) {
            timeinfo = localtime(&hourly_readings.front().timestamp);
            int first_hour = timeinfo->tm_hour;
            
            if (current_hour != first_hour || hourly_readings.size() >= 3600) {
                double sum = std::accumulate(hourly_readings.begin(), hourly_readings.end(), 0.0,
                    [](double acc, const TempReading& r) { return acc + r.temperature; });
                double average = sum / hourly_readings.size();

                std::ofstream outFile(hourly_log_path, std::ios::app);
                outFile << reading.timestamp << " " << average << std::endl;

                daily_readings[reading.timestamp - reading.timestamp % (24*60*60)].push_back(average);

                hourly_readings.clear();
            }
        }

        std::vector<std::string> hourly_lines;
        std::ifstream hourly_file(hourly_log_path);
        std::string line;
        time_t month_ago = reading.timestamp - 30*24*60*60;
        
        while (std::getline(hourly_file, line)) {
            time_t line_time;
            std::istringstream iss(line);
            if (iss >> line_time && line_time >= month_ago) {
                hourly_lines.push_back(line);
            }
        }
        
        std::ofstream outFile(hourly_log_path, std::ios::trunc);
        for (const auto& l : hourly_lines) {
            outFile << l << std::endl;
        }
    }

    void processDailyAverage(time_t current_time) {
        time_t current_day = current_time - current_time % (24*60*60);
        
        std::map<time_t, std::vector<double>> remaining_readings;
        for (const auto& pair : daily_readings) {
            if (pair.first < current_day) {
                double sum = std::accumulate(pair.second.begin(), pair.second.end(), 0.0);
                double average = sum / pair.second.size();

                std::ofstream outFile(daily_log_path, std::ios::app);
                outFile << pair.first << " " << average << std::endl;
            } else {
                remaining_readings[pair.first] = pair.second;
            }
        }
        daily_readings = remaining_readings;
    }

public:
    TemperatureMonitor(const std::string& portName) {
        logs_dir = fs::current_path() / "logs";
        raw_log_path = logs_dir / "raw_temp.log";
        hourly_log_path = logs_dir / "hourly_temp.log";
        daily_log_path = logs_dir / "daily_temp.log";

        try {
            fs::create_directories(logs_dir);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Failed to create logs directory: " << e.what() << std::endl;
            throw;
        }

        serialPort = SerialPort::create();
        if (!serialPort->open(portName, 9600)) {
            throw std::runtime_error("Failed to open serial port: " + portName);
        }
    }

    void run() {
        std::cout << "Temperature monitor started. Reading from serial port..." << std::endl;
        
        std::string data;
        while (true) {
            if (serialPort->read(data)) {
                std::istringstream iss(data);
                TempReading reading;
                
                if (iss >> reading.timestamp >> reading.temperature) {
                    writeToRawLog(reading);
                    processHourlyAverage(reading);
                    processDailyAverage(reading.timestamp);
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    ~TemperatureMonitor() {
        if (serialPort) {
            serialPort->close();
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        std::cout << "Example: " << argv[0] << " COM1    (on Windows)" << std::endl;
        std::cout << "Example: " << argv[0] << " /dev/ttyUSB0    (on Unix)" << std::endl;
        return 1;
    }

    try {
        TemperatureMonitor monitor(argv[1]);
        monitor.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
} 