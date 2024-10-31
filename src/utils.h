#ifndef UTILS
#define UTILS

#include <stdint.h>

#define min(a, b) (a < b) ? a : b
#define max(a, b) (a > b) ? a : b

uint8_t matrix_get(const uint8_t *values, int32_t width, int32_t row, int32_t col);
void matrix_set(uint8_t *values, int32_t width, int32_t row, int32_t col, uint8_t value);

#endif // UTILS
