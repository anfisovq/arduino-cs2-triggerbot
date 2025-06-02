#pragma once
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>

struct WeaponConfig {
    DWORD delay;
    DWORD shot_cooldown;
};

class ConfigManager {
public:
    ConfigManager() = default;

    bool LoadConfig(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') {
                continue;
            }

            std::istringstream iss(line);
            std::string weapon_id;
            DWORD delay, shot_cooldown;

            if (iss >> weapon_id >> delay >> shot_cooldown) {
                weapon_configs_[weapon_id] = { delay, shot_cooldown };
            }
        }
        return true;
    }

    WeaponConfig GetWeaponConfig(const std::string& weapon_id) const {
        auto it = weapon_configs_.find(weapon_id);
        if (it != weapon_configs_.end()) {
            return it->second;
        }
        return { 0, 135 };
    }

private:
    std::unordered_map<std::string, WeaponConfig> weapon_configs_;
};