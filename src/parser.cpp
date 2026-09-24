
#include <cstdint>
#include <vector>

#include "frame.hpp"
#include "parser.hpp"
#include "utility/logger.hpp"
#include "utility/toInt.hpp"

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

                dist     = toInt16(fr.payload.data() + offset);
                angle_q6 = toInt16(fr.payload.data() + offset + sizeof(uint16_t));

                points.push_back(
                        {
                                .dist  = dist,
                                .angle = static_cast<float>(angle_q6) / 64.0f,
                        });
        }

        return points;
}

} // namespace parser
