#include "brightness_control.h"

#include <avr/io.h>

#define PORT_BR PORTC
#define PIN_BR PINC
#define DDR_BR DDRC
#define BIT_BR_DOWN 1
#define BIT_BR_UP 2

#define FIXED16_100 0x6400 // 100.0
#define FIXED16_10_256 0x000A // 10/256 = 0.0390625
#define FIXED16_0_1 0x001A // 0.1 (actually 0.1015625)

static fixed16 perceived_brightness = FIXED16_100;
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
    
    if (br_down_pressed && perceived_brightness > FIXED16_10_256) {
        perceived_brightness -= FIXED16_10_256;
    }
    else if (br_up_pressed && perceived_brightness < FIXED16_100) {
        perceived_brightness += FIXED16_10_256;
    }
}

bool brightness_control_changed(void) {
    return changed;
}

fixed16 brightness_control_get_percentage(void) {
    // The buttons are modifying perceived buttons, but the real brightness needs to be calculated.
    // Using a bold approximation, Y(L*) = 0,01 * L*^2, where Y is luminance and L* is lightness.
    const fixed16 tmp = fixed16_mul(FIXED16_0_1, perceived_brightness);
    fixed16 result = fixed16_mul(tmp, tmp);

    if (result > FIXED16_100) {
        result = FIXED16_100;
    }

    return result;
}
