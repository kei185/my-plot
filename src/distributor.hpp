#pragma once
#include "frame.hpp"
#include "transmitter.hpp"
#include "utility/logger.hpp"
#include "xqueue.hpp"
#include <chrono>
#include <cstdio>
#include <deque>
#include <stop_token>
#include <string>

namespace distributor
{

struct Distributor
{
        virtual void run(std::stop_token) = 0;
        virtual ~Distributor()            = default;
};

struct DeviceController : Distributor
{
        frame::Type                          type;
        transmitter::Transmitter&            transmitter;
        xqueue::Queue<frame::systemMessage>& inQueue;

        void run(std::stop_token) override;

        DeviceController(
                frame::Type,
                transmitter::Transmitter&,
                xqueue::Queue<frame::systemMessage>&);
};

template <typename T> struct Plotter : Distributor
{
        frame::Type       type;
        xqueue::Queue<T>& inQueue;

        Plotter(frame::Type type, xqueue::Queue<T>& inQueue) : type(type), inQueue(inQueue) {}
        ~Plotter() {}

        void               run(std::stop_token) override;
        static std::string toString(T);
};

template <typename T> void Plotter<T>::run(std::stop_token st)
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
        std::deque<T>                         points;
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
                        std::fprintf(file, "%s\n", Plotter<T>::toString(point).c_str());
                std::fputs("e\n", file);
                std::fflush(file);

                nextRedraw = now + REDRAW_INTERVAL;
        }

        std::fputs("exit\n", file);
        pclose(file);
}

} // namespace distributor
