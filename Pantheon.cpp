#include <windows.h>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <array>
#include <algorithm>
#include "memory.hpp"
#include "offsets.hpp"
#include "serialport.hpp"
#include "trigger.hpp"
#include "gundetect.hpp"
#include "config_manager.hpp"

namespace {
    constexpr int TRIGGER_KEY = VK_XBUTTON2;
    constexpr float TRIGGER_HITCHANCE = 90.0f;
    constexpr bool TRIGGER_ENABLED = true;
    constexpr int POLL_INTERVAL_MS = 5;
    constexpr int INACTIVE_POLL_MS = 50;
    constexpr auto UPDATE_INTERVAL = std::chrono::milliseconds(10);

    constexpr std::array<const char*, 7> EXCLUDED_WEAPONS = {
        "weapon_knife",
        "weapon_flashbang",
        "weapon_smokegrenade",
        "weapon_incgrenade",
        "weapon_molotov",
        "weapon_decoy",
        "weapon_hegrenade"
    };
}

int main() {
    SetConsoleTitle(L"pantheon.solutions");

    try {
        std::cout << "Enter config file name : ";
        std::string config_filename;
        std::getline(std::cin, config_filename);
        if (config_filename.empty()) {
            config_filename = "config.pantheon";
        }

        ConfigManager config_manager;
        if (!config_manager.LoadConfig(config_filename)) {
            std::cerr << "Failed to load config file: " << config_filename << std::endl;
            std::cout << "Continue with default values? (y/n): ";
            std::string response;
            std::getline(std::cin, response);
            if (response != "y" && response != "Y") {
                return 1;
            }
        }

        std::cout << "Enter COM port : ";
        std::string port_input;
        std::getline(std::cin, port_input);

        if (port_input.empty() || port_input.find("COM") != 0) {
            throw std::runtime_error("Invalid COM port format.");
        }

        std::wstring port_name = L"\\\\.\\" + std::wstring(port_input.begin(), port_input.end());
        SerialPort serial(port_name);
        std::cout << "Arduino connected on " << port_input << std::endl;

        MemoryReader reader("cs2.exe");
        std::cout << "Successfully attached to cs2.exe process" << std::endl;

        const uintptr_t client_base = reader.GetModuleBaseAddress("client.dll");
        if (!client_base) {
            throw std::runtime_error("Failed to find client.dll module base address");
        }
        std::cout << "client.dll base address: 0x" << std::hex << client_base << std::dec << std::endl;

        const DWORD process_id = GetProcessId(reader.GetProcessHandle());
        if (!process_id) {
            throw std::runtime_error("Failed to get cs2.exe process ID");
        }

        TriggerBot::Trigger trigger(process_id, TRIGGER_HITCHANCE, config_manager);
        bool was_player_detected = false;
        auto last_update = std::chrono::steady_clock::now();

        while (true) {
            if (!trigger.IsProcessWindowActive()) {
                if (was_player_detected) {
                    if (!serial.Write("0", 1)) {
                        std::cerr << "Failed to write '0' to serial port" << std::endl;
                    }
                    was_player_detected = false;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(INACTIVE_POLL_MS));
                continue;
            }

            const uintptr_t local_player_pawn = reader.Read<uintptr_t>(client_base + Offsets::dwLocalPlayerPawn);
            const std::string weapon_info = GunDetect::DetectActiveWeapon(reader, client_base, local_player_pawn);

            if (!local_player_pawn) {
                std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));
                continue;
            }
            const bool is_excluded_weapon = std::any_of(EXCLUDED_WEAPONS.begin(), EXCLUDED_WEAPONS.end(),
                [&weapon_info](const char* weapon) {
                    return weapon_info.find(weapon) != std::string::npos;
                });

            if (is_excluded_weapon) {
                std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));
                continue;
            }

            const int32_t local_team = reader.Read<int32_t>(local_player_pawn + Offsets::m_iTeamNum);
            const int32_t ent_index = reader.Read<int32_t>(local_player_pawn + Offsets::m_iIDEntIndex);
            const bool is_key_pressed = GetAsyncKeyState(TRIGGER_KEY) & 0x8000;

            std::string weapon_id = weapon_info.substr(weapon_info.find("weapon_"));

            bool is_player_detected = false;
            if (is_key_pressed && TRIGGER_ENABLED && ent_index != -1) {
                const uintptr_t entity_list = reader.Read<uintptr_t>(client_base + Offsets::dwEntityList);
                if (!entity_list) {
                    std::cerr << "Invalid entity list pointer" << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));
                    continue;
                }

                const uintptr_t list_entry = reader.Read<uintptr_t>(entity_list + 0x8 * ((ent_index & 0x7FFF) >> 9) + 0x10);
                if (!list_entry) {
                    std::cerr << "Invalid list entry pointer" << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));
                    continue;
                }

                const uintptr_t target_pawn = reader.Read<uintptr_t>(list_entry + 0x78 * (ent_index & 0x1FF));
                if (!target_pawn) {
                    std::cerr << "Invalid target pawn pointer" << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));
                    continue;
                }

                const int32_t target_life_state = reader.Read<int32_t>(target_pawn + Offsets::m_lifeState);
                const int32_t target_team = reader.Read<int32_t>(target_pawn + Offsets::m_iTeamNum);

                if (target_life_state == 256 && target_team != local_team &&
                    (target_team == 2 || target_team == 3)) {
                    trigger.Execute(ent_index, TRIGGER_ENABLED, is_key_pressed, serial, weapon_id);
                    is_player_detected = true;
                }
            }

            const auto now = std::chrono::steady_clock::now();
            if (is_player_detected != was_player_detected &&
                std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update) >= UPDATE_INTERVAL) {
                if (!serial.Write(is_player_detected ? "1" : "0", 1)) {
                    std::cerr << "Failed to write '" << (is_player_detected ? "1" : "0") << "' to serial port" << std::endl;
                }
                else {
                    was_player_detected = is_player_detected;
                    last_update = now;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}