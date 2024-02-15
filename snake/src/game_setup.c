#include "game_setup.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "game.h"
#include "common.h"

// Some handy dandy macros for decompression
#define E_CAP_HEX 0x45
#define E_LOW_HEX 0x65
#define G_CAP_HEX 0x47
#define G_LOW_HEX 0x67
#define S_CAP_HEX 0x53
#define S_LOW_HEX 0x73
#define W_CAP_HEX 0x57
#define W_LOW_HEX 0x77
#define DIGIT_START 0x30
#define DIGIT_END 0x39

/** Initializes the board with walls around the edge of the board.
 *
 * Modifies values pointed to by cells_p, width_p, and height_p and initializes
 * cells array to reflect this default board.
 *
 * Returns INIT_SUCCESS to indicate that it was successful.
 *
 * Arguments:
 *  - cells_p: a pointer to a memory location where a pointer to the first
 *             element in a newly initialized array of cells should be stored.
 *  - width_p: a pointer to a memory location where the newly initialized
 *             width should be stored.
 *  - height_p: a pointer to a memory location where the newly initialized
 *              height should be stored.
 */
enum board_init_status initialize_default_board(int** cells_p, size_t* width_p,
                                                size_t* height_p) {
    *width_p = 20;
    *height_p = 10;
    int* cells = malloc(20 * 10 * sizeof(int));
    *cells_p = cells;
    for (int i = 0; i < 20 * 10; i++) {
        cells[i] = PLAIN_CELL;
    }

    // Set edge cells!
    // Top and bottom edges:
    for (int i = 0; i < 20; ++i) {
        cells[i] = FLAG_WALL;
        cells[i + (20 * (10 - 1))] = FLAG_WALL;
    }
    // Left and right edges:
    for (int i = 0; i < 10; ++i) {
        cells[i * 20] = FLAG_WALL;
        cells[i * 20 + 20 - 1] = FLAG_WALL;
    }

    // Set grass cells!
    // Top and bottom edges:
    for (int i = 1; i < 19; ++i) {
        cells[i + 20] = FLAG_GRASS;
        cells[i + (20 * (9 - 1))] = FLAG_GRASS;
    }
    // Left and right edges:
    for (int i = 1; i < 9; ++i) {
        cells[i * 20 + 1] = FLAG_GRASS;
        cells[i * 20 + 19 - 1] = FLAG_GRASS;
    }

    // Add snake
    cells[20 * 2 + 2] = FLAG_SNAKE;

    return INIT_SUCCESS;
}

/** Initialize variables relevant to the game board.
 * Arguments:
 *  - cells_p: a pointer to a memory location where a pointer to the first
 *             element in a newly initialized array of cells should be stored.
 *  - width_p: a pointer to a memory location where the newly initialized
 *             width should be stored.
 *  - height_p: a pointer to a memory location where the newly initialized
 *              height should be stored.
 *  - snake_p: a pointer to your snake struct (not used until part 3!)
 *  - board_rep: a string representing the initial board. May be NULL for
 * default board.
 */
enum board_init_status initialize_game(int** cells_p, size_t* width_p,
                                       size_t* height_p, snake_t* snake_p,
                                       char* board_rep) {
    enum board_init_status status;
    snake_p->snake_position = NULL;
    g_game_over = 0;
    g_score = 0;
    snake_p->direction = INPUT_NONE;

    if (board_rep == NULL) {
        initialize_default_board(cells_p, width_p, height_p);
        int snake_pos = 20 * 2 + 2;
        insert_first(&(snake_p->snake_position), &snake_pos, sizeof(int));
        status = INIT_SUCCESS;
    } else {
        status = decompress_board_str(cells_p, width_p, height_p, snake_p, board_rep);
    }

    place_food(*cells_p, *width_p, *height_p);

    return status;
}

size_t get_index(int row, int col, size_t width) {
    return row * width + col;
}

int parse_int(char **str) {
    int result = 0;

    while (**str >= '0' && **str <= '9') {
        result = result * 10 + (**str - '0');
        (*str)++;
    }

    return result;
}

/** Takes in a string `compressed` and initializes values pointed to by
 * cells_p, width_p, and height_p accordingly. Arguments:
 *      - cells_p: a pointer to the pointer representing the cells array
 *                 that we would like to initialize.
 *      - width_p: a pointer to the width variable we'd like to initialize.
 *      - height_p: a pointer to the height variable we'd like to initialize.
 *      - snake_p: a pointer to your snake struct (not used until part 3!)
 *      - compressed: a string that contains the representation of the board.
 * Note: We assume that the string will be of the following form:
 * B24x80|E5W2E73|E5W2S1E72... To read it, we scan the string row-by-row
 * (delineated by the `|` character), and read out a letter (E, S or W) a number
 * of times dictated by the number that follows the letter.
 */
enum board_init_status decompress_board_str(int** cells_p, size_t* width_p,
                                            size_t* height_p, snake_t* snake_p,
                                            char* compressed) {
    char* current = compressed + 1;

    int given_rows = parse_int(&current);
    current++; 
    int given_cols = parse_int(&current);
    current++; 

    int* cells = malloc(given_cols * given_rows * sizeof(int));
    *cells_p = cells;

    *width_p = given_cols;
    *height_p = given_rows;

    int snake_count = 0;

    for (int row = 0; row < given_rows; row++) {
        if (!current || *current == '\0') {
            return INIT_ERR_INCORRECT_DIMENSIONS;
        }

        int col = 0; 

        while (*current && *current != '|') {
            if (col >= given_cols) {
                return INIT_ERR_INCORRECT_DIMENSIONS;
            }
            char cellType = *current++;

            int count = parse_int(&current);
            if (count > given_cols - col) {
                return INIT_ERR_INCORRECT_DIMENSIONS;
            }

            if (cellType != 'E' && cellType != 'W' && cellType != 'S' && cellType != 'G' && cellType != 'O') {
                return INIT_ERR_BAD_CHAR;
            }
            if (cellType == 'S') snake_count += count;

            for (int i = 0; i < count; i++) {
                int cell_index = get_index(row, col, *width_p);
                if (cellType == 'W') {
                    (*cells_p)[cell_index] = FLAG_WALL;
                } else if (cellType == 'S') {
                    (*cells_p)[cell_index] = FLAG_SNAKE;
                    int snake_pos = row * (*width_p) + col;
                    insert_first(&(snake_p->snake_position), &snake_pos, sizeof(int));
                } else if (cellType == 'G') {
                    (*cells_p)[cell_index] = FLAG_GRASS;
                } else if (cellType == 'O') {
                    (*cells_p)[cell_index] = FLAG_FOOD;
                } else {
                    (*cells_p)[cell_index] = PLAIN_CELL;
                }
                col++;
            }
        }
        if (col != given_cols) {
            return INIT_ERR_INCORRECT_DIMENSIONS;
        }
        if (*current == '|') current++; 
    }

    if (*current != '\0') {
        return INIT_ERR_INCORRECT_DIMENSIONS;
    }

    if (snake_count != 1) {
        return INIT_ERR_WRONG_SNAKE_NUM;
    }
    return INIT_SUCCESS;
}




    