#include "manager.hpp"
#include "utility/logger.hpp"
#include <format>
#include <print>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
        std::string path = argv[1];

        if (path.empty())
                std::println("error: invalid arguments");

        logger::log(std::format("RECEIVED ARGS [{}]", path));

        manager::Manager application(path);

        auto result = application.run();
        if (!result)
                logger::log(result.error());
}

// #include <iostream>
// #include <boost/crc.hpp>

// int main(void)
// {
//         typedef boost::crc_optimal<16, 0x8005, 0xFFFF, 0, true, true> modbus_crc;

//         char pdu[] = "123456789";

//         modbus_crc crc;
//         crc.process_bytes(pdu, 9);

//         std::cout << std::hex << crc.checksum() << std::endl;
//         return 0;
// }
