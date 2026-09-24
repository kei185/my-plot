
#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <termios.h>
#include <string>
#include "error.hpp"
namespace io
{

struct Port
{
        int            fd;
        struct termios tty;
        Port(std::string);
        ~Port();

        Port(const Port&)            = delete;
        Port& operator=(const Port&) = delete;

        std::expected<void, error::Error> readRaw(std::span<uint8_t>);
        std::expected<void, error::Error> writeRaw(std::span<const uint8_t>);
};

} // namespace io
