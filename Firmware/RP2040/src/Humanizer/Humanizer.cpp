#pragma GCC optimize ("O0")
#include "Humanizer/Humanizer.h"

static constexpr float INT16_MAX_F = 32767.0f;

Humanizer::Humanizer()
{
    settings_.magnitude_cap = 0.85f;
    settings_.magnitude_noise = 0.03f;
    settings_.drift_strength = 0.08f;
    settings_.drift_max = 0.25f;
    settings_.drift_retarget_frames = 400;
    settings_.idle_threshold = 0.10f;
    settings_.release_fade_frames = 12;
    settings_.enabled = true;

    drift_lx_ = 0.0f; drift_ly_ = 0.0f;
    drift_rx_ = 0.0f; drift_ry_ = 0.0f;
    target_lx_ = 0.0f; target_ly_ = 0.0f;
    target_rx_ = 0.0f; target_ry_ = 0.0f;
    retarget_counter_l_ = 0; retarget_counter_r_ = 0;
    fade_counter_l_ = 0; fade_counter_r_ = 0;
    was_idle_l_ = true; was_idle_r_ = true;
    rng_state_ = 12345;
}

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
    float nx = static_cast<float>(x) / INT16_MAX_F;
    float ny = static_cast<float>(y) / INT16_MAX_F;
    float mag_sq = nx * nx + ny * ny;
    float idle_sq = settings_.idle_threshold * settings_.idle_threshold;
    bool is_idle = (mag_sq < idle_sq);

    if (!is_idle && mag_sq > 0.0f)
    {
        float cap_sq = settings_.magnitude_cap * settings_.magnitude_cap;
        if (mag_sq > cap_sq)
        {
            float mag = __builtin_sqrtf(mag_sq);
            float scale = settings_.magnitude_cap / mag;
            nx *= scale;
            ny *= scale;
            mag_sq = cap_sq;
        }
        if (settings_.magnitude_noise > 0.0f && mag_sq > 0.25f)
        {
            float noise = next_rand() * settings_.magnitude_noise * 0.1f;
            nx += noise;
            ny += noise;
        }
    }

    if (is_idle)
    {
        retarget_counter++;
        if (retarget_counter >= settings_.drift_retarget_frames)
        {
            retarget_counter = 0;
            target_x = next_rand() * settings_.drift_max;
            target_y = next_rand() * settings_.drift_max;
        }
        drift_x += (target_x - drift_x) * settings_.drift_strength;
        drift_y += (target_y - drift_y) * settings_.drift_strength;
        nx += drift_x;
        ny += drift_y;
        if (nx >  1.0f) nx =  1.0f;
        if (nx < -1.0f) nx = -1.0f;
        if (ny >  1.0f) ny =  1.0f;
        if (ny < -1.0f) ny = -1.0f;
    }
    else
    {
        drift_x *= 0.95f;
        drift_y *= 0.95f;
        retarget_counter = 0;
    }

    if (was_idle && !is_idle)
        fade_counter = 0;
    else if (!was_idle && is_idle)
        fade_counter = settings_.release_fade_frames;

    if (fade_counter > 0 && is_idle)
    {
        float fade = static_cast<float>(fade_counter) /
                     static_cast<float>(settings_.release_fade_frames);
        nx *= fade;
        ny *= fade;
        fade_counter--;
    }

    was_idle = is_idle;

    x = static_cast<int16_t>(nx * INT16_MAX_F);
    y = static_cast<int16_t>(ny * INT16_MAX_F);
}
