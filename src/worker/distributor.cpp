#include "worker/distributor.hpp"

#include "core/protocol.hpp"
#include "demo/gnuplot.hpp"

#include <stop_token>

namespace distributor
{

/**
 * device
 */

DeviceController::DeviceController(
        frame::Type                          type,
        transmitter::Transmitter&            transmiter,
        xqueue::Queue<frame::systemMessage>& inQueue)
    : type(type), transmitter(transmiter), inQueue(inQueue) {};

void DeviceController::run(std::stop_token st)
{
        protocol::run(st, this->transmitter, this->inQueue);
}

/**
 * Lidar
 */

template <> void Plotter<frame::LidarPoint>::run(std::stop_token st)
{
        demo::run(st, this->inQueue);
}

/**
 * IMU
 */

template <> void Plotter<frame::Imu>::run(std::stop_token st)
{
        demo::runImuOrientation(st, this->inQueue);
}

} // namespace distributor
