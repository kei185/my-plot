#pragma once
#include "frame.hpp"
#include "io.hpp"
#include "xqueue.hpp"
#include <expected>
#include <stop_token>

namespace transmitter
{

struct Transmitter
{
        io::Port& port;

      public:
        Transmitter(io::Port&);

        std::expected<void, error::Error> transmit(frame::OperationType);

        std::expected<void, error::Error>
        request(std::stop_token                      st,
                frame::OperationType                 type,
                xqueue::Queue<frame::systemMessage>& mQueue);
};

} // namespace transmitter
