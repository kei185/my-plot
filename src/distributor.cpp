
#include "protocol.hpp"
#include "distributor.hpp"
#include "frame.hpp"
#include <format>
#include <stop_token>

namespace distributor
{

DeviceController::DeviceController(
        frame::Type                       type,
        transmitter::Transmitter&         transmiter,
        std::queue<frame::systemMessage>& inQueue)
    : type(type), transmitter(transmiter), inQueue(inQueue) {};

void DeviceController::run(std::stop_token st)
{
        protocol::run(st, this->transmitter, this->inQueue);
}

template <> std::string Plotter<frame::LidarPoint>::toString(frame::LidarPoint m)
{
        return std::format("{} {}", m.dist * cos(m.angle), m.dist * sin(m.angle));
}

} // namespace distributor
