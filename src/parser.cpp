
#include <cstdint>
#include <span>
#include <vector>

#include "frame.hpp"
#include "io.hpp"
#include "parser.hpp"

namespace parser
{

template <>
std::vector<frame::systemMessage> Parser<frame::systemMessage>::parsePayload(frame::Frame& fr)
{

        return {{
                .message = std::string(fr.payload.begin(), fr.payload.end()),
                .type    = fr.type,
        }};
}

template <> std::vector<frame::LidarPoint> Parser<frame::LidarPoint>::parsePayload(frame::Frame& fr)
{
        std::vector<frame::LidarPoint> points = {};

        uint16_t angle_q6;
        uint16_t dist;

        for (size_t offset = 0; offset + frame::LIDAR_POINT_SIZE <= fr.payload.size();
             offset += frame::LIDAR_POINT_SIZE) {

                const auto point = std::span<const uint8_t, frame::LIDAR_POINT_SIZE>(
                        fr.payload.data() + offset,
                        frame::LIDAR_POINT_SIZE);

                angle_q6 = io::decodeBigEndian(point.first<2>());
                dist     = io::decodeBigEndian(point.last<2>());

                points.push_back(
                        {
                                .dist  = dist,
                                .angle = static_cast<float>(angle_q6) / 64.0f,
                        });
        }

        return points;
}

} // namespace parser
