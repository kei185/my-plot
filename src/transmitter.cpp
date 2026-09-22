#include "transmitter.hpp"
#include "frame.hpp"
#include "io.hpp"
#include "utility/logger.hpp"
#include <chrono>
#include <format>
#include <queue>
#include <span>
#include <stop_token>

namespace transmitter
{

Transmitter::Transmitter(io::Port& port) : port(port) {}

std::expected<void, error::Error> Transmitter::transmit(frame::OperationType type)
{
        logger::log(std::format("TRANSMIT {}", frame::toString(type)));
        const auto& command = frame::TX.at(type);
        return this->port.writeRaw(std::span<const uint8_t>(command));
}

std::expected<void, error::Error> Transmitter::request(
        std::stop_token                   st,
        frame::OperationType              type,
        std::queue<frame::systemMessage>& mQueue)
{
        // transmit
        if (auto result = this->transmit(type); !result)
                return result;

        // set timeout
        const auto deadline = std::chrono::steady_clock::now() + frame::OPERATION_TIMEOUT;

        while (1) {
                // check stop request
                if (st.stop_requested())
                        return std::unexpected(error::Error::THREAD_ABORTED);

                // check buffer
                if (mQueue.empty()) {
                        if (std::chrono::steady_clock::now() >= deadline)
                                return std::unexpected(error::Error::OPERATION_TIMEOUT);

                        continue;
                }

                auto res = mQueue.front();
                logger::log(std::format("RECEIVE {}", res.message));

                return {};
        }
};

} // namespace transmitter
