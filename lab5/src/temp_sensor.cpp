#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <ctime>
#include <sstream>
#include "serial_port.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        std::cout << "Example: " << argv[0] << " COM1    (on Windows)" << std::endl;
        std::cout << "Example: " << argv[0] << " /dev/ttyUSB0    (on Unix)" << std::endl;
        return 1;
    }

    std::string portName = argv[1];
    auto serialPort = SerialPort::create();

    if (!serialPort->open(portName, 9600)) {
        std::cerr << "Failed to open serial port: " << portName << std::endl;
        return 1;
    }

    std::cout << "Temperature sensor simulator started. Connected to " << portName << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> temp_dist(20.0, 10.0);

    while (true) {
        try {
            double temperature = temp_dist(gen);
            
            auto now = std::chrono::system_clock::now();
            auto timestamp = std::chrono::system_clock::to_time_t(now);
            
            std::ostringstream oss;
            oss << timestamp << " " << temperature << "\n";
            
            if (!serialPort->write(oss.str())) {
                std::cerr << "Failed to write to serial port" << std::endl;
                break;
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            break;
        }
    }

    serialPort->close();
    return 0;
} 