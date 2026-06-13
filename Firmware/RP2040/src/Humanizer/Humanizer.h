#ifndef _HUMANIZER_H_
#define _HUMANIZER_H_

#include <cstdint>
#include <cmath>
#include "Gamepad/Gamepad.h"

// Settings for the humanization pipeline.
// All float values are normalized 0.0–1.0 unless noted.
struct HumanizerSettings
{
    // --- Layer 1: Magnitude shaping ---
    // Caps the maximum output magnitude to prevent perfect square-gate artifact.
    // 0.95 = output never exceeds 95% of INT16_MAX
    float magnitude_cap        = 0.85f;
    // Adds subtle per-frame magnitude variation at high deflection (0 = off)
    float magnitude_noise      = 0.03f;

    // --- Layer 2: Humanization ---
    // How strongly idle drift pulls toward the drift target (0 = off, 1 = instant)
    float drift_strength       = 0.02f;
    // Maximum drift offset in normalized units (0.0–1.0)
    float drift_max            = 0.05f;
    // How often the drift target changes, in frames (~1000hz so 500 = ~0.5s)
    uint32_t drift_retarget_frames = 400;

    // Deadzone threshold — inputs below this (normalized) are treated as idle
    // and drift is applied. Should match or be slightly above your in-game deadzone.
    float idle_threshold       = 0.10f;

    // Release fade — when stick returns to center, fade output to zero over this
    // many frames rather than snapping (0 = off)
    uint32_t release_fade_frames = 12;

    // Enable/disable the whole humanizer
    bool enabled = true;
};

class Humanizer
{
public:
    Humanizer() = default;
    ~Humanizer() = default;

    void set_settings(const HumanizerSettings& settings) { settings_ = settings; }
    const HumanizerSettings& get_settings() const { return settings_; }

    // Call this once per frame after get_pad_in(), before sending to device driver.
    // Modifies joystick_lx/ly/rx/ry in place.
    void process(Gamepad::PadIn& pad_in);

private:
    HumanizerSettings settings_;

    // Per-axis drift state (left and right stick, x and y)
    float drift_lx_ = 0.0f;
    float drift_ly_ = 0.0f;
    float drift_rx_ = 0.0f;
    float drift_ry_ = 0.0f;

    // Drift targets
    float target_lx_ = 0.0f;
    float target_ly_ = 0.0f;
    float target_rx_ = 0.0f;
    float target_ry_ = 0.0f;

    // Frame counters for drift retargeting
    uint32_t retarget_counter_l_ = 0;
    uint32_t retarget_counter_r_ = 0;

    // Release fade counters
    uint32_t fade_counter_l_ = 0;
    uint32_t fade_counter_r_ = 0;

    // Previous frame idle state
    bool was_idle_l_ = true;
    bool was_idle_r_ = true;

    // Simple deterministic pseudo-random (no stdlib rand dependency)
    uint32_t rng_state_ = 12345;
    float next_rand();           // returns -1.0 to 1.0
    float next_rand_pos();       // returns  0.0 to 1.0

    void process_stick(
        int16_t& x, int16_t& y,
        float& drift_x, float& drift_y,
        float& target_x, float& target_y,
        uint32_t& retarget_counter,
        uint32_t& fade_counter,
        bool& was_idle);
};

#endif // _HUMANIZER_H_
