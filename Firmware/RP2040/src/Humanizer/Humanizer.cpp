#include "Humanizer/Humanizer.h"

static constexpr float INT16_MAX_F = 32767.0f;

float Humanizer::next_rand()
{
    rng_state_ ^= rng_state_ << 13;
    rng_state_ ^= rng_state_ >> 17;
    rng_state_ ^= rng_state_ << 5;
    return static_cast<float>(static_cast<int32_t>(rng_state_)) / 2147483648.0f;
}

float Humanizer::next_rand_pos()
{
    return (next_rand() + 1.0f) * 0.5f;
}

void Humanizer::process(Gamepad::PadIn& pad_in)
{
    if (!settings_.enabled) return;

    process_stick(
        pad_in.joystick_lx, pad_in.joystick_ly,
        drift_lx_, drift_ly_,
        target_lx_, target_ly_,
        retarget_counter_l_,
        fade_counter_l_,
        was_idle_l_);

    process_stick(
        pad_in.joystick_rx, pad_in.joystick_ry,
        drift_rx_, drift_ry_,
        target_rx_, target_ry_,
        retarget_counter_r_,
        fade_counter_r_,
        was_idle_r_);
}

void Humanizer::process_stick(
    int16_t& x, int16_t& y,
    float& drift_x, float& drift_y,
    float& target_x, float& target_y,
    uint32_t& retarget_counter,
    uint32_t& fade_counter,
    bool& was_idle)
{
    // Normalize to -1.0 to 1.0
    float nx = static_cast<float>(x) / INT16_MAX_F;
    float ny = static_cast<float>(y) / INT16_MAX_F;

    // Compute magnitude
    float mag = sqrtf(nx * nx + ny * ny);

    bool is_idle = (mag < settings_.idle_threshold);

    // --- Layer 1: Magnitude shaping ---
    // Only applies when stick is actually being deflected
    if (!is_idle && mag > 0.0f)
    {
        // Cap magnitude
        if (mag > settings_.magnitude_cap)
        {
            float scale = settings_.magnitude_cap / mag;
            nx *= scale;
            ny *= scale;
            mag = settings_.magnitude_cap;
        }

        // Add subtle magnitude noise at high deflection
        if (settings_.magnitude_noise > 0.0f && mag > 0.5f)
        {
            float noise = next_rand() * settings_.magnitude_noise * (mag - 0.5f) * 2.0f;
            float noised_mag = mag + noise;
            if (noised_mag > settings_.magnitude_cap) noised_mag = settings_.magnitude_cap;
            if (noised_mag > 0.0f && mag > 0.0f)
            {
                float scale = noised_mag / mag;
                nx *= scale;
                ny *= scale;
                mag = noised_mag;
            }
        }
    }

    // --- Layer 2: Drift ---
    if (is_idle)
    {
        // Retarget drift periodically
        retarget_counter++;
        if (retarget_counter >= settings_.drift_retarget_frames)
        {
            retarget_counter = 0;
            target_x = next_rand() * settings_.drift_max;
            target_y = next_rand() * settings_.drift_max;
        }

        // Smoothly move drift toward target
        drift_x += (target_x - drift_x) * settings_.drift_strength;
        drift_y += (target_y - drift_y) * settings_.drift_strength;

        // Apply drift to output
        nx += drift_x;
        ny += drift_y;

        // Clamp
        if (nx >  1.0f) nx =  1.0f;
        if (nx < -1.0f) nx = -1.0f;
        if (ny >  1.0f) ny =  1.0f;
        if (ny < -1.0f) ny = -1.0f;
    }
    else
    {
        // Stick is active — decay drift slowly back to zero
        drift_x *= 0.95f;
        drift_y *= 0.95f;
        retarget_counter = 0;
    }

    // --- Release fade ---
    // When stick transitions from active to idle, fade output over a few frames
    if (was_idle && !is_idle)
    {
        // Stick just engaged — reset fade
        fade_counter = 0;
    }
    else if (!was_idle && is_idle)
    {
        // Stick just released — start fade
        fade_counter = settings_.release_fade_frames;
    }

    if (fade_counter > 0 && is_idle)
    {
        float fade = static_cast<float>(fade_counter) / static_cast<float>(settings_.release_fade_frames);
        nx *= fade;
        ny *= fade;
        fade_counter--;
    }

    was_idle = is_idle;

    // Convert back to int16
    x = static_cast<int16_t>(nx * INT16_MAX_F);
    y = static_cast<int16_t>(ny * INT16_MAX_F);
}
