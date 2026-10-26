// Trigger a trapezoid waveform on one analog output channel.
#include "common.hpp"

int main(int argc, char** argv) {
    constexpr int channel = 0;
    const auto mask = ao_mask(channel);
    quac::Device device(com_port(argc, argv));

    std::cout << "Setting channel " << channel << " to TRAPEZOID Player.\n";
    auto reply = device.write(kPlayers[channel], quac::PlayerType::Trapezoid);
    std::cout << "  Read back: " << player_name(reply.payload) << ", time: " << reply.timestamp.value_or(0) << "\n";

    quac::TrapezoidSettings settings;
    settings.cycles = 1;
    settings.duration = 1000000;
    settings.update_frequency = 10000;
    settings.frequency = 2;
    settings.amplitude = 5;
    settings.vertical_shift = 2.5f;
    settings.normalized_phase_shift = 0;
    settings.ramp_on_duration = 50000;
    settings.pulse_width_duration = 150000;
    settings.ramp_off_duration = 50000;
    auto written = device.write(kTrapezoid[channel], settings);
    std::cout << "TrapezoidSettings[" << channel << "] written (" << type_name(written.message_type) << ")\n";

    std::cout << "Waiting until channel " << channel << " is ready.\n";
    wait_ready(device, uint8_t(mask));
    std::cout << "Channel[" << channel << "] is ready.\n";

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
    std::cout << "Disconnecting\n";
}
