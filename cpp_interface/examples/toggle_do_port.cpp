// Toggle each digital output line on the Quac in turn.
#include "common.hpp"

int main(int argc, char** argv) {
    quac::Device device(com_port(argc, argv));
    for (int i = 0; i < 4; ++i) {
        auto value = quac::DigitalOutputs(uint8_t(1u << i));
        std::cout << "Writing: ";
        hex_byte(uint8_t(value));
        auto reply = device.write(quac::DOPortState, value);
        std::cout << " Read back: ";
        hex_byte(uint8_t(reply.payload));
        std::cout << "\n";
        sleep_s(0.5);
    }
    std::cout << "Setting all Digital outputs to 0.\n";
    device.write(quac::DOPortState, quac::DigitalOutputs::None);
    std::cout << "Disconnecting.\n";
}
