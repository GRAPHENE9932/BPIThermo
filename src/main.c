#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/power.h>
#include <stdbool.h>
#include "hdc2080.h"

static void debug_shiftout(uint8_t byte) {
    bool cur_bit = byte & 0b10000000 ? true : false;
    for (uint8_t i = 0; i < 8; i++) {
        PORTB |= (1 << PB7);
        _delay_ms(100);
        PORTB &= ~(1 << PB7);
        _delay_ms(100);

        if (cur_bit) {
            PORTB |= (1 << PB7);
            _delay_ms(100);
            PORTB &= ~(1 << PB7);
            _delay_ms(100);
        }

        _delay_ms(1000);

        byte <<= 1;
        cur_bit = byte & 0b10000000 ? true : false;
    }
}

int main(void) {
    clock_prescale_set(clock_div_1); // Disable the default /8 prescaler.

    DDRB |= (1 << PB7);

    hdc2080_init();
    while (1) {
        fixed16 celsius = hdc2080_measure_temperature_sync();
        debug_shiftout(celsius >> 8);
        debug_shiftout(celsius);

        _delay_ms(5000);
    }

    while (1) {
        PORTB ^= (1 << PB7);
        _delay_ms(1000);
    }
}
