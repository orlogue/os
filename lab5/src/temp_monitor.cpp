#include "serial_port.h"
#include "http_server.h"
#include "db_manager.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <memory>
#include <csignal>
#include <filesystem>
#include <sstream>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

namespace fs = std::filesystem;

std::unique_ptr<HttpServer> server;
volatile sig_atomic_t running = 1;

void signal_handler(int) {
    running = 0;
}

fs::path get_executable_path() {
#ifdef __APPLE__
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::string path(size, '\0');
    if (_NSGetExecutablePath(&path[0], &size) == 0) {
        return fs::canonical(path).parent_path();
    }
#elif defined(_WIN32)
    char path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    return fs::path(path).parent_path();
#else
    return fs::canonical("/proc/self/exe").parent_path();
#endif
    return fs::current_path();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <serial_port>" << std::endl;
        return 1;
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    try {
        auto dbManager = std::make_shared<DbManager>("temperature.db");
        dbManager->createTables();

        fs::path exe_path = get_executable_path();
        std::string doc_root = (exe_path / "public").string();
        
        if (!fs::exists(doc_root)) {
            std::cerr << "Warning: Public directory not found at: " << doc_root << std::endl;
            std::cerr << "Creating directory..." << std::endl;
            fs::create_directory(doc_root);
        }

        server = std::make_unique<HttpServer>("0.0.0.0", 8080, doc_root, dbManager);
        
        std::thread server_thread([server_ptr = server.get()]() {
            try {
                server_ptr->start();
            } catch (const std::exception& e) {
                if (running) {
                    std::cerr << "Server error: " << e.what() << std::endl;
                }
            }
        });

        auto port = SerialPort::create();
        if (!port->open(argv[1], 9600)) {
            std::cerr << "Failed to open serial port" << std::endl;
            running = false;
        }

        std::cout << "Server started at http://localhost:8080" << std::endl;
        std::cout << "Press Ctrl+C to stop" << std::endl;

        while (running) {
            try {
                std::string data;
                if (port->read(data)) {
                    std::istringstream iss(data);
                    time_t timestamp;
                    double temperature;
                    if (iss >> timestamp >> temperature) {
                        dbManager->insertTemperature(timestamp, temperature, "raw");
                        std::cout << "Temperature: " << temperature << "°C" << std::endl;
                    }
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        std::cout << "\nShutting down..." << std::endl;
        
        if (server) {
            server->stop();
        }
        
        if (server_thread.joinable()) {
            server_thread.join();
        }
        
        port->close();

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Shutdown complete" << std::endl;
    return 0;
} 