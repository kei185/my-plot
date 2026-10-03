#pragma once
#include <stop_token>
#include "core/frame.hpp"
#include "worker/transmitter.hpp"
#include "core/xqueue.hpp"

namespace protocol
{

void run(std::stop_token, transmitter::Transmitter&, xqueue::Queue<frame::systemMessage>&);

}
