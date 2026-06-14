#ifndef _HUMANIZER_H_
#define _HUMANIZER_H_

#include <cstdint>
#include "Gamepad/Gamepad.h"
#include "libfixmath/fix16.hpp"

struct HumanizerSettings
{
    fix16_t magnitude_cap            = 0x0000D999; // 0.85
    fix16_t magnitude_noise          = 0x00000799; // 0.03
    fix16_t drift_strength           = 0x00001470; // 0.08
    fix16_t drift_max                = 0x00004000; // 0.25
    uint32_t drift_retarget_frames   = 400;
    fix16_t idle_threshold           = 0x00001999; // 0.10
    uint32_t release_fade_frames     = 12;
    bool enabled                     = true;
};

class Humanizer
{
public:
    Humanizer() = default;
    ~Humanizer() = default;

    void set_settings(const HumanizerSettings& settings) { settings_ = settings; }
    const HumanizerSettings& get_settings() const { return settings_; }

    void process(Gamepad::PadIn& pad_in);

private:
    HumanizerSettings settings_;

    fix16_t drift_lx_ = 0, drift_ly_ = 0;
    fix16_t drift_rx_ = 0, drift_ry_ = 0;
    fix16_t target_lx_ = 0, target_ly_ = 0;
    fix16_t target_rx_ = 0, target_ry_ = 0;
    uint32_t retarget_counter_l_ = 0, retarget_counter_r_ = 0;
    uint32_t fade_counter_l_ = 0, fade_counter_r_ = 0;
    bool was_idle_l_ = true, was_idle_r_ = true;
    uint32_t rng_state_ = 12345;

    fix16_t next_rand();

    void process_stick(
        int16_t& x, int16_t& y,
        fix16_t& drift_x, fix16_t& drift_y,
        fix16_t& target_x, fix16_t& target_y,
        uint32_t& retarget_counter,
        uint32_t& fade_counter,
        bool& was_idle);
};

#endif // _HUMANIZER_H_
