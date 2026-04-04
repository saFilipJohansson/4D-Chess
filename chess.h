#ifndef CHESS_H
#define CHESS_H

#include <stdbool.h>
#include "chess_types.h"

// chess_init.c
bool initialize_rules_and_game_state(struct Rules *rules, struct GameState *GameState, enum Variant variant);
void terminate_game_state(struct GameState *game_state);

// chess_logic.c
void get_moves (
    struct Move moves[MAX_MOVES_SINGLE_PIECE], struct Move diagonal_pawn_moves[MAX_MOVES_SINGLE_PIECE], int square_index, 
    struct GameState *game_state, struct Rules *rules
);
struct Move validate_selected_move(
    int origin_square, int destination_square, struct Move possible_moves[], struct GameState *game_state, struct Rules *rules
);
bool evaluate_promotion(int square_index_from, int square_index_moving_to, struct GameState *game_state, struct Rules *rules);
void make_move(struct Move move, enum PieceType promotion_piece_type, struct GameState *game_state, struct Rules *rules);
bool evaluate_win_conditions(struct Move last_move, struct GameState *game_state, struct Rules *rules);

// chess_utils.c
int  square_to_square_index(int square[], int dimensions, int board_shape[]);
void square_index_to_square(int square_index, int square[], int dimensions, int board_shape[]);
bool check_if_int_in_array(int integer, int int_array[]);
bool check_if_move_among_moves(struct Move move, struct Move moves[]);
void copy_int_array(int *from, int *to, int length);

#endif // CHESS_H

