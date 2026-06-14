#ifndef _HUMANIZER_H_
#define _HUMANIZER_H_

#include <cstdint>
#include "Gamepad/Gamepad.h"

struct HumanizerSettings
{
    float magnitude_cap;
    float magnitude_noise;
    float drift_strength;
    float drift_max;
    uint32_t drift_retarget_frames;
    float idle_threshold;
    uint32_t release_fade_frames;
    bool enabled;
};

class Humanizer
{
public:
    Humanizer();
    ~Humanizer() = default;

    void set_settings(const HumanizerSettings& settings) { settings_ = settings; }
    const HumanizerSettings& get_settings() const { return settings_; }

    void process(Gamepad::PadIn& pad_in);

private:
    HumanizerSettings settings_;

    float drift_lx_, drift_ly_;
    float drift_rx_, drift_ry_;
    float target_lx_, target_ly_;
    float target_rx_, target_ry_;
    uint32_t retarget_counter_l_, retarget_counter_r_;
    uint32_t fade_counter_l_, fade_counter_r_;
    bool was_idle_l_, was_idle_r_;
    uint32_t rng_state_;

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
