#include "Humanizer/Humanizer.h"

static const fix16_t FIX_INT16_MAX = 0x7FFF0000;
static const fix16_t FIX_1         = 0x00010000;
static const fix16_t FIX_0         = 0x00000000;
static const fix16_t FIX_NEG1      = 0xFFFF0000;
static const fix16_t FIX_095       = 0x0000F333;

Fix16 Humanizer::next_rand()
{
    rng_state_ ^= rng_state_ << 13;
    rng_state_ ^= rng_state_ >> 17;
    rng_state_ ^= rng_state_ << 5;
    int16_t signed_val = static_cast<int16_t>(rng_state_ % 1000);
    return Fix16(signed_val) / Fix16(static_cast<int16_t>(1000));
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
    fix16_t nx = fix16_div(fix16_from_int(x), FIX_INT16_MAX);
    fix16_t ny = fix16_div(fix16_from_int(y), FIX_INT16_MAX);

    fix16_t mag_sq = fix16_add(fix16_mul(nx, nx), fix16_mul(ny, ny));
    fix16_t idle_sq = fix16_mul(settings_.idle_threshold.value, settings_.idle_threshold.value);
    bool is_idle = (mag_sq < idle_sq);

    if (!is_idle)
    {
        fix16_t cap_sq = fix16_mul(settings_.magnitude_cap.value, settings_.magnitude_cap.value);
        if (mag_sq > cap_sq)
        {
            fix16_t mag = fix16_sqrt(mag_sq);
            if (mag > FIX_0)
            {
                fix16_t scale = fix16_div(settings_.magnitude_cap.value, mag);
                nx = fix16_mul(nx, scale);
                ny = fix16_mul(ny, scale);
            }
        }
    }

    if (is_idle)
    {
        retarget_counter++;
        if (retarget_counter >= settings_.drift_retarget_frames)
        {
            retarget_counter = 0;
            target_x = Fix16(next_rand() * settings_.drift_max);
            target_y = Fix16(next_rand() * settings_.drift_max);
        }
        fix16_t tx = fix16_sub(target_x.value, drift_x.value);
        fix16_t ty = fix16_sub(target_y.value, drift_y.value);
        drift_x.value = fix16_add(drift_x.value, fix16_mul(tx, settings_.drift_strength.value));
        drift_y.value = fix16_add(drift_y.value, fix16_mul(ty, settings_.drift_strength.value));
        nx = fix16_add(nx, drift_x.value);
        ny = fix16_add(ny, drift_y.value);
        if (nx > FIX_1)    nx = FIX_1;
        if (nx < FIX_NEG1) nx = FIX_NEG1;
        if (ny > FIX_1)    ny = FIX_1;
        if (ny < FIX_NEG1) ny = FIX_NEG1;
    }
    else
    {
        drift_x.value = fix16_mul(drift_x.value, FIX_095);
        drift_y.value = fix16_mul(drift_y.value, FIX_095);
        retarget_counter = 0;
    }

    if (was_idle && !is_idle)
        fade_counter = 0;
    else if (!was_idle && is_idle)
        fade_counter = settings_.release_fade_frames;

    if (fade_counter > 0 && is_idle)
    {
        fix16_t fade = fix16_div(
            fix16_from_int(static_cast<int16_t>(fade_counter)),
            fix16_from_int(static_cast<int16_t>(settings_.release_fade_frames)));
        nx = fix16_mul(nx, fade);
        ny = fix16_mul(ny, fade);
        fade_counter--;
    }

    was_idle = is_idle;

    x = static_cast<int16_t>(fix16_to_int(fix16_mul(nx, FIX_INT16_MAX)));
    y = static_cast<int16_t>(fix16_to_int(fix16_mul(ny, FIX_INT16_MAX)));
}
