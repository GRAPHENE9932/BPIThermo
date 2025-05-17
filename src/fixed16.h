#ifndef FIXED16_H
#define FIXED16_H

#include <stdint.h>

#define CLAMP(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))

typedef int16_t fixed16;
typedef uint16_t ufixed16;

fixed16 fixed16_mul(fixed16 a, fixed16 b);
ufixed16 ufixed16_mul(ufixed16 a, ufixed16 b);

#endif
