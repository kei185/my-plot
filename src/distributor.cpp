
#include "protocol.hpp"
#include "distributor.hpp"
#include "frame.hpp"
#include "xqueue.hpp"
#include <format>
#include <stop_token>

namespace distributor
{

DeviceController::DeviceController(
        frame::Type                          type,
        transmitter::Transmitter&            transmiter,
        xqueue::Queue<frame::systemMessage>& inQueue)
    : type(type), transmitter(transmiter), inQueue(inQueue) {};

void DeviceController::run(std::stop_token st)
{
        protocol::run(st, this->transmitter, this->inQueue);
}

template <> std::string Plotter<frame::LidarPoint>::toString(frame::LidarPoint m)
{
        return std::format("{} {}", m.angle, m.dist);
}

} // namespace distributor
