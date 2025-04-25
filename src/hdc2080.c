#define F_CPU 8000000UL
#include "hdc2080.h"

#include <stdbool.h>
#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>

#define HDC2080_I2C_ADDRESS_W 0b10000000
#define HDC2080_I2C_ADDRESS_R 0b10000001
#define PORT_DRDY PORTC
#define PIN_DRDY PINC
#define DDR_DRDY DDRC
#define BIT_DRDY 3

static void debug_error(void) {
    while (1) {
        PORTB ^= (1 << PB7);
        _delay_ms(50);
    }
}

static void i2c_init(void) {
    // Enable internal pull-ups on the I2C pins (PC4 is SDA, PC5 is SCL).
    DDRC &= ~((1 << PC4) | (1 << PC5));
    PORTC |= (1 << PC4) | (1 << PC5);

    // Set the I2C frequency to 50 kHz.
    TWBR = 72;
    TWSR = 0; // Prescaler to 1.
}

static void i2c_mt_start(uint8_t address) {
    // Enable TWI, clear the interrupt flag and command generation of start condition.
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

    while (!(TWCR & (1 << TWINT))); // Wait until the bus is freed and START is sent.

    // Check if the status is indicating successful START.
    if ((TWSR & TW_STATUS_MASK) != TW_START) {
        debug_error();
    }

    // Load the address into the data register and clear the interrupt flag to start transmission.
    TWDR = address;
    TWCR = (1 << TWINT) | (1 << TWEN);

    while (!(TWCR & (1 << TWINT))); // Wait until the transmission is complete.

    // Check if the status is indicating successful ACK.
    if ((TWSR & TW_STATUS_MASK) != TW_MT_SLA_ACK) {
        debug_error();
    }
}

static void i2c_mt(uint8_t data) {
    while (!(TWCR & (1 << TWINT))); // Wait until the interrupt flag is set.

    // Load the data into data register and clear the interrupt flag to start transmission.
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);

    while (!(TWCR & (1 << TWINT))); // Wait until the transmission is complete.

    // Check if the status is indicating successful ACK.
    if ((TWSR & TW_STATUS_MASK) != TW_MT_DATA_ACK) {
        debug_error();
    }
}

static void i2c_mt_stop(void) {
    // Send STOP condition.
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);

    // Wait for STOP to end.
    while (TWCR & (1 << TWSTO));
}

static void i2c_mr_start(uint8_t address) {
    // Enable TWI, clear the interrupt flag and command generation of start condition.
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

    while (!(TWCR & (1 << TWINT))); // Wait until the bus is freed and START is sent.

    // Check if the status is indicating successful START.
    if ((TWSR & TW_STATUS_MASK) != TW_START) {
        debug_error();
    }

    // Load the address into the data register and clear the interrupt flag to start transmission.
    TWDR = address;
    TWCR = (1 << TWINT) | (1 << TWEN);

    while (!(TWCR & (1 << TWINT))); // Wait until the transmission is complete.

    // Check if the status is indicating successful ACK.
    if ((TWSR & TW_STATUS_MASK) != TW_MR_SLA_ACK) {
        debug_error();
    }
}

static uint8_t i2c_mr(bool last) {
    if (last) {
        // Prepare to transmission of NACK after receiving.
        TWCR = (1 << TWINT) | (1 << TWEN);
    }
    else {
        // Prepare to transmission of ACK after receiving.
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    }

    while (!(TWCR & (1 << TWINT))); // Wait until reception is complete.

    uint8_t status = TWSR & TW_STATUS_MASK;
    if (last && status != TW_MR_DATA_NACK) {
        debug_error();
    }
    else if (!last && status != TW_MR_DATA_ACK) {
        debug_error();
    }

    return TWDR; // Get the received data.
}

static void i2c_mr_stop(void) {
    while (!(TWCR & (1 << TWINT))); // Wait until the interrupt flag is set.
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); // Set the stop flag.
}

void hdc2080_init(void) {
    DDR_DRDY &= ~(1 << BIT_DRDY); // Set up the pin connected to DRDY of HDC2080 as input without pullup.
    PORT_DRDY &= ~(1 << BIT_DRDY);

    i2c_init();

    i2c_mt_start(HDC2080_I2C_ADDRESS_W);
    i2c_mt(0x0E); // Select the Reset and DRDY/INT Configuration Register.
    i2c_mt(0b00000110); // Enable the DRDY/INT pin and set interrupt polarity to Active High.
    i2c_mt_stop();

    i2c_mt_start(HDC2080_I2C_ADDRESS_W);
    i2c_mt(0x07); // Select the Interrupt Configuration register.
    i2c_mt(0b10000000); // Enable the DataReady interrupt.
    i2c_mt_stop();
}

void hdc2080_start_measurement(void) {
    i2c_mt_start(HDC2080_I2C_ADDRESS_W);
    i2c_mt(0x0F); // Select the MEASUREMENT CONFIGURATION register.
    // Set temperature resolution to "14 bit", humidity resolution to "14 bit",
    // measurement configuration to "Humidity + Temperature" and measurement trigger to "Start measurement".
    i2c_mt(0b00000001);
    i2c_mt_stop();
}

bool hdc2080_is_measurement_over(void) {
    return PIN_DRDY & (1 << BIT_DRDY);
}

#define UFIXED16_165_BY_256 ((ufixed16)165) // Exactly 165/256, or 0.64453125.
#define UFIXED16_40_596 0x2899 // Supposed to be 40.596, actually 40.59765625.

static fixed16 convert_raw_to_celsius(ufixed16 raw) {
    // Formula from the datasheet is:
    // Temperature (C) = (TEMPERATURE[15:0] / 2^16) * 165 - (40.5 + TEMP_PSRR * (V_DD - 1.8 V))
    // According to the datasheet, TEMP_PSRR = 0.08 C/V and the input voltage of the HDC2080, in
    // case with BPIThermo Revision A, is 3.0V.
    // Substituting and simplifying, we get:
    // Temperature (C) = (TEMPERATURE[15:0] / 2^8) * (165 / 2^8) - 40.596.

    // We must use unsigned values here, as the raw temperature can range from 0.0 to about 255.99609375,
    // which exceeds the fixed16 range.
    return ufixed16_mul(raw, UFIXED16_165_BY_256) - UFIXED16_40_596;
}

fixed16 hdc2080_get_temperature_celsius(void) {
    i2c_mt_start(HDC2080_I2C_ADDRESS_W);
    i2c_mt(0x00); // Select the TEMPERATURE LOW register.
    i2c_mt_stop();

    i2c_mr_start(HDC2080_I2C_ADDRESS_R);
    uint8_t temp_low = i2c_mr(false); // Read out TEMPERATURE LOW.
    uint8_t temp_high = i2c_mr(true); // Read out TEMPERATURE HIGH.
    i2c_mr_stop();

    return convert_raw_to_celsius((ufixed16)temp_high << 8 | temp_low);
}
