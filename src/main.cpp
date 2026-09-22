#include "manager.hpp"
#include "utility/logger.hpp"
#include <print>

int main(int argc, char* argv[])
{
        std::string path     = argv[1];
        std::string baudRate = argv[2];

        if (!(path.empty() && baudRate.empty()))
                std::println("invalid arguments");

        manager::Manager m(path + " " + baudRate);

        if (auto result = m.run(); !result.has_value())
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
