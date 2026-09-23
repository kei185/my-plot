#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <format>
#include <span>
#include <unistd.h>
#include <string>
#include <termios.h>
#include <fcntl.h>
#include "io.hpp"
#include "utility/logger.hpp"

namespace io
{
static int BAUD_RATE = B230400;

Port::Port(std::string path)
{

        logger::log(std::format("TRY OPEN [{}]", path));

        this->fd = open((char*)path.data(), O_RDWR | O_NOCTTY);

        if (this->fd == -1) {
                const int openError = errno;
                logger::log(std::format("failed to open {}: errno={}", path, openError));
                std::exit(openError);
        }

        logger::log(std::format(" [{}] OPENED", path));

        this->tty = {};

        cfmakeraw(&this->tty);
        cfsetispeed(&this->tty, BAUD_RATE);
        cfsetospeed(&this->tty, BAUD_RATE);
        // 1文字8bit
        tty.c_cflag &= ~CSIZE;
        this->tty.c_cflag |= CS8;
        // 読み込み
        this->tty.c_cflag |= CREAD;
        // パリティビットなし
        this->tty.c_cflag &= ~PARENB;
        // stop bit 1bit
        this->tty.c_cflag &= ~CSTOPB;
        this->tty.c_cflag &= ~CLOCAL;
        // 最低1文字読み出し
        tty.c_cc[VMIN] = 1;
        // タイムアウトなし
        tty.c_cc[VTIME] = 0;
        tcsetattr(this->fd, TCSANOW, &this->tty);

        // // ブロッキングに戻す
        // int flag = fcntl(this->fd, F_GETFL, 0);
        // fcntl(this->fd, F_SETFL, flag & ~O_NONBLOCK);
}

Port::~Port()
{
        if (this->fd >= 0)
                close(this->fd);
}

std::expected<void, error::Error> Port::readRaw(std::span<uint8_t> bytes)
{
        ssize_t readSize = 0;
        for (size_t offset = 0; offset < bytes.size(); offset += static_cast<size_t>(readSize)) {

                readSize = read(this->fd, bytes.data() + offset, bytes.size() - offset);

                if (readSize > 0)
                        continue;

                if (readSize < 0 && errno == EINTR) {
                        readSize = 0;
                        continue;
                }

                return std::unexpected(error::Error::IO_READ_FAILED);
        }

        return {};
}

std::expected<void, error::Error> Port::writeRaw(std::span<const uint8_t> bytes)
{
        ssize_t writtenSize = 0;
        for (size_t offset = 0; offset < bytes.size(); offset += static_cast<size_t>(writtenSize)) {

                writtenSize = write(this->fd, bytes.data() + offset, bytes.size() - offset);

                if (writtenSize > 0)
                        continue;

                if (writtenSize < 0 && errno == EINTR) {
                        writtenSize = 0;
                        continue;
                }

                return std::unexpected(error::Error::IO_WRITE_FAILED);
        }

        // if (fsync(this->fd) < 0)
        //         return std::unexpected(error::Error::IO_WRITE_FAILED);

        return {};
}

/**
 * decodes bytes reading as  a little endian byte series
 * this is necessary for the reason below.
 *
 * B[1], B[2], B[3], B[4] are read in reverse way
 * for instance, these are read as blow when successive byte access:
 *      read((uint32_t)n, 4);
 * it gets [B[1],  B[2] , ..] as [4th byte, 3rd byte,...].
 * but we want to read as [B[4],  B[3] , ..].
 * so read it a byte basis here.
 */
uint16_t decodeBigEndian(std::span<const uint8_t, 2> bytes)
{
        return (static_cast<uint16_t>(bytes[0]) << 8) | static_cast<uint16_t>(bytes[1]);
}

uint32_t decodeBigEndian(std::span<const uint8_t, 4> bytes)
{
        return (static_cast<uint32_t>(bytes[0]) << 24) | (static_cast<uint32_t>(bytes[1]) << 16) |
               (static_cast<uint32_t>(bytes[2]) << 8) | static_cast<uint32_t>(bytes[3]);
}

} // namespace io
