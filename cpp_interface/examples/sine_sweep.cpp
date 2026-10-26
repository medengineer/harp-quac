// Stepped logarithmic sine sweep on one analog output channel.
#include "common.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct Options {
    std::string port = "COM7";
    int channel = 0;
    double start_hz = 20;
    double stop_hz = 20000;
    int steps = 60;
    double dwell_ms = 100;
    double amplitude = 0.5;
    double offset = 0;
    int update_rate = 500000;
};

std::string need(int& i, int argc, char** argv, const char* flag) {
    if (i + 1 >= argc) {
        std::cerr << flag << " needs a value\n";
        std::exit(2);
    }
    return argv[++i];
}

Options parse(int argc, char** argv) {
    Options opt;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--port") opt.port = need(i, argc, argv, "--port");
        else if (arg == "--channel") opt.channel = std::stoi(need(i, argc, argv, "--channel"));
        else if (arg == "--start-hz") opt.start_hz = std::stod(need(i, argc, argv, "--start-hz"));
        else if (arg == "--stop-hz") opt.stop_hz = std::stod(need(i, argc, argv, "--stop-hz"));
        else if (arg == "--steps") opt.steps = std::stoi(need(i, argc, argv, "--steps"));
        else if (arg == "--dwell-ms") opt.dwell_ms = std::stod(need(i, argc, argv, "--dwell-ms"));
        else if (arg == "--amplitude") opt.amplitude = std::stod(need(i, argc, argv, "--amplitude"));
        else if (arg == "--offset") opt.offset = std::stod(need(i, argc, argv, "--offset"));
        else if (arg == "--update-rate") opt.update_rate = std::stoi(need(i, argc, argv, "--update-rate"));
        else {
            std::cerr << "unknown argument " << arg << "\n";
            std::exit(2);
        }
    }
    auto fail = [](const char* msg) { std::cerr << msg << "\n"; std::exit(2); };
    if (opt.channel < 0 || opt.channel > 3) fail("channel must be 0..3");
    if (opt.start_hz <= 0) fail("start-hz must be > 0");
    if (opt.stop_hz <= opt.start_hz) fail("stop-hz must be greater than start-hz");
    if (opt.steps < 2) fail("steps must be >= 2");
    if (opt.amplitude < 0 || opt.amplitude > 10) fail("amplitude must be between 0 and 10 V");
    if (std::abs(opt.offset) + opt.amplitude > 10) fail("offset ± amplitude must remain inside the QUAC ±10 V range");
    if (opt.stop_hz >= opt.update_rate / 2.0) fail("stop-hz must be below half the update rate");
    return opt;
}

std::vector<int> logspace(double start, double stop, int count) {
    std::vector<int> values;
    for (int i = 0; i < count; ++i) {
        int value = int(std::lround(start * std::pow(stop / start, double(i) / (count - 1))));
        if (std::find(values.begin(), values.end(), value) == values.end()) values.push_back(value);
    }
    return values;
}

}  // namespace

int main(int argc, char** argv) {
    const Options opt = parse(argc, argv);
    const auto frequencies = logspace(opt.start_hz, opt.stop_hz, opt.steps);
    const auto mask = ao_mask(opt.channel);

    std::cout << "Opening " << opt.port << "\n";
    std::cout << "AO" << opt.channel << ": " << frequencies.front() << " Hz -> " << frequencies.back()
              << " Hz, " << frequencies.size() << " steps\n";
    std::cout << "Amplitude: " << opt.amplitude << " V peak, offset: " << opt.offset << " V\n\n";

    quac::Device device(opt.port);
    device.write(kPlayers[opt.channel], quac::PlayerType::Sine);

    Flag finished;
    auto sub = device.subscribe(quac::DacFinished, [&](const harp::Reply<quac::AnalogOutputs>& msg) {
        if (uint8_t(msg.payload) & uint8_t(mask)) {
            finished.set();
            std::cout << "Waveform finished.\n";
        }
    });

    try {
        for (size_t index = 0; index < frequencies.size(); ++index) {
            const int frequency = frequencies[index];
            finished.clear();
            std::cout << "Applying settings for " << frequency << " [Hz] waveform.\n";

            quac::SineSettings settings;
            settings.cycles = 1;
            settings.duration = uint32_t(opt.dwell_ms * 1000);
            settings.update_frequency = uint32_t(opt.update_rate);
            settings.frequency = uint32_t(frequency);
            settings.amplitude = float(opt.amplitude);
            settings.vertical_shift = float(opt.offset);
            settings.normalized_phase_shift = 0;
            device.write(kSine[opt.channel], settings);

            std::cout << "Waiting for device to become ready.\n";
            bool ready = false;
            for (int attempt = 0; attempt < 1000; ++attempt) {
                if (uint8_t(device.read(quac::DacReady).payload) & uint8_t(mask)) {
                    ready = true;
                    break;
                }
                sleep_s(0.001);
            }
            if (!ready) throw std::runtime_error("channel did not become ready");

            std::cout << index + 1 << "/" << frequencies.size() << "   " << frequency << " Hz\n";
            std::cout << "Starting channel output.\n";
            device.write(quac::DacStart, mask);
            sleep_s(opt.dwell_ms / 1000.0 + 0.1);
        }
    } catch (const std::exception& error) {
        std::cout << "\nAborting sweep... " << error.what() << "\n";
        try {
            device.write(quac::DacAbort, mask);
            device.write(kAo[opt.channel], 0.f);
        } catch (...) {
        }
    }

    std::cout << "\nSweep complete; output returned to 0 V.\n";
}
