#ifdef _WIN32

#include "serial_port.h"
#include <windows.h>
#include <setupapi.h>
#include <iostream>

class SerialPortWin : public SerialPort {
private:
    HANDLE hSerial;
    bool isPortOpen;

public:
    SerialPortWin() : hSerial(INVALID_HANDLE_VALUE), isPortOpen(false) {}

    ~SerialPortWin() override {
        close();
    }

    bool open(const std::string& port, int baudRate) override {
        std::string fullPortName = "\\\\.\\" + port;
        hSerial = CreateFileA(fullPortName.c_str(),
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            0,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            0);

        if (hSerial == INVALID_HANDLE_VALUE) {
            return false;
        }

        DCB dcbSerialParams = {0};
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

        if (!GetCommState(hSerial, &dcbSerialParams)) {
            CloseHandle(hSerial);
            return false;
        }

        dcbSerialParams.BaudRate = baudRate;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity = NOPARITY;

        if (!SetCommState(hSerial, &dcbSerialParams)) {
            CloseHandle(hSerial);
            return false;
        }

        COMMTIMEOUTS timeouts = {0};
        timeouts.ReadIntervalTimeout = 50;
        timeouts.ReadTotalTimeoutConstant = 50;
        timeouts.ReadTotalTimeoutMultiplier = 10;
        timeouts.WriteTotalTimeoutConstant = 50;
        timeouts.WriteTotalTimeoutMultiplier = 10;

        if (!SetCommTimeouts(hSerial, &timeouts)) {
            CloseHandle(hSerial);
            return false;
        }

        isPortOpen = true;
        return true;
    }

    void close() override {
        if (isPortOpen) {
            CloseHandle(hSerial);
            hSerial = INVALID_HANDLE_VALUE;
            isPortOpen = false;
        }
    }

    bool write(const std::string& data) override {
        if (!isPortOpen) return false;

        DWORD bytesWritten;
        return WriteFile(hSerial, data.c_str(), data.length(), &bytesWritten, nullptr) != 0;
    }

    bool read(std::string& data) override {
        if (!isPortOpen) return false;

        char buffer[256];
        DWORD bytesRead;

        if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, nullptr)) {
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                data = buffer;
                return true;
            }
        }
        return false;
    }

    bool isOpen() const override {
        return isPortOpen;
    }
};

std::unique_ptr<SerialPort> SerialPort::create() {
    return std::make_unique<SerialPortWin>();
}

#endif 