// Write a random voltage to each analog output channel, then reset to midscale.
#include "common.hpp"

#include <iomanip>
#include <random>

int main(int argc, char** argv) {
    quac::Device device(com_port(argc, argv));
    std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<float> volts(-10.f, 10.f);
    std::cout << std::fixed << std::setprecision(3);

    for (int i = 0; i < 4; ++i) {
        auto initial = device.read(kAo[i]);
        const float value = volts(rng);
        std::cout << " AO[" << i << "] initial value: " << initial.payload << "\n";
        std::cout << "Writing: " << value << "[V] ";
        auto reply = device.write(kAo[i], value);
        std::cout << "| result: " << reply.payload << " (" << type_name(reply.message_type) << ")\n\n";
        sleep_s(0.5);
    }

    std::cout << "Resetting all Analog Output Waveforms to midscale.\n";
    for (int i = 0; i < 4; ++i) {
        auto reply = device.write(kAo[i], 0.f);
        std::cout << "  result: " << reply.payload << " (" << type_name(reply.message_type) << ")\n";
    }
    std::cout << "Disconnecting.\n";
}
