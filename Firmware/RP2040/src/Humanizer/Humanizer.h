#ifndef _HUMANIZER_H_
#define _HUMANIZER_H_

#include <cstdint>
#include "Gamepad/Gamepad.h"
#include "libfixmath/fix16.hpp"

struct HumanizerSettings
{
    Fix16 magnitude_cap            = Fix16(0.85f);
    Fix16 magnitude_noise          = Fix16(0.03f);
    Fix16 drift_strength           = Fix16(0.08f);
    Fix16 drift_max                = Fix16(0.25f);
    uint32_t drift_retarget_frames = 400;
    Fix16 idle_threshold           = Fix16(0.10f);
    uint32_t release_fade_frames   = 12;
    bool enabled                   = true;
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

    Fix16 drift_lx_, drift_ly_;
    Fix16 drift_rx_, drift_ry_;
    Fix16 target_lx_, target_ly_;
    Fix16 target_rx_, target_ry_;
    uint32_t retarget_counter_l_ = 0, retarget_counter_r_ = 0;
    uint32_t fade_counter_l_ = 0, fade_counter_r_ = 0;
    bool was_idle_l_ = true, was_idle_r_ = true;
    uint32_t rng_state_ = 12345;

    Fix16 next_rand();

    void process_stick(
        int16_t& x, int16_t& y,
        Fix16& drift_x, Fix16& drift_y,
        Fix16& target_x, Fix16& target_y,
        uint32_t& retarget_counter,
        uint32_t& fade_counter,
        bool& was_idle);
};

#endif // _HUMANIZER_H_
