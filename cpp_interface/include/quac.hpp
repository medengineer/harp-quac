#pragma once

// Hand-written counterpart of software/python/device.py. The Harp toolkit
// generates the Python and Bonsai interfaces from device.yml; it does not emit C++.
// Field layout follows device.yml. Update this file when that metadata changes.

#include "harp.hpp"

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string_view>

namespace quac {

inline constexpr uint16_t WHO_AM_I = 1411;
inline constexpr const char* DEVICE_NAME = "Quac";

enum class DigitalInputs : uint8_t { None = 0, DI0 = 0x1, DI1 = 0x2, DI2 = 0x4, DI3 = 0x8 };
enum class DigitalOutputs : uint8_t { None = 0, DO0 = 0x1, DO1 = 0x2, DO2 = 0x4, DO3 = 0x8 };
enum class AnalogOutputs : uint8_t { None = 0, AO0 = 0x1, AO1 = 0x2, AO2 = 0x4, AO3 = 0x8 };
enum class PlayerType : uint8_t { File = 0, Sine = 1, Trapezoid = 2 };
enum class DIPinMode : uint8_t { None = 0, ExternalTrigger = 1, Blanking = 2 };

constexpr DigitalInputs operator|(DigitalInputs a, DigitalInputs b) { return DigitalInputs(uint8_t(a) | uint8_t(b)); }
constexpr DigitalInputs operator&(DigitalInputs a, DigitalInputs b) { return DigitalInputs(uint8_t(a) & uint8_t(b)); }
constexpr DigitalOutputs operator|(DigitalOutputs a, DigitalOutputs b) { return DigitalOutputs(uint8_t(a) | uint8_t(b)); }
constexpr DigitalOutputs operator&(DigitalOutputs a, DigitalOutputs b) { return DigitalOutputs(uint8_t(a) & uint8_t(b)); }
constexpr AnalogOutputs operator|(AnalogOutputs a, AnalogOutputs b) { return AnalogOutputs(uint8_t(a) | uint8_t(b)); }
constexpr AnalogOutputs operator&(AnalogOutputs a, AnalogOutputs b) { return AnalogOutputs(uint8_t(a) & uint8_t(b)); }

#pragma pack(push, 1)

struct AoPortState {
    float ao_channel0 = 0;
    float ao_channel1 = 0;
    float ao_channel2 = 0;
    float ao_channel3 = 0;
};

struct FileSettings {
    uint32_t cycles = 1;
    uint32_t duration = 0;
    uint32_t update_frequency = 500000;
    char path[33]{};

