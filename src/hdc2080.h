#ifndef HDC2080_H
#define HDC2080_H

#include "fixed_point.h"
#include "mode.h"

#include <stdbool.h>

struct hdc2080_data {
    fixed16_8 temperature;
    uint8_t humidity;
};

// After initialization, the HDC2080 will begin measuring temperature and
// humidity with 14 bit accuracy 5 times a second at once.
void hdc2080_init(void);
bool hdc2080_is_measurement_over(void);
struct hdc2080_data hdc2080_acquire_data(enum mode mode);
void hdc2080_power_off(void);

#endif // HDC2080_H
