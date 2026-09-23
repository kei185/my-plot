#pragma once
#include <stop_token>
#include <utility>
#include <vector>

#include "frame.hpp"
#include "xqueue.hpp"

namespace parser
{
struct ParserBase
{
        virtual void run(std::stop_token) = 0;
        virtual ~ParserBase()             = default;
};

template <typename T> struct Parser : public ParserBase
{
        frame::Type                  type;
        xqueue::Queue<frame::Frame>& inQueue;
        xqueue::Queue<T>&            outQueue;

        Parser(frame::Type type, xqueue::Queue<frame::Frame>& inQueue, xqueue::Queue<T>& outQueue)
            : type(type), inQueue(inQueue), outQueue(outQueue)
        {}

        void                  run(std::stop_token) override;
        static std::vector<T> parsePayload(frame::Frame& fr);
};

template <typename T> void Parser<T>::run(std::stop_token st)
{
        while (1) {
                if (st.stop_requested())
                        return;

                if (this->inQueue.empty())
                        continue;

                frame::Frame fr = this->inQueue.pop();

                for (auto& data : Parser<T>::parsePayload(fr))
                        this->outQueue.push(std::move(data));
        }
};

} // namespace parser
