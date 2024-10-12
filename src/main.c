#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

#include "color.h"

#define WIDTH 10
#define HEIGHT 22
#define VISIBLE_HEIGHT 20
#define GRID_SIZE 30

#define CONSTRUCT_TETROMINO(data, side) {data, side}

/*
 * Reprsents the shape of the tetromino
 * Here, `data` is square matrix
 * each side of the matrix is of lenght `side`
 */
typedef struct {
    const uint8_t *data;
    const int32_t side;
} Tetromino;

const uint8_t TETROMINO_1[] = {
    0, 0, 0, 0,
    1, 1, 1, 1,
    0, 0, 0, 0,
    0, 0, 0, 0,
};

const uint8_t TETROMINO_2[] = {
    2, 2,
    2, 2
};
 
const uint8_t TETROMINO_3[] = {
    0, 0, 0,
    3, 3, 3,
    0, 3, 0
};

const Tetromino TETROMINOS[] = {
    CONSTRUCT_TETROMINO(TETROMINO_1, 4),
    CONSTRUCT_TETROMINO(TETROMINO_2, 2),
    CONSTRUCT_TETROMINO(TETROMINO_3, 3),
};

typedef enum {
    GAME_PHASE_PLAY
} GamePhase;

/*
 * Struct representing state of each tetromino
 * tetromino_index selects one of 8 tetrominos
 */
typedef struct {
    uint8_t tetromino_index;
    int32_t offset_row;
    int32_t offset_col;
    int32_t rotation;
} PieceState;

typedef struct {
    uint8_t board[WIDTH * HEIGHT];
    PieceState piece;
    GamePhase phase;
} GameState ;

typedef struct {
    int8_t dleft;
    int8_t dright;
    int8_t dup;

} InputState;

uint8_t matrix_get(const uint8_t *values, int32_t width, int32_t row, int32_t col) {
    int32_t index = row * width + col;
    return values[index];
}

void matrix_set(uint8_t *values, int32_t width, int32_t row, int32_t col, uint8_t value) {
    int32_t index = row * width + col;
    values[index] = value;
}

uint8_t tetromino_get(const Tetromino *tetromino, int32_t row, int32_t col, int32_t rotation) {
    int32_t side = tetromino -> side;
    switch (rotation) {
        case 0:
            return tetromino -> data[row * side + col];
        case 1:
            return tetromino -> data[(side - col - 1) * side + row];
        case 2:
            return tetromino -> data[(side - row - 1) * side + (side - col - 1)];
        case 3:
            return tetromino -> data[col * side + (side - row - 1)];
    }
    return 0;
}

bool check_piece_valid(const PieceState *piece, const uint8_t *board, int32_t width, int32_t height) {
    const Tetromino *tetromino = &TETROMINOS[piece->tetromino_index];
    assert(tetromino);

    for (int32_t row = 0; row < height; ++row) {
        for (int32_t col = 0; col < width; ++col) {
            uint8_t value = tetromino_get(tetromino, row, col, piece->rotation);
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

void update_game_play(GameState *game, const InputState *input) {
    PieceState piece = game -> piece;
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

void update_game(GameState *game, const InputState *input) {
    switch (game->phase) {
        case GAME_PHASE_PLAY:
            return update_game_play(game, input);
            break;
    }
}

void fill_rect(SDL_Renderer* renderer, int32_t x, int32_t y, int32_t width, int32_t height, Color color) {
    SDL_Rect rect = {};
    rect.x = x;
    rect.y = y;
    rect.w = width;
    rect.h = height;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

void draw_cell(SDL_Renderer *renderer, int32_t row, int32_t col, uint8_t value, int32_t offset_x, int32_t offset_y) {
    Color base_color = BASE_COLORS[value];
    Color light_color = LIGHT_COLORS[value];
    Color dark_color = DARK_COLORS[value];

    int32_t edge = GRID_SIZE / 8;

    int32_t x = col * GRID_SIZE + offset_x;
    int32_t y = row * GRID_SIZE + offset_y;

    fill_rect(renderer, x, y, GRID_SIZE, GRID_SIZE, dark_color);
    fill_rect(renderer, x + edge, y, GRID_SIZE - edge, GRID_SIZE - edge, light_color);
    fill_rect(renderer, x + edge, y + edge, GRID_SIZE - edge, GRID_SIZE - edge, base_color);
}

void draw_piece(SDL_Renderer *renderer, const PieceState *piece, int32_t offset_x, int32_t offset_y) {
    const Tetromino *tetromino = TETROMINOS + piece -> tetromino_index;
    for (int32_t row = 0; row < tetromino -> side; ++row) {
        for (int32_t col = 0; col < tetromino -> side; ++col) {
            uint8_t value = tetromino_get(tetromino, row, col, piece -> rotation);
            if (value) {
                draw_cell(renderer,
                        row + piece -> offset_row,
                        col + piece -> offset_col,
                        value,
                        offset_x,
                        offset_y);
            }
        }
    }
}

void render_game(const GameState *game, SDL_Renderer *renderer) {
    draw_piece(renderer, &game -> piece, 0, 0);
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

    GameState game = {};
    InputState input = {};

    bool quit = false;
    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                    quit = true;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        update_game(&game, &input);
        render_game(&game, renderer);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_Quit();

    return 0;
}
