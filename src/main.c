#include "hdc2080.h"
#include "leds.h"
#include "brightness_control.h"

#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/power.h>
#include <stdbool.h>

static const uint8_t SEVEN_SEGMENT_DIGITS[10] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01100111, // 9
};

#define SEVEN_SEGMENT_MINUS 0b01000000
#define SEVEN_SEGMENT_DP 0b10000000
#define SEVEN_SEGMENT_H 0b01110110
#define SEVEN_SEGMENT_I 0b00000110
#define SEVEN_SEGMENT_b 0b01111100
#define SEVEN_SEGMENT_r 0b01010000

static void put_number_on_red_leds(fixed16 number) {
    bool is_negative = false;
    if (number < 0) {
        number = -number;
        is_negative = true;
    }

    uint8_t digits[5]; // [4] [3] [2].[1] [0]

    uint8_t integer_part = number >> 8;

    digits[4] = integer_part / 100;
    digits[3] = integer_part / 10 - digits[4] * 100;
    digits[2] = integer_part % 10;

    uint8_t fractional_part = number;

    digits[1] = ((uint16_t)fractional_part * 10) >> 8;
    digits[0] = (((uint16_t)fractional_part * 100) >> 8) - digits[1] * 10;

    // Assigment of the LED values could be done in a loop, but there are so few iterations
    // and so many conditions, it would be much more comprehensible without a loop.
    if (is_negative) {
        leds[5] = SEVEN_SEGMENT_MINUS;

        if (digits[3] == 0) {
            leds[4] = SEVEN_SEGMENT_DIGITS[digits[2]] | SEVEN_SEGMENT_DP;
            leds[3] = SEVEN_SEGMENT_DIGITS[digits[1]];
            leds[2] = SEVEN_SEGMENT_DIGITS[digits[0]];
        }
        else if (digits[4] == 0) {
            leds[4] = SEVEN_SEGMENT_DIGITS[digits[3]];
            leds[3] = SEVEN_SEGMENT_DIGITS[digits[2]] | SEVEN_SEGMENT_DP;
            leds[2] = SEVEN_SEGMENT_DIGITS[digits[1]];
        }
        else {
            leds[4] = SEVEN_SEGMENT_DIGITS[digits[4]];
            leds[3] = SEVEN_SEGMENT_DIGITS[digits[3]];
            leds[2] = SEVEN_SEGMENT_DIGITS[digits[2]] | SEVEN_SEGMENT_DP;
        }
    }
    else {
        if (digits[3] == 0) {
            leds[5] = 0b00000000;
            leds[4] = SEVEN_SEGMENT_DIGITS[digits[2]] | SEVEN_SEGMENT_DP;
            leds[3] = SEVEN_SEGMENT_DIGITS[digits[1]];
            leds[2] = SEVEN_SEGMENT_DIGITS[digits[0]];
        }
        else if (digits[4] == 0) {
            leds[5] = SEVEN_SEGMENT_DIGITS[digits[3]];
            leds[4] = SEVEN_SEGMENT_DIGITS[digits[2]] | SEVEN_SEGMENT_DP;
            leds[3] = SEVEN_SEGMENT_DIGITS[digits[1]];
            leds[2] = SEVEN_SEGMENT_DIGITS[digits[0]];
        }
        else {
            leds[5] = SEVEN_SEGMENT_DIGITS[digits[4]];
            leds[4] = SEVEN_SEGMENT_DIGITS[digits[3]];
            leds[3] = SEVEN_SEGMENT_DIGITS[digits[2]] | SEVEN_SEGMENT_DP;
            leds[2] = SEVEN_SEGMENT_DIGITS[digits[1]];
        }
    }
}

static void put_temperature_on_leds(fixed16 temperature) {
    put_number_on_red_leds(temperature);
}

static void put_humidity_on_leds(uint8_t humidity) {
    // We only have two seven segment digits for our disposal.
    // So, when humidity is 100, display "HI".
    if (humidity >= 100) {
        leds[1] = SEVEN_SEGMENT_H;
        leds[0] = SEVEN_SEGMENT_I;
        return;
    }

    leds[1] = SEVEN_SEGMENT_DIGITS[humidity / 10];
    leds[0] = SEVEN_SEGMENT_DIGITS[humidity % 10];
}

static void put_brightness_on_leds(fixed16 brightness) {
    leds[1] = SEVEN_SEGMENT_b;
    leds[0] = SEVEN_SEGMENT_r;

    put_number_on_red_leds(brightness);
}

int main(void) {
    clock_prescale_set(clock_div_1); // Disable the default /8 prescaler.

    leds_init();
    hdc2080_init();
    brightness_control_init();
    fixed16 cached_brightness = brightness_control_get_percentage();

    while (1) {
        leds_flash_once(cached_brightness);
        if (hdc2080_is_measurement_over()) {
            struct hdc2080_data data = hdc2080_acquire_data();
            put_temperature_on_leds(data.temperature);
            put_humidity_on_leds(data.humidity);
        }

        brightness_control_update();
        if (brightness_control_changed()) {
            cached_brightness = brightness_control_get_percentage();
            put_brightness_on_leds(cached_brightness);
        }
    }

    while (1) {
        PORTB ^= (1 << PB7);
        _delay_ms(1000);
    }
}
