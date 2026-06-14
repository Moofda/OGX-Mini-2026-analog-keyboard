#include "Humanizer/Humanizer.h"

void Humanizer::process(Gamepad::PadIn& pad_in)
{
    // passthrough - no processing
    (void)pad_in;
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
