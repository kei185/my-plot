#pragma once

#include <cstddef>
#include <cstdint>
#include <queue>
#include <span>
#include <stop_token>
#include <unistd.h>
#include <cstdlib>
#include <expected>

#include "frame.hpp"
#include "error.hpp"
#include "io.hpp"

using namespace error;

namespace receiver
{

struct Receiver
{
        io::Port&                                        port;
        std::map<frame::Type, std::queue<frame::Frame>>& frameStreams;

        Receiver(io::Port&, std::map<frame::Type, std::queue<frame::Frame>>&);

        void run(std::stop_token);

        static std::expected<bool, Error>               findSOF(std::stop_token&, io::Port&);
        static std::expected<frame::FrameHeader, Error> getFrameHeader(io::Port&);
        static std::expected<frame::Frame, Error> getPayload(io::Port&, const frame::FrameHeader&);
        static bool isValidCRC(std::span<const uint8_t, frame::RAW_HEADER_SIZE>);
};
} // namespace receiver
