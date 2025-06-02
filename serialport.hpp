#pragma once
#include <windows.h>
#include <string>
#include <stdexcept>

// https://yougame.biz/members/1274807/
// https://t.me/anfisov

class SerialPort {
private:
    HANDLE handle_;
    static constexpr DWORD WRITE_BUFFER_SIZE = 256;

public:
    explicit SerialPort(const std::wstring& port_name);
    ~SerialPort();

    SerialPort(const SerialPort&) = delete;
    SerialPort& operator=(const SerialPort&) = delete;

    bool Write(const char* data, DWORD length) const;
    void Close() noexcept;

private:
    void ConfigurePort();
    void SetTimeouts();
};