#include "eeprom.h"

#include <avr/interrupt.h>
#include <avr/io.h>

#include <stdbool.h>

#define WRITE_BUF_SIZE 2

static uint8_t bytes_written;
static const uint8_t bytes_to_write_total = 3;
static uint8_t pending_bytes[WRITE_BUF_SIZE];
static uint8_t pending_address;

uint32_t eeprom_read_3_bytes(uint8_t address) {
    while (EECR & (1 << EEPE)); // Wait for other EEPROM operations.

    cli();
    EEARL = address;
    EECR |= (1 << EERE);
    uint8_t result_low = EEDR;
    
    EEARL = address + 1;
    EECR |= (1 << EERE);
    uint8_t result_mid = EEDR;

    EEARL = address + 2;
    EECR |= (1 << EERE);
    uint8_t result_high = EEDR;
    sei();

    return (uint32_t)result_low | ((uint32_t)result_mid << 8) | ((uint32_t)result_high << 16);
}

static void write_byte_no_wait(uint8_t address, uint8_t data) {
    EECR &= ~((1 << EEPM1) | (1 << EEPM0)); // Set the programming mode to atomic.
    EEARL = address;
    EEDR = data;
    EECR |= (1 << EEMPE);
    EECR |= (1 << EEPE); // Initiate the write.
}

void eeprom_write_3_bytes_async(uint8_t address, uint32_t data) {
    while (EECR & (1 << EEPE)); // Wait for other EEPROM operations.

    cli();
    EECR |= (1 << EERIE); // Enable the EEPROM Ready Interrupt.
    write_byte_no_wait(address, data); // Initiate the write of the least significant byte.
    pending_bytes[0] = data >> 8;
    pending_bytes[1] = data >> 16;
    pending_address = address + 1;
    bytes_written = 1;
    sei();
}

ISR(EE_READY_vect) {
    if (bytes_written == bytes_to_write_total) {
        EECR &= ~(1 << EERIE); // Disable the EEPROM Ready Interrupt.
        return;
    }

    write_byte_no_wait(pending_address, pending_bytes[bytes_written - 1]);
    bytes_written++;
    pending_address++;
}
