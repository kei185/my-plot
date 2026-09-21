#pragma once
#include <chrono>
#include <map>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

/**
 * @see https://github.com/kei185/sensor-echo/blob/main/docs/frame.md#tx-frame
 */
namespace frame
{

extern const std::chrono::milliseconds OPERATION_TIMEOUT;
extern const uint8_t                   START_OF_FRAME[];

// TODO 更新する
enum class OperationType
{
        WAIT_READY,
        GET_STATUS,
        START_SCAN,
        END_SCAN,
};

// TODO 更新する
constexpr std::string_view toString(OperationType type)
{
        switch (type) {
                case OperationType::WAIT_READY:
                        return "WAIT_READY";
                case OperationType::GET_STATUS:
                        return "GET_STATUS";
                case OperationType::START_SCAN:
                        return "START_SCAN";
                case OperationType::END_SCAN:
                        return "END_SCAN";
                default:
                        return "UNKNOWN";
        }
}

// TODO
enum class Type : uint8_t
{
        SYSTEM,
        LIDAR,
        // IMU,
        // ENCODER
};

constexpr std::string_view toString(Type type)
{
        switch (type) {
                case Type::SYSTEM:
                        return "SYSTEM";
                case Type::LIDAR:
                        return "LIDAR";
                default:
                        return "UNKNOWN";
        }
}

extern const std::vector<Type> TYPES;

struct FrameHeader
{

        uint16_t length;
        Type     type;
        uint32_t timestamp;
};
extern const size_t FRAME_HEADER_SIZE;

/**
 * payload field holds raw byte array that must be accessed with little endian
 */
struct Frame
{
        uint16_t             length;
        Type                 type;
        std::vector<uint8_t> payload;
};
using Crc     = uint8_t;
using CrcData = uint16_t;

// the base is at the payload length byte field
inline constexpr size_t CRC_OFFSET       = sizeof(CrcData);
inline constexpr size_t TYPE_OFFSET      = CRC_OFFSET + sizeof(Crc);
inline constexpr size_t TIMESTAMP_OFFSET = TYPE_OFFSET + sizeof(Type);
inline constexpr size_t RAW_HEADER_SIZE  = TIMESTAMP_OFFSET + sizeof(uint32_t);

struct LidarPoint
{
        uint16_t dist;
        float    angle;
};
extern const size_t LIDAR_POINT_SIZE;

using systemMessage = std::string;

struct Operation
{
        std::array<uint8_t, 2> command;
        std::string            ack;

        Operation(const std::array<uint8_t, 2>& command, const std::string& ack);
};

extern const std::map<OperationType, Operation> TX;

} // namespace frame
