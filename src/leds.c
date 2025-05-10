#include "leds.h"
#include "var_delay.h"
#include "f_cpu.h"

#include <avr/io.h>
#include <util/delay.h>

// Define pins for the TLC6C598 shift register's inputs.
#define PORT_SER_IN_SRCK PORTB
#define DDR_SER_IN_SRCK DDRB
#define BIT_SER_IN 0
#define BIT_SRCK 1
#define PORT_RCK_CLR_G PORTD
#define DDR_RCK_CLR_G DDRD
#define BIT_RCK 7
#define BIT_CLR 6
#define BIT_G 5

// Define the duration of a half of a clock cycle for shift register control in microseconds.
// 5 us corresponds to 100 kHz.
#define HALF_CYCLE_US 5
// Define the max duration of the LED's being on.
#define LED_DUTY_US 100

uint8_t leds[6];

// Considering only delays, this operation lasts for 17*HALF_CYCLE_US.
static void send_byte_to_shift_register(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        uint8_t cur_bit = byte & 0b10000000;

        if (cur_bit) {
            PORT_SER_IN_SRCK |= (1 << BIT_SER_IN);
        }
        else {
            PORT_SER_IN_SRCK &= ~(1 << BIT_SER_IN);
        }

        _delay_us(HALF_CYCLE_US);

        PORT_SER_IN_SRCK |= (1 << BIT_SRCK);

        _delay_us(HALF_CYCLE_US);

        PORT_SER_IN_SRCK &= ~(1 << BIT_SRCK);

        byte <<= 1;
    }

    PORT_RCK_CLR_G |= (1 << BIT_RCK);
    _delay_us(HALF_CYCLE_US);
    PORT_RCK_CLR_G &= ~(1 << BIT_RCK);
}

void leds_init(void) {
    // PD0, PD1, PD2, PD3, PD4, PA2 are controlling P-type enhancement MOSFET
    // gates for the least significant digits respectively.
    DDRD |= (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3) | (1 << PD4);
    PORTD |= (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3) | (1 << PD4);
    DDRA |= (1 << PA2);
    PORTA |= (1 << PA2);

    // Initialize pins controlling the TLC6C598 shift register.
    DDR_SER_IN_SRCK |= (1 << BIT_SER_IN) | (1 << BIT_SRCK);
    PORT_SER_IN_SRCK &= ~((1 << BIT_SER_IN) | (1 << BIT_SRCK));
    DDR_RCK_CLR_G |= (1 << BIT_RCK) | (1 << BIT_CLR) | (1 << BIT_G);
    PORT_RCK_CLR_G |= (1 << BIT_G);
    PORT_RCK_CLR_G &= ~((1 << BIT_RCK) | (1 << BIT_CLR));
    // We just set the CLR bit to 0 to reset it. Now wait half a cycle and return it to the normal state: 1.
    _delay_us(HALF_CYCLE_US);
    PORT_RCK_CLR_G |= (1 << BIT_CLR);
}

#define FIXED16_2 0x0200

static void flash_for_us(fixed16 us) {
    // The variable_delay_for_us can delay for integer amount of microseconds in range [2us, 255us].
    if (us >= FIXED16_2) {
        uint8_t us_int = us >> 8;

        // Brightness checks for safety. Impulse too wide can burn the LEDs.
        if (us_int > LED_DUTY_US) {
            us_int = LED_DUTY_US;
        }
        else if (us_int < 2) {
            us_int = 2;
        }

        PORT_RCK_CLR_G &= ~(1 << BIT_G);
        variable_delay_for_us(us_int);
        PORT_RCK_CLR_G |= (1 << BIT_G);

        us -= us_int << 8; // Remove the time we waited.
    }

    // Now we potentially have 15 additional cycles to shine.
    uint8_t cycles_left = us >> 5;
    if (cycles_left & 0b00001000) {
        PORT_RCK_CLR_G &= ~(1 << BIT_G);
        __asm__ volatile (
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
        );
        PORT_RCK_CLR_G |= (1 << BIT_G);
    }
    if (cycles_left & 0b00000100) {
        PORT_RCK_CLR_G &= ~(1 << BIT_G);
        __asm__ volatile (
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
        );
        PORT_RCK_CLR_G |= (1 << BIT_G);
    }
    if (cycles_left & 0b00000010) {
        PORT_RCK_CLR_G &= ~(1 << BIT_G);
        __asm__ volatile (
            "nop\n\t"
        );
        PORT_RCK_CLR_G |= (1 << BIT_G);
    }
    if (cycles_left & 0b00000001) {
        PORT_RCK_CLR_G &= ~(1 << BIT_G);
        PORT_RCK_CLR_G |= (1 << BIT_G);
    }
}

void leds_flash_once(fixed16 brightness) {
    for (uint8_t i = 0; i < 6; i++) {
        send_byte_to_shift_register(leds[i]);

        switch (i) {
        case 0:
            PORTD &= ~(1 << PD0);
            break;
        case 1:
            PORTD &= ~(1 << PD1);
            break;
        case 2:
            PORTD &= ~(1 << PD2);
            break;
        case 3:
            PORTD &= ~(1 << PD3);
            break;
        case 4:
            PORTD &= ~(1 << PD4);
            break;
        case 5:
            PORTA &= ~(1 << PA2);
            break;
        }

        flash_for_us(brightness);

        switch (i) {
        case 0:
            PORTD |= (1 << PD0);
            break;
        case 1:
            PORTD |= (1 << PD1);
            break;
        case 2:
            PORTD |= (1 << PD2);
            break;
        case 3:
            PORTD |= (1 << PD3);
            break;
        case 4:
            PORTD |= (1 << PD4);
            break;
        case 5:
            PORTA |= (1 << PA2);
            break;
        }
    }
}
