#include "Humanizer/Humanizer.h"

Fix16 Humanizer::next_rand()
{
    rng_state_ ^= rng_state_ << 13;
    rng_state_ ^= rng_state_ >> 17;
    rng_state_ ^= rng_state_ << 5;
    int32_t signed_val = static_cast<int32_t>(rng_state_);
    return Fix16(signed_val % 1000) / Fix16(1000);
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
    Fix16& drift_x, Fix16& drift_y,
    Fix16& target_x, Fix16& target_y,
    uint32_t& retarget_counter,
    uint32_t& fade_counter,
    bool& was_idle)
{
    static const Fix16 INT16_MAX_F(32767);
    static const Fix16 FIX_1(1);
    static const Fix16 FIX_0(0);
    static const Fix16 FIX_NEG1(-1);
    static const Fix16 FIX_095(0.95f);

    Fix16 nx = Fix16(x) / INT16_MAX_F;
    Fix16 ny = Fix16(y) / INT16_MAX_F;

    Fix16 mag_sq = (nx * nx) + (ny * ny);
    Fix16 idle_sq = settings_.idle_threshold * settings_.idle_threshold;
    bool is_idle = (mag_sq < idle_sq);

    if (!is_idle)
    {
        Fix16 cap_sq = settings_.magnitude_cap * settings_.magnitude_cap;
        if (mag_sq > cap_sq)
        {
            Fix16 mag = fix16::sqrt(mag_sq);
            if (mag > FIX_0)
            {
                Fix16 scale = settings_.magnitude_cap / mag;
                nx = nx * scale;
                ny = ny * scale;
            }
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
        drift_x = drift_x + (target_x - drift_x) * settings_.drift_strength;
        drift_y = drift_y + (target_y - drift_y) * settings_.drift_strength;
        nx = nx + drift_x;
        ny = ny + drift_y;
        if (nx > FIX_1)  nx = FIX_1;
        if (nx < FIX_NEG1) nx = FIX_NEG1;
        if (ny > FIX_1)  ny = FIX_1;
        if (ny < FIX_NEG1) ny = FIX_NEG1;
    }
    else
    {
        drift_x = drift_x * FIX_095;
        drift_y = drift_y * FIX_095;
        retarget_counter = 0;
    }

    if (was_idle && !is_idle)
        fade_counter = 0;
    else if (!was_idle && is_idle)
        fade_counter = settings_.release_fade_frames;

    if (fade_counter > 0 && is_idle)
    {
        Fix16 fade = Fix16((int)fade_counter) / Fix16((int)settings_.release_fade_frames);
        nx = nx * fade;
        ny = ny * fade;
        fade_counter--;
    }

    was_idle = is_idle;

    x = static_cast<int16_t>(fix16_to_int(nx * INT16_MAX_F));
    y = static_cast<int16_t>(fix16_to_int(ny * INT16_MAX_F));
}