    void set_path(std::string_view value) {
        std::memset(path, 0, sizeof path);
        std::memcpy(path, value.data(), value.size() < sizeof path ? value.size() : sizeof path - 1);
    }
};

struct SineSettings {
    uint32_t cycles = 1;
    uint32_t duration = 0;
    uint32_t update_frequency = 500000;
    uint32_t frequency = 1;
    float amplitude = 1;
    float vertical_shift = 0;
    float normalized_phase_shift = 0;
};

struct TrapezoidSettings {
    uint32_t cycles = 1;
    uint32_t duration = 0;
    uint32_t update_frequency = 500000;
    uint32_t frequency = 1;
    float amplitude = 1;
    float vertical_shift = 0;
    float normalized_phase_shift = 0;
    uint32_t ramp_on_duration = 0;
    uint32_t pulse_width_duration = 0;
    uint32_t ramp_off_duration = 0;
};

#pragma pack(pop)

static_assert(std::endian::native == std::endian::little);
static_assert(sizeof(AoPortState) == 16);
static_assert(sizeof(FileSettings) == 45);
static_assert(sizeof(SineSettings) == 28);
static_assert(sizeof(TrapezoidSettings) == 40);
static_assert(offsetof(SineSettings, amplitude) == 16);
static_assert(offsetof(TrapezoidSettings, ramp_on_duration) == 28);
static_assert(offsetof(FileSettings, path) == 12);

inline constexpr harp::Register<DigitalOutputs> DOPortState{32, harp::PayloadType::U8};
inline constexpr harp::Register<DigitalOutputs> DOPortSet{33, harp::PayloadType::U8};
inline constexpr harp::Register<DigitalOutputs> DOPortClear{34, harp::PayloadType::U8};
inline constexpr harp::Register<DigitalInputs> DIPortState{35, harp::PayloadType::U8};
inline constexpr harp::Register<AoPortState> AOPortState{36, harp::PayloadType::Float};
inline constexpr harp::Register<float> AOChannel0{37, harp::PayloadType::Float};
inline constexpr harp::Register<float> AOChannel1{38, harp::PayloadType::Float};
inline constexpr harp::Register<float> AOChannel2{39, harp::PayloadType::Float};
inline constexpr harp::Register<float> AOChannel3{40, harp::PayloadType::Float};
inline constexpr harp::Register<AnalogOutputs> DacReady{41, harp::PayloadType::U8};
inline constexpr harp::Register<AnalogOutputs> DacStart{42, harp::PayloadType::U8};
inline constexpr harp::Register<AnalogOutputs> DacPause{43, harp::PayloadType::U8};
inline constexpr harp::Register<AnalogOutputs> DacAbort{44, harp::PayloadType::U8};
inline constexpr harp::Register<AnalogOutputs> DacFinished{45, harp::PayloadType::U8};
inline constexpr harp::Register<DIPinMode> PinModeDI0{46, harp::PayloadType::U8};
inline constexpr harp::Register<DIPinMode> PinModeDI1{47, harp::PayloadType::U8};
inline constexpr harp::Register<DIPinMode> PinModeDI2{48, harp::PayloadType::U8};
inline constexpr harp::Register<DIPinMode> PinModeDI3{49, harp::PayloadType::U8};
inline constexpr harp::Register<DigitalInputs> AOChannelsAssignedToDI0{50, harp::PayloadType::U8};
inline constexpr harp::Register<DigitalInputs> AOChannelsAssignedToDI1{51, harp::PayloadType::U8};
inline constexpr harp::Register<DigitalInputs> AOChannelsAssignedToDI2{52, harp::PayloadType::U8};
inline constexpr harp::Register<DigitalInputs> AOChannelsAssignedToDI3{53, harp::PayloadType::U8};
inline constexpr harp::Register<PlayerType> ActivePlayer0{54, harp::PayloadType::U8};
inline constexpr harp::Register<PlayerType> ActivePlayer1{55, harp::PayloadType::U8};
inline constexpr harp::Register<PlayerType> ActivePlayer2{56, harp::PayloadType::U8};
inline constexpr harp::Register<PlayerType> ActivePlayer3{57, harp::PayloadType::U8};
inline constexpr harp::Register<FileSettings> FileSettings0{58, harp::PayloadType::U8};
inline constexpr harp::Register<FileSettings> FileSettings1{59, harp::PayloadType::U8};
inline constexpr harp::Register<FileSettings> FileSettings2{60, harp::PayloadType::U8};
inline constexpr harp::Register<FileSettings> FileSettings3{61, harp::PayloadType::U8};
inline constexpr harp::Register<SineSettings> SineSettings0{62, harp::PayloadType::U8};
inline constexpr harp::Register<SineSettings> SineSettings1{63, harp::PayloadType::U8};
inline constexpr harp::Register<SineSettings> SineSettings2{64, harp::PayloadType::U8};
inline constexpr harp::Register<SineSettings> SineSettings3{65, harp::PayloadType::U8};
inline constexpr harp::Register<TrapezoidSettings> TrapezoidSettings0{66, harp::PayloadType::U8};
inline constexpr harp::Register<TrapezoidSettings> TrapezoidSettings1{67, harp::PayloadType::U8};
inline constexpr harp::Register<TrapezoidSettings> TrapezoidSettings2{68, harp::PayloadType::U8};
inline constexpr harp::Register<TrapezoidSettings> TrapezoidSettings3{69, harp::PayloadType::U8};

class Device : public harp::Device {
public:
    explicit Device(std::string port) : harp::Device(std::move(port), WHO_AM_I) {}
};

}  // namespace quac
