
#include "plot.hpp"
#include "xqueue.hpp"
#include <cmath>
#include <format>
#include <string>

namespace plot
{

class Point
{
        Point(int r, int angle)
        {
                this->x = r * std::cos(angle);
                this->y = r * std::sin(angle);
        }

        int x;
        int y;

        std::string stringify() { return std::format("{} {}\r\n", this->x, this->y); }
};

void start(xqueue::Queue<Point>& points) {}

} // namespace plot
