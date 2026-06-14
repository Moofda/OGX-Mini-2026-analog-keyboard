#ifndef _HUMANIZER_H_
#define _HUMANIZER_H_

#include <cstdint>
#include "Gamepad/Gamepad.h"

struct HumanizerSettings
{
    float magnitude_cap            = 0.85f;
    float magnitude_noise          = 0.03f;
    float drift_strength           = 0.08f;
    float drift_max                = 0.25f;
    uint32_t drift_retarget_frames = 400;
    float idle_threshold           = 0.10f;
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

    float drift_lx_ = 0.0f, drift_ly_ = 0.0f;
    float drift_rx_ = 0.0f, drift_ry_ = 0.0f;
    float target_lx_ = 0.0f, target_ly_ = 0.0f;
    float target_rx_ = 0.0f, target_ry_ = 0.0f;
    uint32_t retarget_counter_l_ = 0, retarget_counter_r_ = 0;
    uint32_t fade_counter_l_ = 0, fade_counter_r_ = 0;
    bool was_idle_l_ = true, was_idle_r_ = true;
    uint32_t rng_state_ = 12345;

    float next_rand();
    float next_rand_pos();

    void process_stick(
        int16_t& x, int16_t& y,
        float& drift_x, float& drift_y,
        float& target_x, float& target_y,
        uint32_t& retarget_counter,
        uint32_t& fade_counter,
        bool& was_idle);
};

#endif // _HUMANIZER_H_
