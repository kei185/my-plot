#pragma once
#include <stop_token>
#include "frame.hpp"
#include "transmitter.hpp"
#include "xqueue.hpp"

namespace protocol
{

void run(std::stop_token, transmitter::Transmitter&, xqueue::Queue<frame::systemMessage>&);

}
