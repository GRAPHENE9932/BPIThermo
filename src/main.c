#include "hdc2080.h"
#include "leds/leds.h"
#include "brightness_control.h"
#include "f_cpu.h"
#include "bat_mon.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/power.h>
#include <avr/sleep.h>

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
#define SEVEN_SEGMENT_A 0b01110111
#define SEVEN_SEGMENT_t 0b01111000
#define SEVEN_SEGMENT_L 0b00111000
#define SEVEN_SEGMENT_o 0b01011100
#define SEVEN_SEGMENT_C 0b00111001

// Power off will be initiated after BAT_CRIT_TICKS_THRESHOLD ticks under
// critical battery voltage.
#define BAT_CRIT_TICKS_THRESHOLD 60

enum content : uint8_t {
    CONT_TEMP_AND_HUM = 0,
    CONT_BRIGHTNESS = 1,
    CONT_BATT_LO_MSG = 2,
    CONT_BATT_CR_MSG = 3,
};

struct display_state {
    fixed16 red_number;
    uint8_t blue_number;
    enum content content;
};

static fixed16 cur_brightness;
static uint8_t bat_crit_ticks;

static void put_number_on_red_leds(fixed16 number) {
    bool is_negative = false;
    if (number < 0) {
        number = -number;
        is_negative = true;
    }

    uint8_t digits[5]; // [4] [3] [2].[1] [0]

    uint8_t integer_part = number >> 8;

    digits[4] = integer_part / 100;
    digits[3] = integer_part / 10 - digits[4] * 10;
    digits[2] = integer_part % 10;

    uint8_t fractional_part = number;

    digits[1] = ((uint16_t)fractional_part * 10) >> 8;
    digits[0] = (((uint16_t)fractional_part * 100) >> 8) - digits[1] * 10;

    // Assigment of the LED values could be done in a loop, but there are so few iterations
    // and so many conditions, it would be much more comprehensible without a loop.
    if (is_negative) {
        leds[5] = SEVEN_SEGMENT_MINUS;

        if (digits[4] == 0 && digits[3] == 0) {
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
        if (digits[4] == 0 && digits[3] == 0) {
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

static void put_batt_cr_on_leds(void) {
    leds[5] = SEVEN_SEGMENT_b;
    leds[4] = SEVEN_SEGMENT_A;
    leds[3] = SEVEN_SEGMENT_t;
    leds[2] = SEVEN_SEGMENT_t;
    leds[1] = SEVEN_SEGMENT_C;
    leds[0] = SEVEN_SEGMENT_r;
}

static void put_batt_lo_on_leds(void) {
    leds[5] = SEVEN_SEGMENT_b;
    leds[4] = SEVEN_SEGMENT_A;
    leds[3] = SEVEN_SEGMENT_t;
    leds[2] = SEVEN_SEGMENT_t;
    leds[1] = SEVEN_SEGMENT_L;
    leds[0] = SEVEN_SEGMENT_o;
}

static void power_off(void) {
    leds_power_off();
    hdc2080_power_off();
    cli();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_cpu();
}

static bool display_state_eq(struct display_state* ds_1, struct display_state* ds_2) {
    if (ds_1->content != ds_2->content)
        return false;

    bool result = true;
    switch (ds_1->content) {
    case CONT_TEMP_AND_HUM:
        result &= ds_1->blue_number == ds_2->blue_number;
        [[fallthrough]];
    case CONT_BRIGHTNESS:
        result &= ds_1->red_number == ds_2->red_number;
        [[fallthrough]];
    case CONT_BATT_LO_MSG:
    case CONT_BATT_CR_MSG:
        break;
    }

    return result;
}

static void control_tick(struct display_state* ds, uint16_t tick_count) {
    switch (bat_get_state()) {
    case BAT_CRIT:
        ds->content = CONT_BATT_CR_MSG;
        if (++bat_crit_ticks >= BAT_CRIT_TICKS_THRESHOLD)
            power_off();
        return;
    case BAT_LOW:
        bat_crit_ticks = 0;
        // Show the battery low message every ~4 seconds for ~0,5s.
        if (tick_count % 256 < 32) {
            ds->content = CONT_BATT_LO_MSG;
            return;
        }
        break;
    default:
        bat_crit_ticks = 0;
        break;
    }

    static uint8_t brightness_disp_time = 0;
    brightness_control_update();
    if (brightness_control_changed()) {
        brightness_disp_time = 30;
    }
    if (brightness_disp_time > 0) {
        fixed16 br = brightness_control_get_percentage();
        ds->content = CONT_BRIGHTNESS;
        ds->red_number = br;
        cur_brightness = br;
        brightness_disp_time--;
        return;
    }

    static struct hdc2080_data data;
    if (hdc2080_is_measurement_over())
        data = hdc2080_acquire_data();

    ds->content = CONT_TEMP_AND_HUM;
    ds->red_number = data.temperature;
    ds->blue_number = data.humidity;
}

static void display_tick(struct display_state* ds) {
    static struct display_state prev_ds;

    if (display_state_eq(ds, &prev_ds)) {
        return;
    }

    switch (ds->content) {
    case CONT_TEMP_AND_HUM:
        put_temperature_on_leds(ds->red_number);
        put_humidity_on_leds(ds->blue_number);
        break;
    case CONT_BRIGHTNESS:
        put_brightness_on_leds(ds->red_number);
        break;
    case CONT_BATT_CR_MSG:
        put_batt_cr_on_leds();
        break;
    case CONT_BATT_LO_MSG:
        put_batt_lo_on_leds();
        break;
    }
}

// The tick is supposed to be issued very roughly 60 times a second.
#define TICK_PERIOD 24
static void tick(void) {
    static uint16_t tick_count;
    tick_count++;

    struct display_state ds;
    control_tick(&ds, tick_count);
    display_tick(&ds);
}

int main(void) {
    clock_prescale_set(clock_div_1); // Disable the default /8 prescaler.

    bat_mon_init();
    leds_init();
    hdc2080_init();
    brightness_control_init();
    cur_brightness = brightness_control_get_percentage();

    uint8_t frame_counter = 0;
    while (1) {
        leds_flash_once(cur_brightness);
        
        if (++frame_counter >= TICK_PERIOD) {
            tick();
            frame_counter = 0;
        }
    }
}
