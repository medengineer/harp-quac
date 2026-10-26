// Retrigger a short sine waveform 1000 times, waiting for each DacFinished event.
#include "common.hpp"

#include <random>
#include <stdexcept>

int main(int argc, char** argv) {
    constexpr int channel = 0;
    constexpr int trials = 1000;
    const auto mask = ao_mask(channel);
    quac::Device device(com_port(argc, argv));

    std::cout << "Setting channel " << channel << " to SINE Player.\n";
    auto reply = device.write(kPlayers[channel], quac::PlayerType::Sine);
    std::cout << "  Read back: " << player_name(reply.payload) << ", time: " << reply.timestamp.value_or(0) << "\n";

    quac::SineSettings settings;
    settings.cycles = 1;
    settings.duration = 100000;
    settings.update_frequency = 500000;
    settings.frequency = 10;
    settings.amplitude = 2.5f;
    settings.vertical_shift = 1.25f;
    settings.normalized_phase_shift = 0.5f;

    Flag finished;
    auto sub = device.subscribe(quac::DacFinished, [&](const harp::Reply<quac::AnalogOutputs>&) { finished.set(); });

    std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<double> gap(0, 0.1);

    for (int trial = 1; trial <= trials; ++trial) {
        std::cout << "Trial " << trial << "/" << trials << "\n";
        finished.clear();

        std::cout << "Applying settings.\n";
        auto written = device.write(kSine[channel], settings);
        std::cout << "SineSettings[" << channel << "] written (" << type_name(written.message_type) << ")\n\n";

        std::cout << "Waiting for device to be ready.\n";
        uint8_t ready = 0;
        for (int attempt = 0; attempt < 1000; ++attempt) {
            ready = uint8_t(device.read(quac::DacReady).payload);
            if (ready & uint8_t(mask)) break;
            sleep_s(0.001);
        }
        if (!(ready & uint8_t(mask))) throw std::runtime_error("AO0 never became ready");

        std::cout << "Starting waveform.\n";
        auto started = device.write(quac::DacStart, mask);
        std::cout << " Read back: ";
        show_mask(started);
        std::cout << "\nWaiting for end-of-waveform event.\n";
        if (!finished.wait_for(std::chrono::duration<double>(settings.duration / 1e6 + 0.1)))
            throw std::runtime_error("Waveform not finished");
        sleep_s(gap(rng));
        std::cout << "\n";
    }
    std::cout << "Disconnecting.\n";
}
