#pragma once

#include "error.hpp"

#include <print>
#include <source_location>
#include <string_view>
#include <thread>

namespace logger
{

inline void
log(std::string_view message, std::source_location location = std::source_location::current())
{
        std::println(
                "[{}] {} ({}): {}",
                std::this_thread::get_id(),
                location.file_name(),
                location.function_name(),
                message);
}

inline void log(
        error::Error err,
        std::source_location location = std::source_location::current())
{
        log(error::toString(err), location);
}

} // namespace logger
