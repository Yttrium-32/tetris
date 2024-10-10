#include <stdint.h>

#define WIDTH 10
#define HEIGHT 22
#define VISIBLE_HEIGHT 20

/*
 * Struct representing state of each tetrino
 * tetrino_index selects one of 8 tetrinos
 */
struct piece_state {
    uint8_t tetrino_index;
    int32_t offset_row;
    int32_t offset_col;
    int32_t rotation;
};

struct game_state {
    uint8_t board[WIDTH * HEIGHT];
    struct piece_state piece;
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
