#ifndef HDC2080_H
#define HDC2080_H

#include "fixed16.h"

#include <stdbool.h>

void hdc2080_init(void);
void hdc2080_start_measurement(void);
bool hdc2080_is_measurement_over(void);
fixed16 hdc2080_get_temperature_celsius(void);

#endif // HDC2080_H
