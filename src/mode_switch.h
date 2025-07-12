#ifndef MODE_SWITCH_H
#define MODE_SWITCH_H

#include "mode.h"

#include <avr/io.h>

#define MODE_SWITCH_PORT PORTC
#define MODE_SWITCH_PIN PINC
#define MODE_SWITCH_DDR DDRC
#define MODE_SWITCH_BIT 0

static inline void mode_switch_init(void) {
    // Input pin, enable internal pull-up.
    MODE_SWITCH_DDR &= ~(1 << MODE_SWITCH_BIT);
    MODE_SWITCH_PORT |= (1 << MODE_SWITCH_BIT);
}

static inline enum mode mode_switch_state(void) {
    return !(MODE_SWITCH_PIN & (1 << MODE_SWITCH_BIT));
}

#endif // MODE_SWITCH_H
