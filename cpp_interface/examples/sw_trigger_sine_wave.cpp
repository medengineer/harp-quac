// Trigger a sine waveform on one analog output channel and wait until it finishes.
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
    settings.duration = 3000000;
    settings.update_frequency = 10000;
    settings.frequency = 1;
    settings.amplitude = 2.5f;
    settings.vertical_shift = 1.25f;
    settings.normalized_phase_shift = 0;
    auto written = device.write(kSine[channel], settings);
    std::cout << "SineSettings[" << channel << "] written (" << type_name(written.message_type) << ")\n\n";

    Flag finished;
    auto sub = device.subscribe(quac::DacFinished, [&](const harp::Reply<quac::AnalogOutputs>& msg) {
        std::cout << "DacFinished ";
        show_mask(msg);
        std::cout << "\n\n";
        finished.set();
    });

    std::cout << "Starting waveform.\n";
    auto started = device.write(quac::DacStart, mask);
    std::cout << " Read back: ";
    show_mask(started);
    std::cout << "\nWaiting for end-of-waveform event.\n";
    finished.wait();
    std::cout << "Disconnecting.\n";
}
