#include "fixed16.h"

fixed16 fixed16_mul(fixed16 a, fixed16 b) {
    return ((int32_t)a * (int32_t)b) >> 8;
}

ufixed16 ufixed16_mul(ufixed16 a, ufixed16 b) {
    return ((uint32_t)a * (uint32_t)b) >> 8;
}
