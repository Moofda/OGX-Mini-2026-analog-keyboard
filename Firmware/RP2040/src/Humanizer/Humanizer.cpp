#include "Humanizer/Humanizer.h"

void Humanizer::process(Gamepad::PadIn& pad_in)
{
    if (!settings_.enabled) return;

    // Simple idle check using integer math only
    bool lx_idle = (pad_in.joystick_lx > -800 && pad_in.joystick_lx < 800);
    bool ly_idle = (pad_in.joystick_ly > -800 && pad_in.joystick_ly < 800);
    bool rx_idle = (pad_in.joystick_rx > -800 && pad_in.joystick_rx < 800);
    bool ry_idle = (pad_in.joystick_ry > -800 && pad_in.joystick_ry < 800);

    // Simple drift using integer counters
    if (lx_idle && ly_idle)
    {
        retarget_counter_l_++;
        if (retarget_counter_l_ >= settings_.drift_retarget_frames)
        {
            retarget_counter_l_ = 0;
            rng_state_ ^= rng_state_ << 13;
            rng_state_ ^= rng_state_ >> 17;
            rng_state_ ^= rng_state_ << 5;
            int16_t drift = (int16_t)(rng_state_ % 800);
            pad_in.joystick_lx += drift;
            pad_in.joystick_ly += (int16_t)((rng_state_ >> 8) % 800);
        }
    }
    else
    {
        retarget_counter_l_ = 0;
    }

    if (rx_idle && ry_idle)
    {
        retarget_counter_r_++;
        if (retarget_counter_r_ >= settings_.drift_retarget_frames)
        {
            retarget_counter_r_ = 0;
            rng_state_ ^= rng_state_ << 13;
            rng_state_ ^= rng_state_ >> 17;
            rng_state_ ^= rng_state_ << 5;
            pad_in.joystick_rx += (int16_t)(rng_state_ % 800);
            pad_in.joystick_ry += (int16_t)((rng_state_ >> 8) % 800);
        }
    }
    else
    {
        retarget_counter_r_ = 0;
    }
}

void Humanizer::process_stick(
    int16_t& x, int16_t& y,
    float& drift_x, float& drift_y,
    float& target_x, float& target_y,
    uint32_t& retarget_counter,
    uint32_t& fade_counter,
    bool& was_idle)
{
    (void)x; (void)y;
    (void)drift_x; (void)drift_y;
    (void)target_x; (void)target_y;
    (void)retarget_counter;
    (void)fade_counter;
    (void)was_idle;
}

float Humanizer::next_rand() { return 0; }
float Humanizer::next_rand_pos() { return 0; }
