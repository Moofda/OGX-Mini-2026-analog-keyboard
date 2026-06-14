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
    (void)x; (void)y; (void)drift_x; (void)drift_y;
    (void)target_x; (void)target_y; (void)retarget_counter;
    (void)fade_counter; (void)was_idle;
}
