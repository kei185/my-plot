
#include "protocol.hpp"
#include "distributor.hpp"
#include "frame.hpp"
#include "xqueue.hpp"
#include "utility/logger.hpp"

#include <format>
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

template <> std::string Plotter<frame::LidarPoint>::toString(frame::LidarPoint m)
{
        return std::format("{} {}", m.angle, m.dist);
}

template <> void Plotter<frame::LidarPoint>::run(std::stop_token st)
{
        static size_t MAX_POINTS = 720;

        FILE* file = popen("gnuplot -persist", "w");

        if (file == nullptr) {
                logger::log("OPEN PROCESS gnuplot FAILED");
                return;
        }

        std::fputs(
                "set title 'LiDAR scan'\n"
                "set polar\n"
                "set angles degrees\n"
                "set theta top clockwise\n"
                "set size square\n"
                "set grid polar 30\n"
                "set rrange [0:*]\n"
                "unset key\n"
                "set style line 1 linecolor rgb '#00AEEF' pointtype 7 pointsize 0.5\n",
                file);

        constexpr auto                        REDRAW_INTERVAL = std::chrono::milliseconds(50);
        std::deque<frame::LidarPoint>         points;
        std::chrono::steady_clock::time_point nextRedraw = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point now;

        while (!st.stop_requested()) {
                if (this->inQueue.empty())
                        continue;

                points.push_back(this->inQueue.pop());

                if (points.size() > MAX_POINTS)
                        points.pop_front();

                if ((now = std::chrono::steady_clock::now()) < nextRedraw)
                        continue;

                std::fputs("plot '-' using 1:2 with points linestyle 1\n", file);
                for (const auto& point : points)
                        std::fprintf(
                                file,
                                "%s\n",
                                Plotter<frame::LidarPoint>::toString(point).c_str());
                std::fputs("e\n", file);
                std::fflush(file);

                nextRedraw = now + REDRAW_INTERVAL;
        }

        std::fputs("exit\n", file);
        pclose(file);
}

/**
 * IMU
 */

template <> std::string Plotter<frame::Imu>::toString(frame::Imu m) { return ""; }

template <> void Plotter<frame::Imu>::run(std::stop_token st) {}

} // namespace distributor
