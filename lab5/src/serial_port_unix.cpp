#ifndef _WIN32

#include "serial_port.h"
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>

class SerialPortUnix : public SerialPort {
private:
    int fd_;
    bool is_open_;

public:
    SerialPortUnix() : fd_(-1), is_open_(false) {}
    
    ~SerialPortUnix() override {
        if (is_open_) {
            close();
        }
    }

    bool open(const std::string& port, int baudRate) override {
        fd_ = ::open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
        if (fd_ == -1) {
            return false;
        }

        struct termios options;
        tcgetattr(fd_, &options);
        
        speed_t baud;
        switch (baudRate) {
            case 9600: baud = B9600; break;
            case 19200: baud = B19200; break;
            case 38400: baud = B38400; break;
            case 57600: baud = B57600; break;
            case 115200: baud = B115200; break;
            default: baud = B9600; break;
        }
        
        cfsetispeed(&options, baud);
        cfsetospeed(&options, baud);

        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS8;
        
        options.c_cflag &= ~CRTSCTS;
        
        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        
        options.c_oflag &= ~OPOST;
        
        options.c_iflag &= ~(IXON | IXOFF | IXANY);
        
        tcsetattr(fd_, TCSANOW, &options);
        
        is_open_ = true;
        return true;
    }

    void close() override {
        if (is_open_) {
            ::close(fd_);
            fd_ = -1;
            is_open_ = false;
        }
    }

    bool write(const std::string& data) override {
        if (!is_open_) return false;
        return ::write(fd_, data.c_str(), data.length()) > 0;
    }

    bool read(std::string& data) override {
        if (!is_open_) return false;
        
        char buffer[256];
        int n = ::read(fd_, buffer, sizeof(buffer) - 1);
        
        if (n > 0) {
            buffer[n] = '\0';
            data = buffer;
            return true;
        }
        
        return false;
    }

    bool isOpen() const override {
        return is_open_;
    }
};

std::unique_ptr<SerialPort> SerialPort::create() {
    return std::make_unique<SerialPortUnix>();
}

#endif 