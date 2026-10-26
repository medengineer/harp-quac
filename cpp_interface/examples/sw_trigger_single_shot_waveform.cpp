// Trigger four file-based waveforms across all analog output channels.
#include "common.hpp"

#include <atomic>
#include <string>

int main(int argc, char** argv) {
    constexpr int channels = 4;
    quac::Device device(com_port(argc, argv));

    std::cout << "Setting all active players to FILE\n";
    for (int i = 0; i < channels; ++i) {
        auto reply = device.write(kPlayers[i], quac::PlayerType::File);
        std::cout << " Read back: " << player_name(reply.payload) << " (" << type_name(reply.message_type) << ")";
        if (reply.timestamp) std::cout << ", time: " << *reply.timestamp;
        std::cout << "\n";
    }
    std::cout << "\n";

    for (int i = 0; i < channels; ++i) {
        quac::FileSettings settings;
        settings.cycles = 1;
        settings.duration = 0;
        settings.update_frequency = 500000;
        settings.set_path("channel_" + std::to_string(i) + ".bin");
        std::cout << "Applying FileSettings for waveform " << i << ".\n";
        auto reply = device.write(kFiles[i], settings);
        std::cout << "Read back: (" << type_name(reply.message_type) << ") " << reply.payload.path << "\n";
    }

    wait_ready(device, 0b1111);
    std::cout << "Channels are all ready.\n";

    std::atomic<int> left{channels};
    Flag all_finished;
    auto sub = device.subscribe(quac::DacFinished, [&](const harp::Reply<quac::AnalogOutputs>& msg) {
        std::cout << "DacFinished ";
        show_mask(msg);
        std::cout << "\n\n";
        if (--left <= 0) all_finished.set();
    });

    for (int i = 0; i < channels; ++i) {
        auto value = ao_mask(i);
        std::cout << "Writing: ";
        hex_byte(uint8_t(value));
        auto reply = device.write(quac::DacStart, value);
        std::cout << " Read back: ";
        show_mask(reply);
        std::cout << "\n";
        sleep_s(0.5);
    }

    std::cout << "Waiting for end-of-message replies\n";
    all_finished.wait();
    std::cout << "Done.\n";
}
