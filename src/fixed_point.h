#ifndef FIXED_POINT_H
#define FIXED_POINT_H

#include <stdint.h>

#define CLAMP(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))

typedef int32_t fixed16_8;
typedef int16_t fixed16;

static inline fixed16 fixed16_mul(fixed16 a, fixed16 b) {
    return ((int32_t)a * (int32_t)b) >> 8;
}

static inline fixed16_8 fixed16_8_mul(fixed16_8 a, fixed16_8 b) {
    return ((a * b) >> 8);
}

#endif // FIXED_POINT_H
