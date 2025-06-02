#include "gundetect.hpp"
#include <array>
#include <iostream>

// https://yougame.biz/members/1274807/
// https://t.me/anfisov

namespace GunDetect {
    namespace {
        constexpr std::ptrdiff_t CLASS_INFO_OFFSET = 0x10;
        constexpr std::ptrdiff_t NAME_POINTER_OFFSET = 0x20;
        constexpr size_t MAX_WEAPON_NAME_LENGTH = 256;
    }

    std::string DetectActiveWeapon(const MemoryReader& reader, uintptr_t client_base, uintptr_t local_player_pawn) {
        if (!local_player_pawn) {
            return "No active weapon (no local player pawn)";
        }

        const uintptr_t weapon_services = reader.Read<uintptr_t>(local_player_pawn + Offsets::m_pWeaponServices);
        if (!weapon_services) {
            return "No active weapon (invalid weapon services)";
        }

        const uintptr_t active_weapon_handle = reader.Read<uintptr_t>(weapon_services + Offsets::m_hActiveWeapon);
        if (!active_weapon_handle || (active_weapon_handle & 0x7FFF) == 0x7FFF) {
            return "No active weapon (invalid weapon handle)";
        }

        const uintptr_t entity_list = reader.Read<uintptr_t>(client_base + Offsets::dwEntityList);
        if (!entity_list) {
            return "No active weapon (invalid entity list)";
        }

        const uintptr_t list_entry = reader.Read<uintptr_t>(entity_list + 0x8 * ((active_weapon_handle & 0x7FFF) >> 9) + 0x10);
        if (!list_entry) {
            return "No active weapon (invalid list entry)";
        }

        const uintptr_t weapon_pawn = reader.Read<uintptr_t>(list_entry + 0x78 * (active_weapon_handle & 0x1FF));
        if (!weapon_pawn) {
            return "No active weapon (invalid weapon pawn)";
        }

        const uintptr_t class_info = reader.Read<uintptr_t>(weapon_pawn + CLASS_INFO_OFFSET);
        if (!class_info) {
            return "No active weapon (invalid class info)";
        }

        const uintptr_t name_pointer = reader.Read<uintptr_t>(class_info + NAME_POINTER_OFFSET);
        if (!name_pointer) {
            return "No active weapon (invalid name pointer)";
        }

        const std::string weapon_name = reader.ReadString(name_pointer, MAX_WEAPON_NAME_LENGTH);
        if (weapon_name.empty()) {
            return "No active weapon (empty weapon name)";
        }

        return "Active weapon: " + weapon_name;
    }
}