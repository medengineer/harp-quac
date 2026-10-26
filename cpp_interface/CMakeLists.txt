cmake_minimum_required(VERSION 3.16)
project(quac_cpp LANGUAGES CXX)

add_library(harp src/harp.cpp)
target_include_directories(harp PUBLIC include)
target_compile_features(harp PUBLIC cxx_std_20)

set(EXAMPLES
    read_active_players
    abort
    stress_test
    toggle_ao_port
    toggle_do_port
    wait_for_events
    sine_sweep
    sw_trigger_single_shot_waveform
    sw_trigger_sine_wave
    test_blanking
    sw_trigger_pulse_train
)
foreach(example ${EXAMPLES})
    add_executable(${example} examples/${example}.cpp)
    target_link_libraries(${example} PRIVATE harp)
endforeach()
