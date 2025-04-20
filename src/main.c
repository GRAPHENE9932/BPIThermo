#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/power.h>

int main(void) {
    clock_prescale_set(clock_div_1); // Disable the default /8 prescaler.

    DDRB |= (1 << PB7);

    while (1) {
        PORTB ^= (1 << PB7);
        _delay_ms(250);
    }
}
