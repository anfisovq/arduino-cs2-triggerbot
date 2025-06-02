#pragma once
#include <string>
// https://yougame.biz/members/1274807/
// https://t.me/anfisov


namespace Offsets {
    constexpr std::ptrdiff_t dwEntityList = 0x19FEE38;
    constexpr std::ptrdiff_t dwLocalPlayerPawn = 0x18530D0;

    constexpr std::ptrdiff_t m_iTeamNum = 0x3E3;
    constexpr std::ptrdiff_t m_iHealth = 0x344;
    constexpr std::ptrdiff_t m_hPlayerPawn = 0x824;
    constexpr std::ptrdiff_t m_iIDEntIndex = 0x1458;
    constexpr std::ptrdiff_t m_lifeState = 0x348;
    constexpr std::ptrdiff_t m_pClippingWeapon = 0x13A0;
    constexpr std::ptrdiff_t m_iItemDefinitionIndex = 0x1BA;
    constexpr std::ptrdiff_t m_pWeaponServices = 0x11A8;
    constexpr std::ptrdiff_t m_AttributeManager = 0x1240;
    constexpr std::ptrdiff_t m_hActiveWeapon = 0x58;
    constexpr std::ptrdiff_t m_Item = 0x50;

    inline std::string GetTeamName(int32_t team_num) noexcept {
        switch (team_num) {
        case 1: return "Spectator";
        case 2: return "Terrorists";
        case 3: return "CounterTerrorists";
        default: return "Unknown";
        }
    }
}