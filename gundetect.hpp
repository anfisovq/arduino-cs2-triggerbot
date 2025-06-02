#pragma once
#include <string>
#include "memory.hpp"
#include "offsets.hpp"

// https://yougame.biz/members/1274807/
// https://t.me/anfisov

namespace GunDetect {
    std::string DetectActiveWeapon(const MemoryReader& reader, uintptr_t client_base, uintptr_t local_player_pawn);
}