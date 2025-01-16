#ifndef _WIN32

#include "serial_port.h"
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

class SerialPortUnix : public SerialPort {
private:
    int fd;
    bool isPortOpen;

    speed_t getBaudRate(int baudRate) {
        switch (baudRate) {
            case 9600: return B9600;
            case 19200: return B19200;
            case 38400: return B38400;
            case 57600: return B57600;
            case 115200: return B115200;
            default: return B9600;
        }
    }

public:
    SerialPortUnix() : fd(-1), isPortOpen(false) {}

    ~SerialPortUnix() override {
        close();
    }

    bool open(const std::string& port, int baudRate) override {
        fd = ::open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
        if (fd == -1) {
            return false;
        }

        fcntl(fd, F_SETFL, 0);

        struct termios options;
        if (tcgetattr(fd, &options) != 0) {
            ::close(fd);
            return false;
        }

        speed_t baud = getBaudRate(baudRate);
        cfsetispeed(&options, baud);
        cfsetospeed(&options, baud);

        options.c_cflag |= (CLOCAL | CREAD);
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS8;
        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        options.c_iflag &= ~(IXON | IXOFF | IXANY);
        options.c_oflag &= ~OPOST;

        options.c_cc[VMIN] = 0;
        options.c_cc[VTIME] = 10;

        if (tcsetattr(fd, TCSANOW, &options) != 0) {
            ::close(fd);
            return false;
        }

        isPortOpen = true;
        return true;
    }

    void close() override {
        if (isPortOpen) {
            ::close(fd);
            fd = -1;
            isPortOpen = false;
        }
    }

    bool write(const std::string& data) override {
        if (!isPortOpen) return false;

        return ::write(fd, data.c_str(), data.length()) != -1;
    }

    bool read(std::string& data) override {
        if (!isPortOpen) return false;

        char buffer[256];
        int bytesRead = ::read(fd, buffer, sizeof(buffer) - 1);
        
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            data = buffer;
            return true;
        }
        return false;
    }

    bool isOpen() const override {
        return isPortOpen;
    }
};

std::unique_ptr<SerialPort> SerialPort::create() {
    return std::make_unique<SerialPortUnix>();
}

#endif 