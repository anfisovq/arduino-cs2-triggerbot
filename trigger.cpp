#include "trigger.hpp"
#include <iostream>

// https://yougame.biz/members/1274807/
// https://t.me/anfisov

namespace TriggerBot {
    Trigger::Trigger(DWORD process_id, float hit_chance, const ConfigManager& config_manager)
        : process_id_(process_id)
        , hit_chance_(hit_chance)
        , delay_(0)
        , shot_cooldown_(135)
        , last_shot_time_(0)
        , random_engine_(std::random_device{}())
        , distribution_(0.0f, 100.0f)
        , config_manager_(config_manager) {
    }

    bool Trigger::IsProcessWindowActive() const {
        HWND foreground_window = GetForegroundWindow();
        if (!foreground_window) {
            return false;
        }
        DWORD window_process_id;
        GetWindowThreadProcessId(foreground_window, &window_process_id);
        return window_process_id == process_id_;
    }

    void Trigger::UpdateWeaponConfig(const std::string& weapon_id) {
        WeaponConfig config = config_manager_.GetWeaponConfig(weapon_id);
        delay_ = config.delay;
        shot_cooldown_ = config.shot_cooldown;
    }

    void Trigger::Execute(int entity_index, bool is_enabled, bool is_key_pressed, SerialPort& serial, const std::string& weapon_id) {
        if (entity_index == -1 || !is_enabled || !is_key_pressed) {
            return;
        }

        UpdateWeaponConfig(weapon_id);

        const DWORD current_time = GetTickCount();
        if (last_shot_time_ == 0 || (current_time - last_shot_time_) >= shot_cooldown_) {
            Sleep(delay_);
            if (distribution_(random_engine_) <= hit_chance_) {
                if (!serial.Write("1", 1)) {
                    std::cerr << "Failed to write '1' to serial port." << std::endl;
                }
                last_shot_time_ = current_time;
            }
        }
    }
}