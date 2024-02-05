#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>

#include "linked_list.h"
#include "mbstrings.h"
#include "common.h"


void handle_position(int prev_head, int new_head) {
    
}

/** Updates the game by a single step, and modifies the game information
 * accordingly. Arguments:
 *  - cells: a pointer to the first integer in an array of integers representing
 *    each board cell.
 *  - width: width of the board.
 *  - height: height of the board.
 *  - snake_p: pointer to your snake struct (not used until part 3!)
 *  - input: the next input.
 *  - growing: 0 if the snake does not grow on eating, 1 if it does.
 */
void update(int* cells, size_t width, size_t height, snake_t* snake_p,
            enum input_key input, int growing) {
    // `update` should update the board, your snake's data, and global
    // variables representing game information to reflect new state. If in the
    // updated position, the snake runs into a wall or itself, it will not move
    // and global variable g_game_over will be 1. Otherwise, it will be moved
    // to the new position. If the snake eats food, the game score (`g_score`)
    // increases by 1. This function assumes that the board is surrounded by
    // walls, so it does not handle the case where a snake runs off the board.

    // TODO: implement!
    int new_head;
    
    if (input == INPUT_UP) {
        g_snake_direction = INPUT_UP;
    } else if (input == INPUT_DOWN) {
        g_snake_direction = INPUT_DOWN;
    } else if (input == INPUT_LEFT) {
        g_snake_direction = INPUT_LEFT;
    } else if (input == INPUT_RIGHT){
        g_snake_direction = INPUT_RIGHT;
    } 

    if (g_snake_direction == INPUT_UP) {
        new_head = (g_snake_head_row - 1) * width + g_snake_head_col;
        if (cells[new_head] == FLAG_WALL) {
            g_game_over = 1;
        }
        else if (cells[new_head] & FLAG_FOOD) {
            g_snake_head_row--;
            int prev_head = (g_snake_head_row + 1) * width + g_snake_head_col;

            if (!(cells[prev_head] & FLAG_GRASS)) {
                cells[prev_head] = PLAIN_CELL;
            } else {
                cells[prev_head] = FLAG_GRASS;
            }
            if(cells[new_head] & FLAG_GRASS) {
                cells[new_head] = FLAG_SNAKE | FLAG_GRASS;
            } else {
                cells[new_head] = FLAG_SNAKE;
            }

            g_score++;
            place_food(cells, width, height);

        }
        else {
            g_snake_head_row--;
            int prev_head = (g_snake_head_row + 1) * width + g_snake_head_col;
            if (!(cells[prev_head] & FLAG_GRASS)) {
                cells[prev_head] = PLAIN_CELL;
            } else {
                cells[prev_head] = FLAG_GRASS;
            }
            if(cells[new_head] & FLAG_GRASS) {
                cells[new_head] = FLAG_SNAKE | FLAG_GRASS;
            } else {
                cells[new_head] = FLAG_SNAKE;
            }
        } 
    }
    if (g_snake_direction == INPUT_DOWN) {
        new_head = (g_snake_head_row + 1) * width + g_snake_head_col;
        if (cells[new_head] == FLAG_WALL) {
            g_game_over = 1;
        }
        else if (cells[new_head] & FLAG_FOOD) {
            g_snake_head_row++;
            int prev_head = (g_snake_head_row - 1) * width + g_snake_head_col;

            if (!(cells[prev_head] & FLAG_GRASS)) {
                cells[prev_head] = PLAIN_CELL;
            } else {
                cells[prev_head] = FLAG_GRASS;
            }
            if(cells[new_head] & FLAG_GRASS) {
                cells[new_head] = FLAG_SNAKE | FLAG_GRASS;
            } else {
                cells[new_head] = FLAG_SNAKE;
            }

            g_score++;
            place_food(cells, width, height);

        }
        else {
            g_snake_head_row++;
            int prev_head = (g_snake_head_row - 1) * width + g_snake_head_col;
            if (!(cells[prev_head] & FLAG_GRASS)) {
                cells[prev_head] = PLAIN_CELL;
            } else {
                cells[prev_head] = FLAG_GRASS;
            }
            if(cells[new_head] & FLAG_GRASS) {
                cells[new_head] = FLAG_SNAKE | FLAG_GRASS;
            } else {
                cells[new_head] = FLAG_SNAKE;
            }
        } 
    }
    if (g_snake_direction == INPUT_LEFT) {
        new_head = g_snake_head_row * width + (g_snake_head_col - 1);
        if (cells[new_head] == FLAG_WALL) {
            g_game_over = 1;
        }
        else if (cells[new_head] & FLAG_FOOD) {
            g_snake_head_col--;
            int prev_head = g_snake_head_row * width + (g_snake_head_col + 1);

            if (!(cells[prev_head] & FLAG_GRASS)) {
                cells[prev_head] = PLAIN_CELL;
            } else {
                cells[prev_head] = FLAG_GRASS;
            }
            if(cells[new_head] & FLAG_GRASS) {
                cells[new_head] = FLAG_SNAKE | FLAG_GRASS;
            } else {
                cells[new_head] = FLAG_SNAKE;
            }

            g_score++;
            place_food(cells, width, height);

        }
        else {
            g_snake_head_col--;
            int prev_head = g_snake_head_row * width + (g_snake_head_col + 1);
            if (!(cells[prev_head] & FLAG_GRASS)) {
                cells[prev_head] = PLAIN_CELL;
            } else {
                cells[prev_head] = FLAG_GRASS;
            }
            if(cells[new_head] & FLAG_GRASS) {
                cells[new_head] = FLAG_SNAKE | FLAG_GRASS;
            } else {
                cells[new_head] = FLAG_SNAKE;
            }
        } 
    }
    if (g_snake_direction == INPUT_RIGHT) {
        new_head = g_snake_head_row * width + (g_snake_head_col + 1);
        if (cells[new_head] == FLAG_WALL) {
            g_game_over = 1;
        }
        else if (cells[new_head] & FLAG_FOOD) {
            g_snake_head_col++;
            int prev_head = g_snake_head_row * width + (g_snake_head_col - 1);

            if (!(cells[prev_head] & FLAG_GRASS)) {
                cells[prev_head] = PLAIN_CELL;
            } else {
                cells[prev_head] = FLAG_GRASS;
            }
            if(cells[new_head] & FLAG_GRASS) {
                cells[new_head] = FLAG_SNAKE | FLAG_GRASS;
            } else {
                cells[new_head] = FLAG_SNAKE;
            }

            g_score++;
            place_food(cells, width, height);
        }
        else {
            g_snake_head_col++;
            int prev_head = g_snake_head_row * width + (g_snake_head_col - 1);
            if (!(cells[prev_head] & FLAG_GRASS)) {
                cells[prev_head] = PLAIN_CELL;
            } else {
                cells[prev_head] = FLAG_GRASS;
            }
            if(cells[new_head] & FLAG_GRASS) {
                cells[new_head] = FLAG_SNAKE | FLAG_GRASS;
            } else {
                cells[new_head] = FLAG_SNAKE;
            }
        } 
    }
}

    


