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

namespace fs = std::filesystem;

// Structure to store temperature reading
struct TempReading {
    time_t timestamp;
    double temperature;
};

class TemperatureMonitor {
private:
    std::string raw_log_path = "logs/raw_temp.log";
    std::string hourly_log_path = "logs/hourly_temp.log";
    std::string daily_log_path = "logs/daily_temp.log";
    std::vector<TempReading> hourly_readings;
    std::map<time_t, std::vector<double>> daily_readings;

    // Helper function to get formatted timestamp
    std::string getFormattedTime(time_t timestamp) {
        char buffer[26];
        struct tm* timeinfo = localtime(&timestamp);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        return std::string(buffer);
    }

    // Write temperature to raw log and maintain 24-hour window
    void writeToRawLog(const TempReading& reading) {
        std::vector<TempReading> readings;
        
        // Read existing readings
        std::ifstream inFile(raw_log_path);
        time_t timestamp;
        double temp;
        while (inFile >> timestamp >> temp) {
            readings.push_back({timestamp, temp});
        }
        inFile.close();

        // Add new reading
        readings.push_back(reading);

        // Remove readings older than 24 hours
        time_t cutoff = reading.timestamp - 24*60*60;
        readings.erase(
            std::remove_if(readings.begin(), readings.end(),
                [cutoff](const TempReading& r) { return r.timestamp < cutoff; }),
            readings.end()
        );

        // Write back to file
        std::ofstream outFile(raw_log_path, std::ios::trunc);
        for (const auto& r : readings) {
            outFile << r.timestamp << " " << r.temperature << std::endl;
        }
    }

    // Calculate and write hourly average
    void processHourlyAverage(const TempReading& reading) {
        hourly_readings.push_back(reading);
        
        // Get the hour of the current reading
        struct tm* timeinfo = localtime(&reading.timestamp);
        int current_hour = timeinfo->tm_hour;
        
        // Check if we have readings for a full hour
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
        
        for (auto it = daily_readings.begin(); it != daily_readings.end();) {
            if (it->first < current_day) {
                double sum = std::accumulate(it->second.begin(), it->second.end(), 0.0);
                double average = sum / it->second.size();

                std::ofstream outFile(daily_log_path, std::ios::app);
                outFile << it->first << " " << average << std::endl;

                it = daily_readings.erase(it);
            } else {
                ++it;
            }
        }
    }

public:
    TemperatureMonitor() {
        fs::create_directories("logs");
    }

    void run() {
        std::cout << "Temperature monitor started. Reading from temp/temperature_sensor" << std::endl;
        
        std::ifstream sensorFile("temp/temperature_sensor");
        if (!sensorFile.is_open()) {
            std::cerr << "Failed to open sensor file" << std::endl;
            return;
        }

        while (true) {
            sensorFile.seekg(0);
            TempReading reading;
            
            if (sensorFile >> reading.timestamp >> reading.temperature) {
                writeToRawLog(reading);
                processHourlyAverage(reading);
                processDailyAverage(reading.timestamp);
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
};

int main() {
    TemperatureMonitor monitor;
    monitor.run();
    return 0;
} 