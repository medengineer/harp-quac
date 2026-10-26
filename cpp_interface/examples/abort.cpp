// Trigger a sine waveform on one analog output channel and abort it shortly after.
#include "common.hpp"

int main(int argc, char** argv) {
    constexpr int channel = 0;
    const auto mask = ao_mask(channel);
    quac::Device device(com_port(argc, argv));

    std::cout << "Setting channel " << channel << " to SINE Player.\n";
    auto reply = device.write(kPlayers[channel], quac::PlayerType::Sine);
    std::cout << "  Read back: " << player_name(reply.payload) << ", time: " << reply.timestamp.value_or(0) << "\n";

    quac::SineSettings settings;
    settings.cycles = 1;
    settings.duration = 0;
    settings.update_frequency = 10000;
    settings.frequency = 1;
    settings.amplitude = 1;
    settings.vertical_shift = 0.5f;
    settings.normalized_phase_shift = 0;
    auto written = device.write(kSine[channel], settings);
    std::cout << "SineSettings[" << channel << "] written (" << type_name(written.message_type) << ")\n";

    std::cout << "Checking if channel is ready.\n";
    wait_ready(device, uint8_t(mask));
    std::cout << "Channel is ready.\n";

    std::cout << "Starting waveform.\n";
    auto started = device.write(quac::DacStart, mask);
    std::cout << " Read back: ";
    show_mask(started);
    std::cout << "\n";

    sleep_s(1.25);
    std::cout << "Aborting waveform.\n";
    auto aborted = device.write(quac::DacAbort, mask);
    std::cout << " Read back: ";
    show_mask(aborted);
    std::cout << "\nDisconnecting.\n";
}
