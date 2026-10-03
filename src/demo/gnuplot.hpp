#pragma once

#include "core/frame.hpp"
#include "core/xqueue.hpp"

#include <stop_token>

namespace demo
{

void run(std::stop_token, xqueue::Queue<frame::LidarPoint>&);
void run(std::stop_token, xqueue::Queue<frame::Imu>&);

} // namespace demo
