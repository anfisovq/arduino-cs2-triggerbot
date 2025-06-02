#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

// https://yougame.biz/members/1274807/
// https://t.me/anfisov

class MemoryReader {
public:
    explicit MemoryReader(const std::string& process_name);
    ~MemoryReader();

    MemoryReader(const MemoryReader&) = delete;
    MemoryReader& operator=(const MemoryReader&) = delete;

    HANDLE GetProcessHandle() const noexcept;
    uintptr_t GetModuleBaseAddress(const std::string& module_name);

    template<typename T>
    T Read(uintptr_t address) const;

    std::vector<uint8_t> ReadBytes(uintptr_t address, size_t size) const;
    std::string ReadString(uintptr_t address, size_t length) const;
    bool WriteBytes(uintptr_t address, const std::vector<uint8_t>& data) const;
    std::vector<float> ReadMatrix(uintptr_t address) const;

private:
    HANDLE process_handle_;
    std::unordered_map<std::string, uintptr_t> module_cache_;

    static std::string ConvertWStringToString(const std::wstring& wstr);
};