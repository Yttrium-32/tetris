#include <assert.h>
#include <stdbool.h>
#include <time.h>

#include <SDL2/SDL.h>

#include "color.h"

#define WIDTH 10
#define HEIGHT 22
#define VISIBLE_HEIGHT 20
#define GRID_SIZE 30

#define min(a, b) a < b ? a : b
#define max(a, b) a > b ? a : b

const uint8_t FRAMES_PER_DROP[] = {
    48,
    43,
    38,
    33,
    28,
    23,
    18,
    13,
    8,
    6,
    5, 5, 5,
    4, 4, 4,
    3, 3, 3,
    2, 2, 2,
    2, 2, 2,
    2, 2, 2,
    1
};

const float_t TARGET_SECONDS_PER_FRAME = 1.f / 60.f;

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

const uint8_t TETROMINO_4[] = {
    0, 4, 4,
    4, 4, 0,
    0, 0, 0
};

const uint8_t TETROMINO_5[] = {
    5, 5, 0,
    0, 5, 5,
    0, 0, 0
};

const uint8_t TETROMINO_6[] = {
    6, 0, 0,
    6, 6, 6,
    0, 0, 0
};

const uint8_t TETROMINO_7[] = {
    0, 0, 7,
    7, 7, 7,
    0, 0, 0
};

const Tetromino TETROMINOS[] = {
    {TETROMINO_1, 4},
    {TETROMINO_2, 2},
    {TETROMINO_3, 3},
    {TETROMINO_4, 3},
    {TETROMINO_5, 3},
    {TETROMINO_6, 3},
    {TETROMINO_7, 3},
};

typedef enum {
    GAME_PHASE_PLAY,
    GAME_PHASE_LINE,
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
    uint8_t lines[HEIGHT];
    PieceState piece;
    GamePhase phase;

    int32_t level;
    int32_t start_level;
    int32_t pending_line_count;
    int32_t line_count;
    int32_t points;

    float_t highlight_end_time;
    float_t next_drop_time;
    float_t time;
} GameState ;

typedef struct {
    uint8_t left;
    uint8_t right;
    uint8_t up;
    uint8_t down;
    uint8_t a;

    int8_t dleft;
    int8_t dright;
    int8_t dup;
    int8_t ddown;
    int8_t da;
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

uint8_t check_row_filled(const uint8_t *values, int32_t width, int32_t row) {
    for (int32_t col = 0; col < width; ++col) {
        if (!matrix_get(values, width, row, col)) {
            return 0;
        }
    }
    return 1;
}

int32_t find_lines(const uint8_t *values, int32_t width, int32_t height, uint8_t *lines_out) {
    int32_t count = 0;
    for (int32_t row = 0; row < height; row++) {
        uint8_t filled = check_row_filled(values, width, row);
        lines_out[row] = filled;
        count += filled;
    }
    return count;
}

void clear_lines(uint8_t *values, int32_t width, int32_t height, const uint8_t *lines) {
    int32_t src_row = height - 1;
    for (int32_t dst_row = height - 1; dst_row >= 0; --dst_row) {
        while (src_row > 0 && lines[src_row]) {
            --src_row;
        }
        if (src_row < 0) {
            memset(values + dst_row * width, 0, width);
        } else {
            memcpy(values + dst_row * width,
                   values + src_row * width,
                    width);
            --src_row;
        }
    }
}

bool check_piece_valid(const PieceState *piece, const uint8_t *board, int32_t width, int32_t height) {
    const Tetromino *tetromino = &TETROMINOS[piece->tetromino_index];
    assert(tetromino);

    for (int32_t row = 0; row < tetromino->side; ++row) {
        for (int32_t col = 0; col < tetromino->side; ++col) {
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
                if (matrix_get(board, width, board_row, board_col)) {
                    return false;
                }
            }
        }
    }
    return true;
}

void merge_piece(GameState *game) {
    const Tetromino *tetromino = TETROMINOS + game->piece.tetromino_index;

    for (int32_t row = 0; row < tetromino->side; row++) {
        for (int32_t col = 0; col < tetromino->side; col++) {
            uint8_t value = tetromino_get(tetromino, row, col, game->piece.rotation);
            if (value) {
                int32_t board_row = game->piece.offset_row + row;
                int32_t board_col = game->piece.offset_col + col;
                matrix_set(game->board, WIDTH, board_row, board_col, value);
            }
        }
    }
}

float_t get_time_to_next_drop(int32_t level) {
    if (level > 29)
        level = 29;
    return FRAMES_PER_DROP[level] * TARGET_SECONDS_PER_FRAME;
}

uint8_t get_random_tetromino() {
    srand((unsigned) time(NULL));
    return rand() % 7;
}

