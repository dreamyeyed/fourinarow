/*
 * fourinarow.c
 *
 * This is a basic text-based four in a row game. I made it mainly for
 * practice. Feel free to do anything what you want with it.
 *
 * Written by dreamyeyed in 2015.
 *
 * * * * *
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication along
 * with this software. If not, see
 * <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Each place on the board can contain either a piece or nothing. We'll define
 * an enum to represent the possible states.
 */
enum piece
{
    /* 
     * We'll define no piece as 0, so we can use these enum values as booleans:
     *
     * if (board[0][0]) {
     *   there is a piece
     * } else {
     *   no piece
     * }
     */
    NO_PIECE = 0,
    P1_PIECE,
    P2_PIECE
};

/*
 * Define the players.
 */
enum player
{
    PLAYER1,
    PLAYER2
};

/*
 * Some other useful constants.
 */
#define BOARD_W (7)
#define BOARD_H (6)

/*
 * This structure contains the state of the game at a specific moment in time.
 * We need to know the pieces on the board and whose turn it is.
 */
struct game_state
{
    enum piece board[BOARD_H][BOARD_W];
    enum player current_player;
};

/*
 * Allocates, initializes and returns a new game state. Returns NULL on error.
 */
struct game_state *state_new()
{
    /* I prefer using sizeof like this instead of sizeof(struct game_state),
     * because it's harder to make a mistake. */
    struct game_state *state = malloc(sizeof(*state));

    /* Don't forget to check the return value of malloc. In reality, though,
     * it *never* returns NULL on Linux unless you request more memory than
     * the computer physically contains. */
    if (!state) {
        return NULL;
    }

    /* Initialize the state with sane values. Remember that we defined NO_PIECE
     * as 0. sizeof works here because board is a static array. */
    memset(state->board, 0, sizeof(state->board));
    state->current_player = PLAYER1;

    return state;
}

/*
 * Destroys a game state. Does nothing if given a NULL pointer.
 */

void state_delete(struct game_state *state)
{
    /* free checks for NULL pointers, so we don't have to do it manually. */
    free(state);
}

/*
 * Draws a game state in the given output stream.
 */
void state_draw(const struct game_state *state, FILE *fp)
{
    size_t row, col;

    /* This code is unfortunately quite ugly. */
    for (row = 0; row < BOARD_H; ++row) {
        for (col = 0; col < BOARD_W; ++col) {
            fprintf(fp, "+-");
        }
        fprintf(fp, "+\n");
        for (col = 0; col < BOARD_W; ++col) {
            fprintf(fp, "|");
            switch (state->board[row][col]) {
                case NO_PIECE:
                putc(' ', fp);
                break;

                case P1_PIECE:
                putc('x', fp);
                break;

                case P2_PIECE:
                putc('o', fp);
                break;

                default:
                /* This assertion always fails, but produces a useful
                 * message. */
                assert("invalid piece" && 0);
            }
        }
        fprintf(fp, "|\n");
    }
    for (col = 0; col < BOARD_W; ++col) {
        fprintf(fp, "+-");
    }
    fprintf(fp, "+\n");
}

int main(void)
{
    struct game_state *state = state_new();

    if (!state) {
        exit(EXIT_FAILURE);
    }

    state_draw(state, stdout);
    state_delete(state);

    return 0;
}
