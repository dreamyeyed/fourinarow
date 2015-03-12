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
#include <float.h>
#include <math.h>
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
 * The state of the game. Technically could be called game_state because
 * structs and enums are in different namespaces, but that could cause
 * confusion.
 */
enum game_status
{
    GAME_IN_PROGRESS,
    GAME_P1_VICTORY,
    GAME_P2_VICTORY,
    GAME_DRAW
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
    enum game_status status;
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
 * Returns 1 if the given coordinates are within the board; 0 otherwise.
 * This could be implemented as a macro (or in C99 as an inline function) for
 * a little speed boost, but the compiler can probably optimize it anyway.
 */
static int valid_coords(int row, int col)
{
    return 0 <= row && row < BOARD_H
        && 0 <= col && col < BOARD_W;
}

/*
 * Updates the game status.
 */
void state_update_status(struct game_state *state)
{
    int row, col;
    int draw;

    /* If the game is already over, there's nothing to do. */
    if (state->status != GAME_IN_PROGRESS) {
        return;
    }

    /* The game is a draw if every square on the board is full. It's enough to
     * check the top square of each column. */
    draw = 1;
    for (col = 0; col < BOARD_W; ++col) {
        if (state->board[BOARD_H-1][col] == NO_PIECE) {
            draw = 0;
            break;
        }
    }
    if (draw) {
        state->status = GAME_DRAW;
        return;
    }

    /*
     * Checking for victory is more complicated. Basically, we loop through
     * every square on the board and then through eight possible directions. If
     * the first three squares in a direction are on the board and contain the
     * same piece as the starting square, then we found a winner.
     *
     * TODO: Perhaps split this mess into multiple functions.
     */
    for (row = 0; row < BOARD_H; ++row) {
        for (col = 0; col < BOARD_W; ++col) {
            int dx, dy;
            enum piece start_piece = state->board[row][col];

            /* Skip empty spaces. */
            if (!start_piece) {
                continue;
            }

            /* Direction is represented with two variables. dy means vertical
             * movement and dx is horizontal movement. For example,
             * dy = 1 and dx = 1 is up and to the right. */
            for (dx = -1; dx <= 1; ++dx) {
                for (dy = -1; dy <= 1; ++dy) {
                    int victory, i;

                    /* This is a nonsense direction. */
                    if (dx == 0 && dy == 0) {
                        continue;
                    }

                    /* Assume that the game is over until proven otherwise. */
                    victory = 1;
                    for (i = 1; i < 4; ++i) {
                        if (!valid_coords(row+dy*i, col+dx*i)
                         || state->board[row+dy*i][col+dx*i] != start_piece) {
                            victory = 0;
                            break;
                        }
                    }

                    if (victory) {
                        /* We found a winner! */
                        state->status = start_piece == P1_PIECE
                                ? GAME_P1_VICTORY
                                : GAME_P2_VICTORY;
                        return;
                    }
                }
            }
        }
    }
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

    /* Update the game status. */
    state_update_status(new_state);

    return new_state;
}

/*
 * Evaluates the game state for player 1. Returns a number in range [-1, 1],
 * where +1 = P1 victory, -1 = P2 victory and 0 = draw.
 *
 * To evaluate the state for player 2, simply use -state_evaluate(state).
 */
double state_evaluate(const struct game_state *state)
{
    /* These variables will be used later. Unfortunately C89 forces us to
     * declare them here. */
    int horizontal_middle = (BOARD_W-1)/2;
    int vertical_middle = (BOARD_H-1)/2;
    double max_piece_value = 1.0 / (BOARD_W*BOARD_H/2);
    double move_value = 0;
    int row, col;

    switch (state->status) {
        case GAME_P1_VICTORY:
        return 1;

        case GAME_P2_VICTORY:
        return -1;

        case GAME_DRAW:
        return 0;

        case GAME_IN_PROGRESS:
        /* continue evaluation */
        break;

        default:
        assert("state_evaluate: invalid game state" && 0);
    }

    /* We'll use a very simple heuristic for testing: the closer a piece is
     * to the center of the board, the better.  We can always improve it
     * later. */
    for (row = 0; row < BOARD_H; ++row) {
        for (col = 0; col < BOARD_W; ++col) {
            if (state->board[row][col] != NO_PIECE) {
                /* Calculate distance from center. */
                int dy = abs(row - vertical_middle);
                int dx = abs(col - horizontal_middle);
                /* add 1 to ensure dist is never 0 */
                double dist = sqrt(dy*dy + dx*dx) + 1;

                if (state->board[row][col] == P1_PIECE) {
                    move_value += max_piece_value / dist;
                } else {
                    move_value -= max_piece_value / dist;
                }
            }
        }
    }

    assert(-1.0 <= move_value && move_value <= 1.0);

    return move_value;
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

/* typedef for a function that can evaluate a game state. typedefs are very
 * useful because function pointers can be difficult to read. */
typedef double (*eval_fn)(const struct game_state *);

/*
 * Chooses the best move for P2 using the minimax algorithm. Don't call this
 * function directly.
 */
static int _minimax(const struct game_state *state, eval_fn eval,
                    double *score, int max_depth, int curr_depth)
{
    int best_move;
    double best_score;
    int col;

    /* Maximum depth reached? Just evaluate the state with the evaluation
     * function. */
    if (curr_depth >= max_depth) {
        *score = eval(state);
        return -1;
    }

    /* Game over? */
    switch (state->status) {
        case GAME_P1_VICTORY:
        *score = 1;
        return -1;

        case GAME_P2_VICTORY:
        *score = -1;
        return -1;

        case GAME_DRAW:
        *score = 0;
        return -1;

        case GAME_IN_PROGRESS:
        break;

        default:
        assert("state_evaluate: invalid game state" && 0);
    }

    /* Game is still is progress */
    best_move = -1;
    /* Remember that low score is good for P2. */
    best_score = state->current_player == PLAYER1 ? DBL_MIN : DBL_MAX;

    for (col = 0; col < BOARD_W; ++col) {
        int current_move;
        double current_score;

        struct game_state *new_state = state_move(state, col);
        if (!new_state) {
            /* illegal move */
            continue;
        }

        /* Check if this move is better than the best one found so far */
        current_move = _minimax(new_state, eval, &current_score,
                                max_depth, curr_depth+1);
        if (state->current_player == PLAYER1) {
            if (current_score > best_score) {
                best_move = col;
                best_score = current_score;
            }
        } else if (current_score < best_score) {
            best_move = col;
            best_score = current_score;
        }

        free(new_state);
    }

    *score = best_score;
    return best_move;
}

/*
 * Chooses the best move for P2. max_depth is the number of moves in the future
 * that the algorithm considers.
 */
int minimax(const struct game_state *state, eval_fn eval, int max_depth)
{
    double score;
    /* We don't care about the score, just return the move. */
    return _minimax(state, eval, &score, max_depth, 0);
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

        /* Check if the game is already over */
        switch (state->status) {
            case GAME_IN_PROGRESS:
            /* Continue the game. */
            break;

            case GAME_P1_VICTORY:
            printf("Player 1 won! Congratulations!\n");
            goto game_over;

            case GAME_P2_VICTORY:
            printf("Player 2 won! Congratulations!\n");
            goto game_over;

            case GAME_DRAW:
            printf("It's a draw!\n");
            goto game_over;

            default:
            assert("Invalid game state!" && 0);
            break;
        }

        /* Allow either the player or the AI to make a move */
        if (state->current_player == PLAYER1) {
            column = read_column();
        } else {
            column = minimax(state, state_evaluate, 6);
        }

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

game_over:
    state_delete(state);

    return 0;
}
