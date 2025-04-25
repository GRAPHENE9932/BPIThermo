#ifndef HDC2080_H
#define HDC2080_H

#include "fixed16.h"

#include <stdbool.h>

struct hdc2080_data {
    fixed16 temperature;
    uint8_t humidity;
};

// After initialization, the HDC2080 will begin measuring temperature and
// humidity with 14 bit accuracy 5 times a second at once.
void hdc2080_init(void);
bool hdc2080_is_measurement_over(void);
struct hdc2080_data hdc2080_acquire_data(void);

#endif // HDC2080_H