/** Sets a random space on the given board to food.
 * Arguments:
 *  - cells: a pointer to the first integer in an array of integers representing
 *    each board cell.
 *  - width: the width of the board
 *  - height: the height of the board
 */
void place_food(int* cells, size_t width, size_t height) {
    /* DO NOT MODIFY THIS FUNCTION */
    unsigned food_index = generate_index(width * height);
    // check that the cell is empty or only contains grass
    if ((*(cells + food_index) == PLAIN_CELL) || (*(cells + food_index) == FLAG_GRASS)) {
        *(cells + food_index) |= FLAG_FOOD;
    } else {
        place_food(cells, width, height);
    }
    /* DO NOT MODIFY THIS FUNCTION */
}

/** Prompts the user for their name and saves it in the given buffer.
 * Arguments:
 *  - `write_into`: a pointer to the buffer to be written into.
 */
void read_name(char* write_into) {
    // TODO: implement! (remove the call to strcpy once you begin your
    // implementation)
    strcpy(write_into, "placeholder");
}

/** Cleans up on game over — should free any allocated memory so that the
 * LeakSanitizer doesn't complain.
 * Arguments:
 *  - cells: a pointer to the first integer in an array of integers representing
 *    each board cell.
 *  - snake_p: a pointer to your snake struct. (not needed until part 3)
 */
void teardown(int* cells, snake_t* snake_p) {
    // TODO: implement!
}
