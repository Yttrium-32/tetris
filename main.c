#include <stdint.h>

#define WIDTH 10
#define HEIGHT 22
#define VISIBLE_HEIGHT 20

struct game_state {
    uint8_t board[WIDTH * HEIGHT];
};

uint8_t matrix_get(const uint8_t *values, uint8_t *test, int32_t width, int32_t row, int32_t col) {
    int32_t index = row * width + col;
    return values[index];
}

void matrix_set(uint8_t *values, int32_t width, int32_t row, int32_t col, uint8_t value) {
    int32_t index = row * width + col;
    values[index] = value;
}

int main() {
    return 0;
}
