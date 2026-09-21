#include "transmitter.hpp"
#include "frame.hpp"
#include "io.hpp"
#include <chrono>
#include <print>
#include <queue>
#include <span>
#include <stop_token>
#include <thread>

namespace transmitter
{

Transmitter::Transmitter(io::Port& port) : port(port) {}

std::expected<void, error::Error> Transmitter::transmit(frame::OperationType type)
{
        const auto& command = frame::TX.at(type).command;
        return this->port.writeRaw(std::span<const uint8_t>(command));
}

std::expected<void, error::Error> Transmitter::request(
        std::stop_token                   st,
        frame::OperationType              type,
        std::queue<frame::systemMessage>& mQueue)
{
        if (auto result = this->transmit(type); !result)
                return result;

        std::println("{}: TRANSMIT {}", std::this_thread::get_id(), frame::toString(type));

        const auto deadline = std::chrono::steady_clock::now() + frame::OPERATION_TIMEOUT;

        while (1) {
                if (st.stop_requested())
                        return std::unexpected(error::Error::THREAD_ABORTED);

                if (mQueue.empty()) {
                        if (std::chrono::steady_clock::now() >= deadline)
                                return std::unexpected(error::Error::OPERATION_TIMEOUT);

                        continue;
                }

                auto res = mQueue.front();
                std::println("{}: RECEIVE {}", std::this_thread::get_id(), res);
                return {};
        }
};

} // namespace transmitter
