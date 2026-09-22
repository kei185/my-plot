

#include <print>
#include <queue>
#include <stop_token>
#include <string>
#include <iostream>
#include "frame.hpp"
#include "utility/logger.hpp"
#include "utility/unwrap.hpp"
#include "transmitter.hpp"
#include "protocol.hpp"

namespace protocol
{

static void waitforReady(std::stop_token& st, std::queue<frame::systemMessage>& mQueue)
{
        frame::systemMessage sm;
        do {
                if (st.stop_requested())
                        return;

                if (mQueue.empty())
                        continue;

                sm = mQueue.front();

                logger::log(sm.message);

                mQueue.pop();
        } while (sm.type != frame::Type::READY);
}

void run(
        std::stop_token                   st,
        transmitter::Transmitter&         transmitter,
        std::queue<frame::systemMessage>& mQueue)
{
        waitforReady(st, mQueue);
        if (st.stop_requested())
                return;

        std::println("press ENTER to start scan");
        std::string s;
        std::getline(std::cin, s);

        logger::log("SCAN STARTED");
        logger::log("SCAN STARTEDってこと!?");

        unwrap(transmitter.request(st, frame::OperationType::START_SCAN, mQueue));
};

} // namespace protocol
