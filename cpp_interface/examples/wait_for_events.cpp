// Subscribe to and print all Harp events from the device.
#include "common.hpp"

#include <string>

int main(int argc, char** argv) {
    quac::Device device(com_port(argc, argv));
    auto sub = device.subscribe_all([](const harp::RawMessage& msg) {
        std::cout << "HarpMessage(type=" << type_name(msg.type) << ", address=" << int(msg.address) << ", payload=";
        for (uint8_t byte : msg.payload) {
            hex_byte(byte);
            std::cout << " ";
        }
        if (msg.timestamp) std::cout << "t=" << *msg.timestamp;
        std::cout << ")\n\n";
    });

    std::cout << "Waiting for events. Press Enter to disconnect.\n";
    std::string line;
    std::getline(std::cin, line);
    sub.unsubscribe();
    std::cout << "Disconnecting.\n";
}
