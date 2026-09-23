#include "transmitter.hpp"
#include "frame.hpp"
#include "io.hpp"
#include "utility/logger.hpp"
#include "xqueue.hpp"
#include <chrono>
#include <format>
#include <span>
#include <stop_token>

namespace transmitter
{

Transmitter::Transmitter(io::Port& port) : port(port) {}

/**
 * Transmits a frame of the given type of operation.
 */
std::expected<void, error::Error> Transmitter::transmit(frame::OperationType type)
{
        const auto& command = frame::OPERATION.at(type);

        logger::log(std::format("TRANSMIT {} TRY", frame::toString(type)));

        auto result = this->port.writeRaw(std::span<const uint8_t>(command));
        if (!result)
                return result;

        logger::log(std::format("TRANSMIT {} DONE", frame::toString(type)));

        return {};
}

/**
 * Sends a request to the transmitter and waits for an ACK response.
 */
std::expected<void, error::Error> Transmitter::request(
        std::stop_token                      st,
        frame::OperationType                 type,
        xqueue::Queue<frame::systemMessage>& mQueue)
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
                        // if timeout
                        if (std::chrono::steady_clock::now() >= deadline)
                                return std::unexpected(error::Error::OPERATION_TIMEOUT);

                        continue;
                }

                auto res = mQueue.pop();

                logger::log(std::format("RECEIVED {}", res.message));
                if (res.type == frame::ACK_TYPE(type))
                        return {};
        }
};

} // namespace transmitter
