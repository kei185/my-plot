// @generated
#pragma once

#include "core/frame.hpp"
#include "core/xqueue.hpp"

#include <stop_token>

namespace demo
{

void run(std::stop_token, xqueue::Queue<frame::LidarPoint>&);
// Both IMU plots consume the queue, so run only one for a given queue.
void runImuTimeSeries(std::stop_token, xqueue::Queue<frame::Imu>&);
void runImuOrientation(std::stop_token, xqueue::Queue<frame::Imu>&);

} // namespace demo
