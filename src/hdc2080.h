#ifndef HDC2080_H
#define HDC2080_H

#include "fixed16.h"

#include <stdbool.h>

// After initialization, the HDC2080 will begin measuring temperature and
// humidity with 14 bit accuracy 5 times a second at once.
void hdc2080_init(void);
bool hdc2080_is_measurement_over(void);
fixed16 hdc2080_get_temperature_celsius(void);

#endif // HDC2080_H
