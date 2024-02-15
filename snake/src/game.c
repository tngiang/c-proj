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


    if (g_game_over) {
        return;
    }

    int cur_loc = *(int*)get_first(snake_p->snake_position);
    int new_loc = cur_loc;

    if (input == INPUT_NONE) {
        input = snake_p->direction;
    }
    if (input == INPUT_UP) {
        if (snake_p->direction == INPUT_DOWN) {
            new_loc = new_loc + width;
        } else {
            new_loc = new_loc - width;
            snake_p->direction = INPUT_UP;
        }
    } else if (input == INPUT_DOWN) {
        if (snake_p->direction == INPUT_UP) {
            new_loc = new_loc - width;
        } else {
            new_loc = new_loc + width;
            snake_p->direction = INPUT_DOWN;
        } 
    } else if (input == INPUT_LEFT){
        if (snake_p->direction == INPUT_RIGHT) {
            new_loc = new_loc + 1;
        } else {
            new_loc = new_loc - 1;
            snake_p->direction = INPUT_LEFT;
        }
    } else if (input == INPUT_RIGHT) {
        if (snake_p->direction == INPUT_LEFT) {
            new_loc = new_loc - 1;
        } else {
            new_loc = new_loc + 1;
            snake_p->direction = INPUT_RIGHT;
        }
    } else if (input == INPUT_NONE) {
        new_loc = new_loc + 1;
        snake_p->direction = INPUT_NONE;
    }
    if(cells[new_loc] == FLAG_WALL) {
        g_game_over = 1;
        return;
    }
    else if (cells[new_loc] & FLAG_FOOD) {
        if (!growing) {
            void* removed = remove_last(&(snake_p->snake_position));
            int removed_loc = *(int*) removed;
            if (cells[removed_loc] & FLAG_GRASS) {
                cells[removed_loc] = FLAG_GRASS;
            } else {
                cells[removed_loc] = PLAIN_CELL;
            }
            free(removed);
        }
        insert_first(&(snake_p->snake_position), &new_loc, sizeof(int));
        if (cells[new_loc] & FLAG_GRASS) {
            cells[new_loc] = FLAG_GRASS | FLAG_SNAKE;
        } else {
            cells[new_loc] = FLAG_SNAKE;
        }
        g_score++;
        place_food(cells, width, height);
    } else {
        if (length_list(snake_p->snake_position) < 2) {
            if (cells[new_loc] == FLAG_GRASS) {
                if (!(cells[cur_loc] & FLAG_GRASS)) {
                    cells[cur_loc] = PLAIN_CELL;
                } else {
                    cells[cur_loc] = FLAG_GRASS;
                }
                cells[new_loc] = FLAG_GRASS | FLAG_SNAKE;
            } else {
                if (!(cells[cur_loc] & FLAG_GRASS)) {
                    cells[cur_loc] = PLAIN_CELL;
                } else {
                    cells[cur_loc] = FLAG_GRASS;
                }
                cells[new_loc] = FLAG_SNAKE;
            }  
            free(remove_last(&(snake_p->snake_position)));
            insert_first(&(snake_p->snake_position), &new_loc, sizeof(int));
        } else {
            void* last = remove_last(&snake_p->snake_position);
            int last_loc = *(int*)last;
            //cells[last_loc] = PLAIN_CELL;
            if (cells[last_loc] & FLAG_GRASS) {
                cells[last_loc] = FLAG_GRASS;
            } else {
                cells[last_loc] = PLAIN_CELL;
            }
            if (cells[new_loc] & FLAG_SNAKE) {
                if (cells[last_loc] & FLAG_GRASS) {
                    cells[last_loc] = FLAG_SNAKE | FLAG_GRASS;
                } else {
                    cells[last_loc] = FLAG_SNAKE;
                }
                free(last);
                g_game_over = 1;
                return;
            }
            free(last);
            if (cells[new_loc] & FLAG_GRASS) {
                cells[new_loc] = FLAG_SNAKE | FLAG_GRASS;
            } else {
                cells[new_loc] = FLAG_SNAKE;
            }
            insert_first(&(snake_p->snake_position), &new_loc, sizeof(int));
        }
    }
    return;
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
    printf("Name > ");
    fflush(0);
    read(0, write_into, 1000);
    while (*write_into == '\n') {
        printf("Name Invalid: must be longer than 0 characters.\n");
        printf("Name > ");
        fflush(0);
        write_into = "\0";
        //read(0, write_into, 1000);
    }
    for (int i = 0; i < (int)strlen(write_into); i++) {
        if (write_into[i] == '\n') {
            write_into[i] = '\0';
        }
    }
    g_name = write_into;
    g_name_len = mbslen(write_into);
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
    while(snake_p->snake_position != NULL) {
        void* removed = remove_last(&(snake_p->snake_position));
        free(removed);
    }
}
