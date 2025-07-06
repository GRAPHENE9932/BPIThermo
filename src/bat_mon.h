#ifndef BAT_MON_H
#define BAT_MON_H

#include <stdint.h>
#include <stdbool.h>

#define BAT_MON_VOLTAGE_UNKNOWN 0xFFFF
// The voltage divider between the battery and the ADC pin has ratio of
// 0,2444. The voltage reference of the ADC is internal (1,1V).
// The number below corresponds to 2,997V.
#define BAT_MON_VOLTAGE_CRITICAL 682
#define BAT_MON_VOLTAGE_LOW 751 // 3,300V.

enum bat_state : uint8_t {
    BAT_NORMAL = 0,
    BAT_LOW = 1,
    BAT_CRIT = 2,
};

extern volatile uint16_t bat_mon_voltage;

void bat_mon_init(void);
enum bat_state bat_get_state(void);

#endif // BAT_MON_H
