#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>

uint32_t eeprom_read_3_bytes(uint8_t address);
void eeprom_write_3_bytes_async(uint8_t address, uint32_t data);

#endif // EEPROM_H