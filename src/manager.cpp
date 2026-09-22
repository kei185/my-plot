#include <cstddef>
#include <expected>
#include <memory>
#include <queue>
#include <unistd.h>
#include <map>

#include "distributor.hpp"
#include "error.hpp"
#include "parser.hpp"
#include "frame.hpp"
#include "receiver.hpp"
#include "transmitter.hpp"
#include "io.hpp"
#include "utility/logger.hpp"
#include "utility/unwrap.hpp"
#include "manager.hpp"

using namespace error;

namespace manager
{
DataStreams::DataStreams()
    : system(std::make_unique<std::queue<frame::systemMessage>>()),
      lidar(std::make_unique<std::queue<frame::LidarPoint>>())
{}

Manager::Manager(const std::string file)
    : port(file), frameStreams(std::make_unique<std::map<frame::Type, std::queue<frame::Frame>>>()),
      dataStreams(DataStreams()), transmitter(), receiverWorker(), parsers(), distributors()
{
        // prepare transmitter
        this->transmitter = std::make_unique<transmitter::Transmitter>(this->port);
        logger::log("TRANSMITTER DISPATCHED");

        // prepare frame streams
        for (auto type : frame::TYPES)
                this->frameStreams->emplace(type, std::queue<frame::Frame>());

        // prepare receiver
        this->receiverWorker.instance =
                std::make_unique<receiver::Receiver>(this->port, *this->frameStreams);

        logger::log("RECEIVER DISPATCHED");

        // prepare parsers
        unwrap(Manager::initParsers(this->parsers, *this->frameStreams, this->dataStreams));

        logger::log("PARSERS DISPATCHED");

        // prepare distributors
        unwrap(Manager::initDistributors(
                this->distributors,
                this->dataStreams,
                *this->transmitter));

        logger::log("DISTRIBUTORS DISPATCHED");
}

Manager::~Manager()
{
        this->receiverWorker.thread.request_stop();

        for (auto& [_, p] : this->parsers)
                if (auto _result = p.abort(); !_result.has_value())
                        logger::log(_result.error());

        for (auto& [_, d] : this->distributors)
                if (auto _result = d.abort(); !_result.has_value())
                        logger::log(_result.error());
}

std::expected<void, Error> Manager::run()
{
        if (auto _result = this->receiverWorker.dispatch(); !_result.has_value())
                return _result;

        for (auto& [_, p] : this->parsers)
                if (auto _result = p.dispatch(); !_result.has_value())
                        return _result;

        for (auto& [_, d] : this->distributors)
                if (auto _result = d.dispatch(); !_result.has_value())
                        return _result;

        this->receiverWorker.thread.join();

        return {};
}

std::expected<void, Error> Manager::initParsers(
        std::map<frame::Type, ParserWorker>&             parsers,
        std::map<frame::Type, std::queue<frame::Frame>>& frameStreams,
        DataStreams&                                     streams)
{
        for (auto type : frame::TYPES)
                if (auto [it, success] = parsers.try_emplace(type); !success)
                        return std::unexpected<Error>(Error::PARSER_INIT_FAILED);

        frame::Type type;

        type                   = frame::Type::SYSTEM;
        parsers[type].instance = std::make_unique<parser::Parser<frame::systemMessage>>(
                type,
                frameStreams[type],
                *streams.system);

        type                   = frame::Type::LIDAR;
        parsers[type].instance = std::make_unique<parser::Parser<frame::LidarPoint>>(
                type,
                frameStreams[type],
                *streams.lidar);

        return {};
}

std::expected<void, Error> Manager::initDistributors(
        std::map<frame::Type, DistributorWorker>& distributors,
        DataStreams&                              streams,
        transmitter::Transmitter&                 transmitter)
{

        for (auto type : frame::TYPES)
                if (auto [it, success] = distributors.try_emplace(type); !success)
                        return std::unexpected<Error>(Error::DISTRIBUTOR_INIT_FAILED);

        frame::Type type;

        type = frame::Type::SYSTEM;
        distributors[type].instance =
                std::make_unique<distributor::DeviceController>(type, transmitter, *streams.system);

        type = frame::Type::LIDAR;
        distributors[type].instance =
                std::make_unique<distributor::Plotter<frame::LidarPoint>>(type, *streams.lidar);

        return {};
}

} // namespace manager
