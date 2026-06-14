#include "Humanizer/Humanizer.h"

static inline fix16_t fp_mul(fix16_t a, fix16_t b)
{
    return (fix16_t)(((int64_t)a * b) >> 16);
}

static inline fix16_t fp_div(fix16_t a, fix16_t b)
{
    if (b == 0) return 0;
    return (fix16_t)(((int64_t)a << 16) / b);
}

static inline fix16_t fp_from_int(int16_t a)
{
    return (fix16_t)((int32_t)a << 16);
}

static inline int16_t fp_to_int(fix16_t a)
{
    return (int16_t)(a >> 16);
}

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

static const fix16_t FIX_1    = 0x00010000;
static const fix16_t FIX_0    = 0x00000000;
static const fix16_t FIX_NEG1 = 0xFFFF0000;
static const fix16_t FIX_095  = 0x0000F333;

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
    (void)pad_in;
    return;
}

void Humanizer::process_stick(
    int16_t& x, int16_t& y,
    fix16_t& drift_x, fix16_t& drift_y,
    fix16_t& target_x, fix16_t& target_y,
    uint32_t& retarget_counter,
    uint32_t& fade_counter,
    bool& was_idle)
{
    fix16_t nx = fp_div(fp_from_int(x), fp_from_int(32767));
    fix16_t ny = fp_div(fp_from_int(y), fp_from_int(32767));

    fix16_t mag_sq = fp_mul(nx, nx) + fp_mul(ny, ny);
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
        drift_x = drift_x + fp_mul(target_x - drift_x, settings_.drift_strength);
        drift_y = drift_y + fp_mul(target_y - drift_y, settings_.drift_strength);
        nx = nx + drift_x;
        ny = ny + drift_y;
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

    x = static_cast<int16_t>(fp_to_int(fp_mul(nx, fp_from_int(32767))));
    y = static_cast<int16_t>(fp_to_int(fp_mul(ny, fp_from_int(32767))));
}
