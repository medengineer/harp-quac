// Test mute/unmute on DI0. Wire DI0 to DO0 so the digital output drives blanking.
#include "common.hpp"

int main(int argc, char** argv) {
    constexpr int channel = 0;
    const auto mask = ao_mask(channel);
    quac::Device device(com_port(argc, argv));

    device.write(quac::DOPortState, quac::DigitalOutputs::DO0);  // start muted
    std::cout << "Setting DI0 to BLANKING and assigning it to AO0.\n";
    device.write(quac::PinModeDI0, quac::DIPinMode::Blanking);
    device.write(quac::AOChannelsAssignedToDI0, quac::DigitalInputs::DI0);

    std::cout << "Setting channel " << channel << " to SINE Player.\n";
    auto reply = device.write(kPlayers[channel], quac::PlayerType::Sine);
    std::cout << "  Read back: " << player_name(reply.payload) << ", time: " << reply.timestamp.value_or(0) << "\n";

    quac::SineSettings settings;
    settings.cycles = 1;
    settings.duration = 5000000;
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
    std::cout << "\n";

    sleep_s(0.5);
    for (int i = 0; i < 3; ++i) {
        device.write(quac::DOPortState, quac::DigitalOutputs::None);  // unmute
        sleep_s(0.25);
        device.write(quac::DOPortState, quac::DigitalOutputs::DO0);  // mute
        sleep_s(0.25);
    }
    device.write(quac::DOPortState, quac::DigitalOutputs::None);

    std::cout << "Waiting for end-of-waveform event.\n";
    finished.wait();
    std::cout << "Disconnecting.\n";
}
