#include <array>
#include <map>
#include <stdint.h>
#include <cstddef>
#include <cstdint>
#include <string>
#include <sys/types.h>
#include "frame.hpp"

namespace frame
{
const std::chrono::milliseconds OPERATION_TIMEOUT(1000);

// TODO byteサイズ数値になおす
const size_t            FRAME_HEADER_SIZE = sizeof(FrameHeader);
const size_t            LIDAR_POINT_SIZE  = sizeof(LidarPoint);
const uint8_t           START_OF_FRAME[]  = {0xAA, 0x55};
const std::vector<Type> TYPES             = {
        Type::SYSTEM,
        Type::LIDAR,
        // Type::IMU,
        // Type::ENCODER
};

static const std::array<uint8_t, 2> COMMAND_GET_STATUS = {0xAA, 0xA1};
static const std::array<uint8_t, 2> COMMAND_START_SCAN = {0xAA, 0xA2};
static const std::array<uint8_t, 2> COMMAND_END_SCAN   = {0xAA, 0xA3};

static const std::string ACK_DEVICE_READY = "ACK DEVICE READY";
static const std::string ACK_GET_STATUS   = "";
static const std::string ACK_START_SCAN   = "SRT SCAN ACK";
static const std::string ACK_END_SCAN     = "END SCAN ACK";

Operation::Operation(const std::array<uint8_t, 2>& command, const std::string& ack)
    : command(command), ack(ack)
{}

const std::map<OperationType, Operation> TX = {
        {OperationType::WAIT_READY, Operation(std::array<uint8_t, 2>(), ACK_DEVICE_READY)},
        {OperationType::GET_STATUS, Operation(COMMAND_GET_STATUS, ACK_GET_STATUS)},
        {OperationType::START_SCAN, Operation(COMMAND_START_SCAN, ACK_START_SCAN)},
        {OperationType::END_SCAN, Operation(COMMAND_END_SCAN, ACK_END_SCAN)},
};

} // namespace frame
