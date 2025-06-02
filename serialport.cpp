#include "serialport.hpp"

// https://yougame.biz/members/1274807/
// https://t.me/anfisov

SerialPort::SerialPort(const std::wstring& port_name) : handle_(INVALID_HANDLE_VALUE) {
    handle_ = CreateFileW(port_name.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (handle_ == INVALID_HANDLE_VALUE) {
        const DWORD error = GetLastError();
        throw std::runtime_error(error == ERROR_FILE_NOT_FOUND
            ? "Arduino port not found"
            : "Error connecting to port: " + std::to_string(error));
    }

    ConfigurePort();
    SetTimeouts();
}

SerialPort::~SerialPort() {
    Close();
}

void SerialPort::ConfigurePort() {
    DCB dcb = {};
    dcb.DCBlength = sizeof(dcb);

    if (!GetCommState(handle_, &dcb)) {
        Close();
        throw std::runtime_error("Failed to get serial port state");
    }

    dcb.BaudRate = CBR_9600;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;

    if (!SetCommState(handle_, &dcb)) {
        Close();
        throw std::runtime_error("Failed to set serial port state");
    }
}

void SerialPort::SetTimeouts() {
    COMMTIMEOUTS timeouts = {};
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(handle_, &timeouts)) {
        Close();
        throw std::runtime_error("Failed to set serial port timeouts");
    }
}

bool SerialPort::Write(const char* data, DWORD length) const {
    DWORD bytes_written;
    if (!WriteFile(handle_, data, length, &bytes_written, nullptr)) {
        return false;
    }
    return bytes_written == length;
}

void SerialPort::Close() noexcept {
    if (handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(handle_);
        handle_ = INVALID_HANDLE_VALUE;
    }
}