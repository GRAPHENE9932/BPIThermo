#ifndef HDC2080_H
#define HDC2080_H

#include "fixed16.h"

void hdc2080_init(void);
fixed16 hdc2080_measure_temperature_sync(void);

#endif // HDC2080_H
