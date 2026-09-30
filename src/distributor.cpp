
#include "protocol.hpp"
#include "distributor.hpp"
#include "frame.hpp"
#include "xqueue.hpp"
#include "utility/logger.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <deque>
#include <format>
#include <stop_token>
#include <thread>

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

template <> std::string Plotter<frame::Imu>::toString(frame::Imu m)
{
        return std::format(
                "{} {} {} {} {} {}",
                m.trans.x,
                m.trans.y,
                m.trans.z,
                m.rot.x,
                m.rot.y,
                m.rot.z);
}

/* Static 3D plane, kept for later rotation support.
template <> void Plotter<frame::Imu>::run(std::stop_token st)
{
        if (st.stop_requested())
                return;

        FILE* file = popen("gnuplot -persist", "w");

        if (file == nullptr) {
                logger::log("OPEN PROCESS gnuplot FAILED");
                return;
        }

        std::fputs(
                "set title 'IMU plane'\n"
                "set xlabel 'X'\n"
                "set ylabel 'Y'\n"
                "set zlabel 'Z'\n"
                "set xrange [-1.5:1.5]\n"
                "set yrange [-1.5:1.5]\n"
                "set zrange [-1.5:1.5]\n"
                "set cbrange [-1:1]\n"
                "set view 60, 30\n"
                "set view equal xyz\n"
                "set xyplane at 0\n"
                "unset key\n"
                "unset colorbox\n"
                "set pm3d depthorder border linecolor rgb '#00749A'\n"
                "splot '-' using 1:2:3 with pm3d fillcolor rgb '#00AEEF'\n"
                "-1 -1 0\n"
                "1 -1 0\n"
                "\n"
                "-1 1 0\n"
                "1 1 0\n"
                "e\n",
                file);
        std::fflush(file);

        std::fputs("exit\n", file);
        pclose(file);
}
*/

template <> void Plotter<frame::Imu>::run(std::stop_token st)
{
        if (st.stop_requested())
                return;

        FILE* file = popen("gnuplot -persist", "w");

        if (file == nullptr) {
                logger::log("OPEN PROCESS gnuplot FAILED");
                return;
        }

        std::fputs(
                "set title 'IMU time series'\n"
                "set xlabel 'Elapsed time [s]'\n"
                "set format x '%.2f'\n"
                "set ylabel 'Translation raw value'\n"
                "set y2label 'Rotation raw value'\n"
                "set ytics nomirror\n"
                "set y2tics\n"
                "set grid\n"
                "set key outside top center horizontal maxrows 2\n"
                "set lmargin 10\n"
                "set rmargin 10\n"
                "set style data linespoints\n"
                "set style line 1 linecolor rgb '#00AEEF' linewidth 1.5 pointtype 7 pointsize 0.3\n"
                "set style line 2 linecolor rgb '#F28E2B' linewidth 1.5 pointtype 7 pointsize 0.3\n"
                "set style line 3 linecolor rgb '#59A14F' linewidth 1.5 pointtype 7 pointsize "
                "0.3\n"
                "set style line 4 linecolor rgb '#00AEEF' linewidth 1.5 dashtype 2\n"
                "set style line 5 linecolor rgb '#F28E2B' linewidth 1.5 dashtype 2\n"
                "set style line 6 linecolor rgb '#59A14F' linewidth 1.5 dashtype 2\n",
                file);

        constexpr size_t MAX_POINTS      = 1000;
        constexpr auto   REDRAW_INTERVAL = std::chrono::milliseconds(50);

        struct Sample
        {
                double     time;
                frame::Imu value;
        };

        std::deque<Sample> points;
        const auto         start      = std::chrono::steady_clock::now();
        auto               nextRedraw = start;
        bool               dirty      = false;

        while (!st.stop_requested()) {
                if (!this->inQueue.empty()) {
                        auto value = this->inQueue.pop();
                        // Imu has no device timestamp; use elapsed host time when dequeued.
                        const double time = std::chrono::duration<double>(
                                                    std::chrono::steady_clock::now() - start)
                                                    .count();
                        points.push_back({time, value});
                        if (points.size() > MAX_POINTS)
                                points.pop_front();
                        dirty = true;
                } else {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }

                const auto now = std::chrono::steady_clock::now();
                if (!dirty || now < nextRedraw)
                        continue;

                auto transMin = points.front().value.trans.x;
                auto transMax = transMin;
                auto rotMin   = points.front().value.rot.x;
                auto rotMax   = rotMin;

                std::fputs("$imu << EOD\n", file);
                for (const auto& point : points) {
                        const auto& t = point.value.trans;
                        const auto& r = point.value.rot;
                        transMin      = std::min({transMin, t.x, t.y, t.z});
                        transMax      = std::max({transMax, t.x, t.y, t.z});
                        rotMin        = std::min({rotMin, r.x, r.y, r.z});
                        rotMax        = std::max({rotMax, r.x, r.y, r.z});
                        std::fprintf(file, "%.6f %s\n", point.time, toString(point.value).c_str());
                }
                std::fputs("EOD\n", file);

                // Keep flat signals visible and avoid an empty range, including all-zero data.
                const double transPadding = std::max(1.0, (transMax - transMin) * 0.05);
                const double rotPadding   = std::max(1.0, (rotMax - rotMin) * 0.05);
                std::fprintf(
                        file,
                        "set xrange [%.6f:%.6f]\n",
                        points.front().time,
                        std::max(points.back().time, points.front().time + 0.05));
                std::fprintf(
                        file,
                        "set yrange [%f:%f]\n"
                        "set y2range [%f:%f]\n"
                        // The Qt terminal draws multiplot panels one at a time, which exposes
                        // a blank canvas during live redraws. Two Y axes keep both scales in
                        // one plot so each update is presented as a single frame.
                        "plot $imu using 1:2 axes x1y1 title 'Translation X' linestyle 1, "
                        "$imu using 1:3 axes x1y1 title 'Translation Y' linestyle 2, "
                        "$imu using 1:4 axes x1y1 title 'Translation Z' linestyle 3, "
                        "$imu using 1:5 axes x1y2 title 'Rotation X' linestyle 4, "
                        "$imu using 1:6 axes x1y2 title 'Rotation Y' linestyle 5, "
                        "$imu using 1:7 axes x1y2 title 'Rotation Z' linestyle 6\n",
                        transMin - transPadding,
                        transMax + transPadding,
                        rotMin - rotPadding,
                        rotMax + rotPadding);
                std::fflush(file);

                dirty      = false;
                nextRedraw = now + REDRAW_INTERVAL;
        }

        std::fputs("exit\n", file);
        pclose(file);
}

} // namespace distributor
