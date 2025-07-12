#ifndef LEDS_H
#define LEDS_H

#include "../fixed_point.h"

#include <stdint.h>

// Buffer that stores the desired state of 7-segment display's LEDs.
// Each byte is responsible for its digit on 7-segment display.
// 0th byte is responsible for the least significant blue digit.
// 1st byte for the most significant blue digit.
// 2nd byte for the least significant red digit.
// And so on.
// In the each byte, its bits are responsible for the corresponding LED segments (starting from LSB):
// A B C D E F G DP
extern uint8_t leds[6];

void leds_init(void);
// Brightness must be in range [1/256, 100.0].
void leds_flash_once(fixed16 brightness);
void leds_power_off(void);

#endif // LEDS_H
