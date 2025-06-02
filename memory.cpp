#include "memory.hpp"
#include <psapi.h>
#include <algorithm>

namespace {
    constexpr size_t MAX_PROCESS_COUNT = 1024;
    constexpr size_t MAX_MODULE_COUNT = 1024;
}

MemoryReader::MemoryReader(const std::string& process_name) : process_handle_(nullptr) {
    DWORD processes[MAX_PROCESS_COUNT];
    DWORD bytes_returned;

    if (!EnumProcesses(processes, sizeof(processes), &bytes_returned)) {
        throw std::runtime_error("Failed to enumerate processes: " + std::to_string(GetLastError()));
    }

    for (unsigned int i = 0; i < bytes_returned / sizeof(DWORD); ++i) {
        HANDLE h_process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processes[i]);
        if (!h_process) {
            continue;
        }

        wchar_t sz_process_name[MAX_PATH];
        if (GetModuleBaseNameW(h_process, nullptr, sz_process_name, MAX_PATH)) {
            std::string process_name_str = ConvertWStringToString(sz_process_name);
            if (process_name == process_name_str) {
                process_handle_ = h_process;
                return;
            }
        }
        CloseHandle(h_process);
    }

    throw std::runtime_error("Process not found: " + process_name);
}

MemoryReader::~MemoryReader() {
    if (process_handle_) {
        CloseHandle(process_handle_);
    }
}

HANDLE MemoryReader::GetProcessHandle() const noexcept {
    return process_handle_;
}

std::string MemoryReader::ConvertWStringToString(const std::wstring& wstr) {
    if (wstr.empty()) {
        return "";
    }

    const int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string str(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size, nullptr, nullptr);
    return str.substr(0, str.size() - 1);
}

uintptr_t MemoryReader::GetModuleBaseAddress(const std::string& module_name) {
    if (module_name.empty() || !process_handle_) {
        return 0;
    }

    if (module_cache_.find(module_name) != module_cache_.end()) {
        return module_cache_[module_name];
    }

    HMODULE modules[MAX_MODULE_COUNT];
    DWORD bytes_needed;

    if (!EnumProcessModules(process_handle_, modules, sizeof(modules), &bytes_needed)) {
        return 0;
    }

    for (unsigned int i = 0; i < (bytes_needed / sizeof(HMODULE)); ++i) {
        wchar_t sz_mod_name[MAX_PATH];
        if (GetModuleBaseNameW(process_handle_, modules[i], sz_mod_name, MAX_PATH)) {
            std::string mod_name_str = ConvertWStringToString(sz_mod_name);
            if (module_name == mod_name_str) {
                MODULEINFO mod_info;
                if (GetModuleInformation(process_handle_, modules[i], &mod_info, sizeof(mod_info))) {
                    const uintptr_t base_addr = reinterpret_cast<uintptr_t>(mod_info.lpBaseOfDll);
                    module_cache_[module_name] = base_addr;
                    return base_addr;
                }
            }
        }
    }

    return 0;
}

template<typename T>
T MemoryReader::Read(uintptr_t address) const {
    T result{};
    if (!ReadProcessMemory(process_handle_, reinterpret_cast<LPCVOID>(address), &result, sizeof(T), nullptr)) {
        return T{};
    }
    return result;
}

std::vector<uint8_t> MemoryReader::ReadBytes(uintptr_t address, size_t size) const {
    std::vector<uint8_t> buffer(size);
    if (!ReadProcessMemory(process_handle_, reinterpret_cast<LPCVOID>(address), buffer.data(), size, nullptr)) {
        return {};
    }
    return buffer;
}

std::string MemoryReader::ReadString(uintptr_t address, size_t length) const {
    std::vector<uint8_t> bytes = ReadBytes(address, length);
    if (bytes.empty()) {
        return "";
    }

    std::string result(bytes.begin(), bytes.end());
    const size_t null_pos = result.find('\0');
    return null_pos != std::string::npos ? result.substr(0, null_pos) : result;
}

bool MemoryReader::WriteBytes(uintptr_t address, const std::vector<uint8_t>& data) const {
    return WriteProcessMemory(process_handle_, reinterpret_cast<LPVOID>(address),
        data.data(), data.size(), nullptr) != 0;
}

std::vector<float> MemoryReader::ReadMatrix(uintptr_t address) const {
    std::vector<uint8_t> bytes = ReadBytes(address, 64);
    std::vector<float> matrix(16);
    if (!bytes.empty()) {
        std::memcpy(matrix.data(), bytes.data(), 64);
    }
    return matrix;
}

template int MemoryReader::Read<int>(uintptr_t) const;
template uintptr_t MemoryReader::Read<uintptr_t>(uintptr_t) const;
template int32_t MemoryReader::Read<int32_t>(uintptr_t) const;