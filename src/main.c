#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include <SDL2/SDL.h>

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

typedef enum {
    GAME_PHASE_PLAY
} game_phase;

/*
 * Struct representing state of each tetrino
 * tetrino_index selects one of 8 tetrinos
 */
typedef struct {
    uint8_t tetrino_index;
    int32_t offset_row;
    int32_t offset_col;
    int32_t rotation;
} piece_state;

typedef struct {
    uint8_t board[WIDTH * HEIGHT];
    piece_state piece;
    game_phase phase;
} game_state ;

typedef struct {
    int8_t dleft;
    int8_t dright;
    int8_t dup;

} input_state;

uint8_t matrix_get(const uint8_t *values, int32_t width, int32_t row, int32_t col) {
    int32_t index = row * width + col;
    return values[index];
}

void matrix_set(uint8_t *values, int32_t width, int32_t row, int32_t col, uint8_t value) {
    int32_t index = row * width + col;
    values[index] = value;
}

uint8_t tetrino_get(const tetrino *tetrino, int32_t row, int32_t col, int32_t rotation) {
    int32_t side = tetrino -> side;
    switch (rotation) {
        case 0:
            return tetrino -> data[row * side + col];
        case 1:
            return tetrino -> data[(side - col - 1) * side + row];
        case 2:
            return tetrino -> data[(side - row - 1) * side + (side - col - 1)];
        case 3:
            return tetrino -> data[col * side + (side - row - 1)];
    }
    return 0;
}

bool check_piece_valid(const piece_state *piece, const uint8_t *board, int32_t width, int32_t height) {
    const tetrino *tetrino = &TETRINOS[piece->tetrino_index];
    assert(tetrino);

    for (int32_t row = 0; row < height; ++row) {
        for (int32_t col = 0; col < width; ++col) {
            uint8_t value = tetrino_get(tetrino, row, col, piece->rotation);
            if (value > 0) {
                int32_t board_row = piece->offset_row + row;
                int32_t board_col = piece->offset_col + col;
                if (board_row < 0) {
                    return false;
                }
                if (board_row >= height) {
                    return false;
                }
                if (board_col < 0) {
                    return false;
                }
                if (board_col >= width) {
                    return false;
                }
                if (matrix_get(board, width, row, col)) {
                    return false;
                }
            }
        }
    }
    return true;
}

void update_game_play(game_state *game, const input_state *input) {
    piece_state piece = game -> piece;
    if (input -> dleft > 0) {
        --piece.offset_col;
    }
    if (input -> dright > 0) {
        ++piece.offset_col;
    }
    if (input -> dup > 0) {
        piece.rotation = (piece.rotation + 1) % 4;
    }
    if (check_piece_valid(&piece, game -> board, WIDTH, HEIGHT)) {
        game->piece = piece;
    }
}

void update_game(game_state *game, const input_state *input) {
    switch (game->phase) {
        case GAME_PHASE_PLAY:
            return update_game_play(game, input);
            break;
    }
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return 1;
    }
    SDL_Window *window = SDL_CreateWindow("Tetris",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            400, 720,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    return 0;
}
