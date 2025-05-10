#ifndef BRIGHTNESS_CONTROL_H
#define BRIGHTNESS_CONTROL_H

#include "fixed16.h"

#include <stdbool.h>

void brightness_control_init(void);
void brightness_control_update(void);
bool brightness_control_changed(void);
fixed16 brightness_control_get_percentage(void);

#endif // BRIGHTNESS_CONTROL_H
