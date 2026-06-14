#ifndef _HUMANIZER_H_
#define _HUMANIZER_H_

#include <cstdint>
#include <cmath>
#include "Gamepad/Gamepad.h"

// Settings for the humanization pipeline.
// All float values are normalized 0.0-1.0 unless noted.
struct HumanizerSettings
{
    // --- Layer 1: Magnitude shaping ---
    float magnitude_cap            = 0.85f;
    float magnitude_noise          = 0.03f;

    // --- Layer 2: Drift ---
    float drift_strength           = 0.02f;
    float drift_max                = 0.05f;
    uint32_t drift_retarget_frames = 400;

    // Inputs below this (normalized) are treated as idle and drift is applied
    float idle_threshold           = 0.10f;

    // Fade output to zero over this many frames when stick returns to center
    uint32_t release_fade_frames   = 12;

    // Enable/disable the whole humanizer
    bool enabled                   = true;
};

class Humanizer
{
public:
    // Singleton - one shared instance used by both XInput.cpp and WebApp.cpp
    static Humanizer& get_instance()
    {
        static Humanizer instance;
        return instance;
    }

    Humanizer(const Humanizer&) = delete;
    Humanizer& operator=(const Humanizer&) = delete;

    void set_settings(const HumanizerSettings& settings) { settings_ = settings; }
    const HumanizerSettings& get_settings() const { return settings_; }

    // Load settings from flash on boot
    void load_from_flash();

    // Save settings to flash (called after SET_HUMANIZER packet received)
    void save_to_flash();

    // Call once per frame in XInput.cpp after get_pad_in()
    void process(Gamepad::PadIn& pad_in);

private:
    Humanizer() = default;
    ~Humanizer() = default;

    HumanizerSettings settings_;

    float drift_lx_ = 0.0f;
    float drift_ly_ = 0.0f;
    float drift_rx_ = 0.0f;
    float drift_ry_ = 0.0f;

    float target_lx_ = 0.0f;
    float target_ly_ = 0.0f;
    float target_rx_ = 0.0f;
    float target_ry_ = 0.0f;

    uint32_t retarget_counter_l_ = 0;
    uint32_t retarget_counter_r_ = 0;

    uint32_t fade_counter_l_ = 0;
    uint32_t fade_counter_r_ = 0;

    bool was_idle_l_ = true;
    bool was_idle_r_ = true;

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
