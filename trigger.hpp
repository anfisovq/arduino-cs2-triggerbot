#pragma once
#include <windows.h>
#include <random>
#include "serialport.hpp"
#include "config_manager.hpp"

// https://yougame.biz/members/1274807/
// https://t.me/anfisov

namespace TriggerBot {
    class Trigger {
    public:
        explicit Trigger(DWORD process_id, float hit_chance, const ConfigManager& config_manager);

        void Execute(int entity_index, bool is_enabled, bool is_key_pressed, SerialPort& serial, const std::string& weapon_id);
        bool IsProcessWindowActive() const;
        void UpdateWeaponConfig(const std::string& weapon_id);

    private:
        void SimulateLeftClick() const;

        const DWORD process_id_;
        const float hit_chance_;
        DWORD delay_;
        DWORD shot_cooldown_;
        DWORD last_shot_time_;
        std::mt19937 random_engine_;
        std::uniform_real_distribution<float> distribution_;
        const ConfigManager& config_manager_;
    };
}