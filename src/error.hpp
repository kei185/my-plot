
#pragma once

#include <string_view>

namespace error
{

enum class Error
{
        OPEN_FILE_FAILED,
        CLOSE_FILE_FAILED,
        NODE_INIT_FAILED,
        FILE_INTERNAL_ERROR,
        IO_READ_FAILED,
        IO_WRITE_FAILED,
};

constexpr std::string_view toString(Error error)
{
        switch (error) {
                case Error::OPEN_FILE_FAILED:
                        return "OPEN_FILE_FAILED";
                case Error::CLOSE_FILE_FAILED:
                        return "CLOSE_FILE_FAILED";
                case Error::NODE_INIT_FAILED:
                        return "NODE_INIT_FAILED";
                case Error::FILE_INTERNAL_ERROR:
                        return "FILE_INTERNAL_ERROR";
                case Error::IO_READ_FAILED:
                        return "IO_READ_FAILED";
                case Error::IO_WRITE_FAILED:
                        return "IO_WRITE_FAILED";
                default:
                        return "UNKNOWN";
        }
}

} // namespace error
