#pragma once

#include "error.hpp"
#include "logger.hpp"

#include <cstdlib>
#include <expected>
#include <source_location>
#include <utility>

[[noreturn]] inline void abortByError(
        error::Error err,
        std::source_location location = std::source_location::current())
{
        logger::log(err, location);
        std::exit(1);
}

template <typename T>
T unwrap(
        std::expected<T, error::Error> result,
        std::source_location location = std::source_location::current())
{
        if (!result)
                abortByError(result.error(), location);

        return std::move(result).value();
}
