#include "quac.hpp"

#include <iostream>
#include <string>

namespace {

const char* name(quac::PlayerType type) {
    switch (type) {
        case quac::PlayerType::File: return "FILE";
        case quac::PlayerType::Sine: return "SINE";
        case quac::PlayerType::Trapezoid: return "TRAPEZOID";
    }
    return "UNKNOWN";
}

}  // namespace

int main(int argc, char** argv) {
    const std::string port = argc > 1 ? argv[1] : "COM7";
    quac::Device device(port);
    std::cout << quac::DEVICE_NAME << " WhoAmI " << device.who_am_i() << "\n";

    constexpr harp::Register<quac::PlayerType> players[] = {
        quac::ActivePlayer0, quac::ActivePlayer1, quac::ActivePlayer2, quac::ActivePlayer3};
    for (int i = 0; i < 4; ++i) {
        auto reply = device.read(players[i]);
        std::cout << "ActivePlayer" << i << ": " << name(reply.payload);
        if (reply.timestamp) std::cout << " t=" << *reply.timestamp;
        std::cout << "\n";
    }

    auto outputs = device.read(quac::DOPortState);
    auto inputs = device.read(quac::DIPortState);
    std::cout << "DOPortState: 0x" << std::hex << int(outputs.payload) << "\n";
    std::cout << "DIPortState: 0x" << int(inputs.payload) << std::dec << "\n";
}
