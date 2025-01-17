#pragma once

#include <string>
#include <memory>

class SerialPort {
public:
    virtual ~SerialPort() = default;
    
    virtual bool open(const std::string& port, int baudRate) = 0;
    
    virtual void close() = 0;
    
    virtual bool write(const std::string& data) = 0;
    
    virtual bool read(std::string& data) = 0;
    
    virtual bool isOpen() const = 0;
    
    static std::unique_ptr<SerialPort> create();
}; 