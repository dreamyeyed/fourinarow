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
 * We need to know the pieces on the board, whose turn it is and if the game is
 * already over.
 */
struct game_state
{
    enum piece board[BOARD_H][BOARD_W];
    enum player current_player;
    /* enums don't have to be named. The same is true for structs as well. */
    enum {
        GAME_IN_PROGRESS,
        GAME_P1_VICTORY,
        GAME_P2_VICTORY,
        GAME_DRAW
    } status;
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
    state->status = GAME_IN_PROGRESS;

    return state;
}

/*
 * Makes a copy of a game state. Returns NULL on error.
 */
struct game_state *state_copy(const struct game_state *state)
{
    struct game_state *new_state = malloc(sizeof(*new_state));

    if (!new_state) {
        return NULL;
    }

    /* State doesn't contain anything that's dynamically allocated, so we can
     * safely use memcpy. */
    memcpy(new_state, state, sizeof(*state));

    return new_state;
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
    int row, col;

    /* This code is unfortunately quite ugly. */
    for (row = BOARD_H - 1; row >= 0; --row) {
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

/*
 * Adds the current player's piece in the given column and returns the new
 * state. The original state is left unmodified. Returns NULL if move is not
 * permitted.
 */
struct game_state *state_move(const struct game_state *state, int column)
{
    struct game_state *new_state;
    size_t row;

    /* Check that the game is not over yet */
    if (state->status != GAME_IN_PROGRESS) {
        return NULL;
    }

    /* Check that the board contains the given column */
    if (column < 0 || column >= BOARD_W) {
        return NULL;
    }

    /* Check that the column isn't full */
    if (state->board[BOARD_H-1][column]) {
        return NULL;
    }

    /* Find the lowest free space and put a new piece there */
    new_state = state_copy(state);
    for (row = 0; row < BOARD_H; ++row) {
        if (!new_state->board[row][column]) {
            new_state->board[row][column] =
                    new_state->current_player == PLAYER1 ? P1_PIECE : P2_PIECE;
            break;
        }
    }

    /* Update the current player */
    new_state->current_player =
            new_state->current_player == PLAYER1 ? PLAYER2 : PLAYER1;

    /* TODO: Check if the game is now over */

    return new_state;
}

/*
 * Reads a column number from the user. The user should input a number in range
 * [1, BOARD_W]; 1 is subtracted from that because C arrays are zero-indexed.
 *
 * Returns the column number [0, BOARD_W-1] if successful; -1 on error.
 */
int read_column(void)
{
    char buf[256];

    printf("Enter a column number [1, %d]\n", BOARD_W);
    /*
     * This is the idiomatic way to read input in C. fgets returns NULL on
     * error, so the loop runs as long as it manages to read something.
     *
     * A little note: sizeof(char) == 1 by definition, which means that
     * sizeof(buf) gives us the capacity of buf.
     */
    while (fgets(buf, sizeof(buf), stdin)) {
        int column;

        /*
         * Why fgets + sscanf instead of scanf? Because if the user inputs
         * non-numbers, scanf doesn't remove them from the input stream.
         *
         * sscanf returns the number of matched elements, which should be one.
         */
        if (sscanf(buf, "%d", &column) == 1) {
            if (column >= 1 && column <= BOARD_W) {
                return column - 1;
            } else {
                printf("Column number must be in range [1, %d]\n", BOARD_W);
            }
        } else {
            printf("Please enter a number\n");
        }
    }

    /* This line is reached only if fgets failed, generally because end of file
     * was reached. */
    return -1;
}

int main(void)
{
    struct game_state *state = state_new();

    if (!state) {
        exit(EXIT_FAILURE);
    }

    /*
     * This is the main loop of the game. It will run until the game ends.
     */
    for (;;) {
        int column;
        struct game_state *new_state;

        state_draw(state, stdout);

        /* Allow the player to make a move. */
        column = read_column();
        if (column == -1) {
            break;
        }

        /* Perform the move. */
        new_state = state_move(state, column);
        if (new_state) {
            state_delete(state);
            state = new_state;
        }
    }

    state_delete(state);

    return 0;
}
