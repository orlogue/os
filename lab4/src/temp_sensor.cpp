#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <random>
#include <ctime>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    fs::create_directories("temp");
    
    std::ofstream outFile("temp/temperature_sensor");
    if (!outFile.is_open()) {
        std::cerr << "Failed to open output file" << std::endl;
        return 1;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> temp_dist(20.0, 5.0);

    std::cout << "Temperature sensor simulator started. Writing to temp/temperature_sensor" << std::endl;

    while (true) {
        double temperature = temp_dist(gen);
        
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::system_clock::to_time_t(now);
        
        outFile.seekp(0);
        outFile << timestamp << " " << temperature << std::endl;
        outFile.flush();

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
} 