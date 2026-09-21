#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <print>
#include <stop_token>
#include <thread>
#include <unistd.h>
#include <cstdlib>
#include <span>
#include <vector>

#include <boost/crc.hpp>

#include "receiver.hpp"
#include "error.hpp"
#include "frame.hpp"
#include "io.hpp"

using namespace error;

namespace receiver
{

inline void abortByError(Error err)
{
        std::println("{}: {}", std::this_thread::get_id(), toString(err));
        // TODO 必要なら復帰プロセスを考える
        std::exit(1);
}

template <typename T> T unwrap(const std::expected<T, Error>&& result)
{
        if (!result.has_value())
                abortByError(result.error());

        return result.value();
}

Receiver::Receiver(io::Port& port, std::map<frame::Type, std::queue<frame::Frame>>& frameStreams)
    : port(port), frameStreams(frameStreams)
{}

void Receiver::run(std::stop_token st)
{
        std::expected<frame::FrameHeader, Error> frh;
        frame::Frame                             fr;

        while (1) {
                unwrap(this->findSOF(st, this->port));

                frh = this->getFrameHeader(this->port);

                if (!frh.has_value()) {
                        if (frh.error() == error::Error::INVALID_CRC)
                                continue;
                        else
                                abortByError(frh.error());
                }

                fr = unwrap(this->getPayload(this->port, frh.value()));

                this->frameStreams[fr.type].push(fr);
        }

        return;
}

std::expected<void, Error> Receiver::findSOF(std::stop_token& st, io::Port& port)
{
        std::array<uint8_t, 1> byte = {};

        while (1) {
                if (st.stop_requested())
                        return {};

                unwrap(port.readRaw(byte));

                if (byte[0] != frame::START_OF_FRAME[0])
                        continue;

                if (auto result = port.readRaw(byte); !result)
                        return std::unexpected<Error>(result.error());

                if (byte[0] != frame::START_OF_FRAME[1])
                        continue;

                break;
        }

        return {};
};

std::expected<frame::FrameHeader, Error> Receiver::getFrameHeader(io::Port& port)
{
        std::array<uint8_t, frame::RAW_HEADER_SIZE> rawHeader = {};

        auto result = port.readRaw(rawHeader);
        if (!result)
                return std::unexpected<Error>(result.error());

        if (!Receiver::isValidCRC(rawHeader))
                return std::unexpected<Error>(error::Error::INVALID_CRC);

        // TODO: low priority fix hard code
        frame::FrameHeader frh = {
                .length    = io::decodeBigEndian(std::span<const uint8_t, 2>(rawHeader.data(), 2)),
                .type      = static_cast<frame::Type>(rawHeader[frame::TYPE_OFFSET]),
                .timestamp = io::decodeBigEndian(
                        std::span<const uint8_t, 4>(rawHeader.data() + frame::TIMESTAMP_OFFSET, 4)),
        };

        return frh;
}

std::expected<frame::Frame, Error>
Receiver::getPayload(io::Port& port, const frame::FrameHeader& frh)
{
        frame::Frame fr;
        fr.length  = frh.length;
        fr.type    = frh.type;
        fr.payload = std::vector<uint8_t>(fr.length);

        auto result = port.readRaw(std::span<uint8_t>(fr.payload));
        if (!result.has_value())
                return std::unexpected<Error>(result.error());

        return fr;
}

bool Receiver::isValidCRC(std::span<const uint8_t, frame::RAW_HEADER_SIZE> rawHeader)
{
        const frame::Crc expected = rawHeader[frame::CRC_OFFSET];

        boost::crc_optimal<8, 0x31, 0, 0, false, false> crc;

        crc.process_bytes(rawHeader.data(), frame::CRC_OFFSET);

        return crc.checksum() == expected;
}

} // namespace receiver
