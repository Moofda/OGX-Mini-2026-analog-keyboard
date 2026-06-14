#include "Humanizer/Humanizer.h"

// Q16.16 fixed point constants - pure integer literals, no library needed
static const fix16_t FIX_INT16_MAX = 0x7FFF0000; // 32767.0
static const fix16_t FIX_1         = 0x00010000; // 1.0
static const fix16_t FIX_0         = 0x00000000; // 0.0
static const fix16_t FIX_NEG1      = 0xFFFF0000; // -1.0
static const fix16_t FIX_095       = 0x0000F333; // 0.95

// Inline fixed point math - no library functions needed
static inline fix16_t fp_mul(fix16_t a, fix16_t b)
{
    return (fix16_t)(((int64_t)a * b) >> 16);
}

static inline fix16_t fp_div(fix16_t a, fix16_t b)
{
    return (fix16_t)(((int64_t)a << 16) / b);
}

static inline fix16_t fp_add(fix16_t a, fix16_t b)
{
    return a + b;
}

static inline fix16_t fp_sub(fix16_t a, fix16_t b)
{
    return a - b;
}

static inline fix16_t fp_from_int(int16_t a)
{
    return (fix16_t)((int32_t)a << 16);
}

static inline int16_t fp_to_int(fix16_t a)
{
    return (int16_t)(a >> 16);
}

// Integer square root for magnitude calculation
static fix16_t fp_sqrt(fix16_t x)
{
    if (x <= 0) return 0;
    fix16_t result = x;
    fix16_t prev = 0;
    for (int i = 0; i < 20 && result != prev; i++)
    {
        prev = result;
        result = (result + (fix16_t)(((int64_t)x << 16) / result)) >> 1;
    }
    return result;
}

fix16_t Humanizer::next_rand()
{
    rng_state_ ^= rng_state_ << 13;
    rng_state_ ^= rng_state_ >> 17;
    rng_state_ ^= rng_state_ << 5;
    int16_t signed_val = static_cast<int16_t>(rng_state_ % 1000);
    return fp_div(fp_from_int(signed_val), fp_from_int(1000));
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
    fix16_t& drift_x, fix16_t& drift_y,
    fix16_t& target_x, fix16_t& target_y,
    uint32_t& retarget_counter,
    uint32_t& fade_counter,
    bool& was_idle)
{
    fix16_t nx = fp_div(fp_from_int(x), FIX_INT16_MAX);
    fix16_t ny = fp_div(fp_from_int(y), FIX_INT16_MAX);

    fix16_t mag_sq = fp_add(fp_mul(nx, nx), fp_mul(ny, ny));
    fix16_t idle_sq = fp_mul(settings_.idle_threshold, settings_.idle_threshold);
    bool is_idle = (mag_sq < idle_sq);

    if (!is_idle)
    {
        fix16_t cap_sq = fp_mul(settings_.magnitude_cap, settings_.magnitude_cap);
        if (mag_sq > cap_sq)
        {
            fix16_t mag = fp_sqrt(mag_sq);
            if (mag > FIX_0)
            {
                fix16_t scale = fp_div(settings_.magnitude_cap, mag);
                nx = fp_mul(nx, scale);
                ny = fp_mul(ny, scale);
            }
        }
    }

    if (is_idle)
    {
        retarget_counter++;
        if (retarget_counter >= settings_.drift_retarget_frames)
        {
            retarget_counter = 0;
            target_x = fp_mul(next_rand(), settings_.drift_max);
            target_y = fp_mul(next_rand(), settings_.drift_max);
        }
        fix16_t tx = fp_sub(target_x, drift_x);
        fix16_t ty = fp_sub(target_y, drift_y);
        drift_x = fp_add(drift_x, fp_mul(tx, settings_.drift_strength));
        drift_y = fp_add(drift_y, fp_mul(ty, settings_.drift_strength));
        nx = fp_add(nx, drift_x);
        ny = fp_add(ny, drift_y);
        if (nx > FIX_1)    nx = FIX_1;
        if (nx < FIX_NEG1) nx = FIX_NEG1;
        if (ny > FIX_1)    ny = FIX_1;
        if (ny < FIX_NEG1) ny = FIX_NEG1;
    }
    else
    {
        drift_x = fp_mul(drift_x, FIX_095);
        drift_y = fp_mul(drift_y, FIX_095);
        retarget_counter = 0;
    }

    if (was_idle && !is_idle)
        fade_counter = 0;
    else if (!was_idle && is_idle)
        fade_counter = settings_.release_fade_frames;

    if (fade_counter > 0 && is_idle)
    {
        fix16_t fade = fp_div(
            fp_from_int(static_cast<int16_t>(fade_counter)),
            fp_from_int(static_cast<int16_t>(settings_.release_fade_frames)));
        nx = fp_mul(nx, fade);
        ny = fp_mul(ny, fade);
        fade_counter--;
    }

    was_idle = is_idle;

    x = static_cast<int16_t>(fp_to_int(fp_mul(nx, FIX_INT16_MAX)));
    y = static_cast<int16_t>(fp_to_int(fp_mul(ny, FIX_INT16_MAX)));
}
