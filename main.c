#include <stdint.h>

#define WIDTH 10
#define HEIGHT 22
#define VISIBLE_HEIGHT 20

#define construct_tetrino(data, side) {data, side}

/*
 * Reprsents the shape of the tetrino
 * Here, `data` is square matrix
 * each side of the matrix is of lenght `side`
 */
typedef struct {
    const uint8_t *data;
    const int32_t side;
} tetrino;

const uint8_t TETRINO_1[] = {
    0, 0, 0, 0,
    1, 1, 1, 1,
    0, 0, 0, 0,
    0, 0, 0, 0,
};

const uint8_t TETRINO_2[] = {
    2, 2,
    2, 2
};
 
const uint8_t TETRINO_3[] = {
    0, 0, 0,
    3, 3, 3,
    0, 3, 0
};

const tetrino TETRINOS[] = {
    construct_tetrino(TETRINO_1, 4),
    construct_tetrino(TETRINO_2, 2),
    construct_tetrino(TETRINO_3, 3),
};

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