void spawn_piece(GameState *game) {
    memset(&game->piece, 0, sizeof(PieceState));
    game->piece.tetromino_index = get_random_tetromino();
    game->piece.offset_col = WIDTH / 2;
    game->next_drop_time = game->time + get_time_to_next_drop(game->level);
}

bool soft_drop(GameState *game) {
    ++game->piece.offset_row;
    bool is_valid = check_piece_valid(&game->piece, game->board, WIDTH, HEIGHT);
    if (!is_valid) {
        --game -> piece.offset_row;
        merge_piece(game);
        spawn_piece(game);
        return false;
    }
    game->next_drop_time = game->time + get_time_to_next_drop(game->level);
    return true;
}

int32_t compute_points(int32_t level, int32_t line_count) {
    switch (line_count) {
        case 1:
            return 40 * (level + 1);
        case 2:
            return 100 * (level + 1);
        case 3:
            return 300 * (level + 1);
        case 4:
            return 1200 * (level + 1);
    }
    return 0;
}

int32_t get_lines_for_next_level(int32_t start_level, int32_t level) {
    int32_t first_level_up_limit = min(
            (start_level * 10 + 10),
            max(100, (start_level * 10 - 50))
    );
    if (level == start_level) {
        return first_level_up_limit;
    }

    int32_t diff = level - start_level;
    return first_level_up_limit + diff * 10;
}

void update_game_line(GameState *game) {
    if (game->time >= game->highlight_end_time) {
        clear_lines(game->board, WIDTH, HEIGHT, game->lines);
        game->line_count += game->pending_line_count;
        game->points += compute_points(game->level, game->pending_line_count);

        int32_t lines_for_next_level = get_lines_for_next_level(
                game->start_level,
                game->level
        );

        if (game->line_count >= lines_for_next_level) {
            ++game->level;
        }

        game->phase = GAME_PHASE_PLAY;
    }
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

    bool is_valid = check_piece_valid(&piece, game -> board, WIDTH, HEIGHT);
    if (is_valid) {
        game->piece = piece;
    }

    if (input->ddown > 0) {
        soft_drop(game);
    }

    if (input->da > 0)
        while(soft_drop(game));

    while (game->time >= game->next_drop_time) {
        soft_drop(game);
    }

    game->pending_line_count = find_lines(game->board, WIDTH, HEIGHT, game->lines);
    if (game->pending_line_count > 0) {
        game->phase = GAME_PHASE_LINE;
        game->highlight_end_time = game-> time + 0.5f;
    }
}

void update_game(GameState *game, const InputState *input) {
    switch (game->phase) {
        case GAME_PHASE_PLAY:
            return update_game_play(game, input);
            break;
        case GAME_PHASE_LINE:
            update_game_line(game);
            break;
    }
}

void fill_rect(SDL_Renderer* renderer, int32_t x, int32_t y, int32_t width, int32_t height, Color color) {
    SDL_Rect rect = {0};
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

void draw_board(SDL_Renderer *renderer, const uint8_t *board, int32_t width, int32_t height, int32_t offset_x, int32_t offset_y) {
    for (int32_t row = 0; row < height; ++row) {
        for (int32_t col = 0; col < width; ++col) {
            uint8_t value = matrix_get(board, width, row, col);
            draw_cell(renderer, row, col, value, offset_x, offset_y);
        }
    }
}

void render_game(const GameState *game, SDL_Renderer *renderer) {
    Color highlight_color = color(0xFF, 0xFF, 0xFF, 0xFF);
    draw_board(renderer, game -> board, WIDTH, HEIGHT, 0, 0);
    draw_piece(renderer, &game -> piece, 0, 0);

    if (game->phase == GAME_PHASE_LINE) {
        for (int32_t row = 0; row < HEIGHT; ++row) {
            if (game->lines[row]) {
                int32_t x = 0;
                int32_t y = row * GRID_SIZE;
                fill_rect(renderer, x, y, WIDTH * GRID_SIZE, GRID_SIZE, highlight_color);
            }
        }
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

    GameState game = {};
    InputState input = {};

    spawn_piece(&game);
    game.piece.tetromino_index = 2;

    bool quit = false;
    while (!quit) {
        game.time = SDL_GetTicks() / 1000.0f;

        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                    quit = true;
            }
        }

        int32_t key_count;
        const uint8_t *key_states = SDL_GetKeyboardState(&key_count);

        InputState prev_input = input;

        input.left = key_states[SDL_SCANCODE_LEFT];
        input.right = key_states[SDL_SCANCODE_RIGHT];
        input.up = key_states[SDL_SCANCODE_UP];
        input.down = key_states[SDL_SCANCODE_DOWN];
        input.a = key_states[SDL_SCANCODE_SPACE];

        input.dleft = input.left - prev_input.left;
        input.dright = input.right - prev_input.right;
        input.dup = input.up - prev_input.up;
        input.ddown = input.down - prev_input.down;
        input.da = input.a - prev_input.a;

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
