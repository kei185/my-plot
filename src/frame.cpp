#include <array>
#include <stdint.h>
#include <cstddef>
#include <cstdint>
#include <sys/types.h>
#include "frame.hpp"

namespace frame
{
const std::chrono::milliseconds OPERATION_TIMEOUT(1000);

const size_t            FRAME_HEADER_SIZE = RAW_HEADER_SIZE;
const uint8_t           START_OF_FRAME[]  = {0xAA, 0x55};
const std::vector<Type> TYPES             = {
        Type::SYSTEM,
        Type::LIDAR,
        // TODO
        // Type::IMU,
        // Type::ENCODER
};

// static const std::array<uint8_t, 2> COMMAND_GET_STATUS = {0xAA, 0xA1};
static const std::array<uint8_t, 2> COMMAND_START_SCAN = {0xAA, 0xA2};
// static const std::array<uint8_t, 2> COMMAND_END_SCAN   = {0xAA, 0xA3};

const std::map<OperationType, std::array<uint8_t, 2>> TX = {
        {OperationType::START_SCAN, COMMAND_START_SCAN},
};

} // namespace frame
