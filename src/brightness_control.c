#include "brightness_control.h"

#include <avr/io.h>

#define PORT_BR PORTC
#define PIN_BR PINC
#define DDR_BR DDRC
#define BIT_BR_DOWN 1
#define BIT_BR_UP 2

#define FIXED16_100 0x6400 // 100.0
#define FIXED16_97_082 0x6115 // 97.08203125
#define FIXED16_0_25 0x0040 // 0.25
#define FIXED16_0_1 0x0019 // 0.1 (actually 0.09765625)
#define FIXED16_0_01 0x0003 // 0.01 (actually 0.01171875)

static fixed16 perceived_brightness = FIXED16_97_082;
static bool changed = false;
static bool special_command = false;

void brightness_control_init(void) {
    // Make these pin input and enable internal pull-ups.
    DDR_BR &= ~((1 << BIT_BR_DOWN) | (1 << BIT_BR_UP));
    PORT_BR |= (1 << BIT_BR_DOWN) | (1 << BIT_BR_UP);
}

void brightness_control_update(void) {
    const bool br_down_pressed = !(PIN_BR & (1 << BIT_BR_DOWN));
    const bool br_up_pressed = !(PIN_BR & (1 << BIT_BR_UP));
    changed = br_down_pressed != br_up_pressed;
    special_command = br_down_pressed && br_up_pressed;

    if (!changed) {
        return;
    }
    
    if (br_down_pressed) {
        perceived_brightness -= FIXED16_0_25;
    }
    if (br_up_pressed) {
        perceived_brightness += FIXED16_0_25;
    }

    // ~97 and not 100 is used as a maximum value because of poor precision of 16-bit fixed
    // point numbers. If we use 100 for the formula in brightness_control_get_percentage
    // we get more than 106 as a result, so I've adjusted the limits a little.
    perceived_brightness = FIXED16_CLAMP(perceived_brightness, FIXED16_0_25, FIXED16_97_082);
}

bool brightness_control_changed(void) {
    return changed;
}

fixed16 brightness_control_get_percentage(void) {
    // The buttons are modifying perceived buttons, but the real brightness needs to be calculated.
    // Using a bold approximation, Y(L*) = 0.0001 * L*^3, where Y is luminance and L* is lightness.
    // Adapting this equation to the limited fixed16 range we get:
    // Y(L*) = 0.01L* * 0.1L* * 0.1L*
    fixed16 result = fixed16_mul(FIXED16_0_1, perceived_brightness);
    result = fixed16_mul(result, result);
    fixed16 tmp = fixed16_mul(FIXED16_0_01, perceived_brightness);
    result = fixed16_mul(result, tmp);

    if (result > FIXED16_100) {
        result = FIXED16_100;
    }

    return result;
}
