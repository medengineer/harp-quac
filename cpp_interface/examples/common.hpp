#pragma once

#include "quac.hpp"

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

inline constexpr harp::Register<quac::PlayerType> kPlayers[] = {
    quac::ActivePlayer0, quac::ActivePlayer1, quac::ActivePlayer2, quac::ActivePlayer3};
inline constexpr harp::Register<quac::SineSettings> kSine[] = {
    quac::SineSettings0, quac::SineSettings1, quac::SineSettings2, quac::SineSettings3};
inline constexpr harp::Register<quac::TrapezoidSettings> kTrapezoid[] = {
    quac::TrapezoidSettings0, quac::TrapezoidSettings1, quac::TrapezoidSettings2, quac::TrapezoidSettings3};
inline constexpr harp::Register<quac::FileSettings> kFiles[] = {
    quac::FileSettings0, quac::FileSettings1, quac::FileSettings2, quac::FileSettings3};
inline constexpr harp::Register<float> kAo[] = {quac::AOChannel0, quac::AOChannel1, quac::AOChannel2, quac::AOChannel3};

inline std::string com_port(int argc, char** argv) { return argc > 1 ? argv[1] : "COM7"; }

inline const char* player_name(quac::PlayerType type) {
    switch (type) {
        case quac::PlayerType::File: return "FILE";
        case quac::PlayerType::Sine: return "SINE";
        case quac::PlayerType::Trapezoid: return "TRAPEZOID";
    }
    return "UNKNOWN";
}

inline const char* type_name(harp::MessageType type) {
    switch (type) {
        case harp::MessageType::Read: return "Read";
        case harp::MessageType::Write: return "Write";
        case harp::MessageType::Event: return "Event";
    }
    return "?";
}

inline quac::AnalogOutputs ao_mask(int channel) { return quac::AnalogOutputs(uint8_t(1u << channel)); }

inline void hex_byte(uint8_t value) {
    std::cout << "0x" << std::hex << int(value) << std::dec;
}

template <class T>
void show_mask(const harp::Reply<T>& reply) {
    hex_byte(uint8_t(reply.payload));
    std::cout << " (" << type_name(reply.message_type) << ")";
    if (reply.timestamp) std::cout << ", time: " << *reply.timestamp;
}

class Flag {
public:
    void set() {
        std::lock_guard lock(mu_);
        ready_ = true;
        cv_.notify_all();
    }
    void clear() {
        std::lock_guard lock(mu_);
        ready_ = false;
    }
    void wait() {
        std::unique_lock lock(mu_);
        cv_.wait(lock, [&] { return ready_; });
    }
    bool wait_for(std::chrono::duration<double> timeout) {
        std::unique_lock lock(mu_);
        return cv_.wait_for(lock, timeout, [&] { return ready_; });
    }

private:
    std::mutex mu_;
    std::condition_variable cv_;
    bool ready_ = false;
};

inline void sleep_s(double seconds) { std::this_thread::sleep_for(std::chrono::duration<double>(seconds)); }

inline void wait_ready(quac::Device& device, uint8_t mask) {
    for (;;) {
        auto reply = device.read(quac::DacReady);
        if (uint8_t(reply.payload) & mask) return;
        std::cout << "Not ready, state ";
        hex_byte(uint8_t(reply.payload));
        std::cout << "\n";
        sleep_s(0.1);
    }
}
