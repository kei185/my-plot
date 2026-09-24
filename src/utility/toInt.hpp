#pragma once
#include <cstdint>

constexpr uint32_t toInt32(const uint8_t* p)
{
        return (static_cast<uint32_t>(p[3]) << 24) | (static_cast<uint32_t>(p[2]) << 16) |
               (static_cast<uint32_t>(p[1]) << 8) | static_cast<uint32_t>(p[0]);
}

constexpr uint16_t toInt16(const uint8_t* p)
{
        return (static_cast<uint16_t>(p[1]) << 8) | static_cast<uint16_t>(p[0]);
}
