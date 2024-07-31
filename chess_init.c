#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "chess.h"

static struct Square *parse_board_from_textfile(char *text_file_name, int dimensions, int *board_shape);

bool initialize_rules_and_game_state(struct Rules *rules, struct GameState *game_state, enum Variant variant) {
    rules->dimensions = 2;
    rules->board_shape[0] = 8;
    rules->board_shape[1] = 8;
    rules->is_forward_dimension[0] = false;
    rules->is_forward_dimension[1] = true;
    rules->win_conditions[0] = CHECKMATE;
    for (int i = 1; i < MAX_NBR_OF_WIN_CONDITIONS; ++i) {
        rules->win_conditions[i] = NULL_WIN_CONDITION;   // not necessary. indexes over dimensions never accessed
    }
    rules->castling_allowed = true;
    rules->moves_per_turn_by_color[PIECE_COLOR_WHITE] = 1;
    rules->moves_per_turn_by_color[PIECE_COLOR_BLACK] = 1;
    rules->same_piece_can_move_twice = false;
    rules->king_invincible = false;
    rules->king_allowed_to_capture = true;
    rules->goal_square_by_piece_color[PIECE_COLOR_WHITE] = -1;  // these are never accessed if not particular variant
    rules->goal_square_by_piece_color[PIECE_COLOR_BLACK] = -1;
    //rules->pieces_two_lives = false;
    for (int i = 0; i < MAX_DIMENSIONS; ++i) {
        rules->dimension_wrapping[i] = false;   // not necessary. indexes over dimensions never accessed
    }
    rules->promotion_after_x_steps_in_single_dimension = 0;
    rules->non_pawn_piece_types_that_can_promote[0] = NULL_PIECE_TYPE;
    rules->simultaneous_moves = false;
    //rules->starting_position_snapshot;
    rules->gravity_dimension = -1;  // no gravity
    rules->gravity_direction = 0;   // no gravity
    rules->can_move_anywhere_unoccupied = false;

    // GAME STATE
    game_state->whos_turn = PIECE_COLOR_WHITE;
    game_state->moves_made_this_turn = 0;
    for (int piece_color = 0; piece_color < PIECE_COLOR_COUNT; ++piece_color) {
        for (int i = 0; i < MAX_MOVES_PER_TURN; ++i) {
            game_state->last_moves_by_piece_color[piece_color][i].destination_square = -1;
        }
    }

    switch (variant) {
        case STANDARD_CHESS:
            game_state->board = parse_board_from_textfile("standard_8x8.txt", rules->dimensions, rules->board_shape);
            break;
        case ANYTHING_CAN_PROMOTE_CHESS:
            rules->non_pawn_piece_types_that_can_promote[0] = ROOK;
            rules->non_pawn_piece_types_that_can_promote[1] = KNIGHT;
            rules->non_pawn_piece_types_that_can_promote[2] = BISHOP;
            game_state->board = parse_board_from_textfile("standard_8x8.txt", rules->dimensions, rules->board_shape);
            break;
        case LONG_RANGE_CHESS:
            rules->board_shape[0] = 8;
            rules->board_shape[1] = 12;
            rules->moves_per_turn_by_color[PIECE_COLOR_WHITE] = 2;
            rules->moves_per_turn_by_color[PIECE_COLOR_BLACK] = 2;
            game_state->board = parse_board_from_textfile("long_range_8x16.txt", rules->dimensions, rules->board_shape);
            game_state->moves_made_this_turn = 1;
            break;
        case TWO_MOVES_CHESS:
            rules->moves_per_turn_by_color[PIECE_COLOR_WHITE] = 2;
            rules->moves_per_turn_by_color[PIECE_COLOR_BLACK] = 2;
            rules->win_conditions[0] = KING_CAPTURED;
            game_state->board = parse_board_from_textfile("standard_8x8.txt", rules->dimensions, rules->board_shape);
            game_state->moves_made_this_turn = 1;
            break;
        case TEN_MOVES_CHESS:
            rules->moves_per_turn_by_color[PIECE_COLOR_WHITE] = 10;
            rules->moves_per_turn_by_color[PIECE_COLOR_BLACK] = 10;
            rules->win_conditions[0] = KING_CAPTURED;
            game_state->board = parse_board_from_textfile("standard_8x8.txt", rules->dimensions, rules->board_shape);
            game_state->moves_made_this_turn = 5;
            break;
        case STANDARD_10X10_CHESS:
            rules->board_shape[0] = 10;
            rules->board_shape[1] = 10;
            game_state->board = parse_board_from_textfile("standard_10x10.txt", rules->dimensions, rules->board_shape);
            break;
        case STANDARD_24X24_CHESS:
            rules->board_shape[0] = 24;
            rules->board_shape[1] = 24;
            game_state->board = parse_board_from_textfile("standard_24x24.txt", rules->dimensions, rules->board_shape);
            break;
        case SPARSE_CHESS:
            rules->board_shape[0] = 17;
            rules->board_shape[1] = 12;
            game_state->board = parse_board_from_textfile("sparse_17x12.txt", rules->dimensions, rules->board_shape);
            break;
//        case CAPTURE_ALL_PAWNS_CHESS:
//            rules->win_conditions[0] = ALL_PAWNS_CAPTURED;
//            break;
        case KING_MARCH_CHESS:
            rules->king_invincible = true;
            rules->king_allowed_to_capture = false;
            rules->win_conditions[0] = KING_ARRIVED;
            rules->goal_square_by_piece_color[PIECE_COLOR_WHITE] = 60;
            rules->goal_square_by_piece_color[PIECE_COLOR_BLACK] = 4;
            game_state->board = parse_board_from_textfile("standard_8x8.txt", rules->dimensions, rules->board_shape);
            break;
        case GRAVITY_CHESS:
            rules->gravity_dimension = 0;
            rules->gravity_direction = 1;
            game_state->board = parse_board_from_textfile("standard_8x8.txt", rules->dimensions, rules->board_shape);
            break;
        case RANK_SEVEN_AND_EIGHT_CHESS:
            game_state->board = parse_board_from_textfile("rank_seven_and_eight.txt", rules->dimensions, rules->board_shape);
            break;
        case MORE_PAWNS_CHESS:
            game_state->board = parse_board_from_textfile("more_pawns_8x8.txt", rules->dimensions, rules->board_shape);
            break;
        case MORE_PAWNS_GRAVITY_CHESS:
            rules->gravity_dimension = 0;
            rules->gravity_direction = 1;
            game_state->board = parse_board_from_textfile("more_pawns_8x8.txt", rules->dimensions, rules->board_shape);
            break;
        case MOVE_TO_ANY_SQUARE_CHESS:
            rules->can_move_anywhere_unoccupied = true;
            game_state->board = parse_board_from_textfile("standard_8x8.txt", rules->dimensions, rules->board_shape);
            break;
        case THREE_D_5X5X5_CHESS:
            rules->dimensions = 3;
            rules->board_shape[0] = 5;
            rules->board_shape[1] = 5;
            rules->board_shape[2] = 5;
            rules->is_forward_dimension[0] = false;
            rules->is_forward_dimension[1] = true;
            rules->is_forward_dimension[2] = false;
            game_state->board = parse_board_from_textfile("three_d_5x5x5.txt", rules->dimensions, rules->board_shape);
            break;
        case FOUR_D_3X3X3X3_V1_CHESS:
            rules->dimensions = 4;
            rules->board_shape[0] = 3;
            rules->board_shape[1] = 3;
            rules->board_shape[2] = 3;
            rules->board_shape[3] = 3;
            rules->is_forward_dimension[0] = false;
            rules->is_forward_dimension[1] = true;
            rules->is_forward_dimension[2] = false;
            rules->is_forward_dimension[3] = true;
            game_state->board = parse_board_from_textfile("four_d_3x3x3x3_v1.txt", rules->dimensions, rules->board_shape);
            break;
        case FOUR_D_3X3X3X3_V2_CHESS:
            rules->dimensions = 4;
            rules->board_shape[0] = 3;
            rules->board_shape[1] = 3;
            rules->board_shape[2] = 3;
            rules->board_shape[3] = 3;
            rules->is_forward_dimension[0] = false;
            rules->is_forward_dimension[1] = true;
            rules->is_forward_dimension[2] = false;
            rules->is_forward_dimension[3] = true;
            game_state->board = parse_board_from_textfile("four_d_3x3x3x3_v2.txt", rules->dimensions, rules->board_shape);
            break;
        case FOUR_D_3X3X3X3_V4_CHESS:
            rules->dimensions = 4;
            rules->board_shape[0] = 3;
            rules->board_shape[1] = 3;
            rules->board_shape[2] = 3;
            rules->board_shape[3] = 3;
            rules->is_forward_dimension[0] = false;
            rules->is_forward_dimension[1] = true;
            rules->is_forward_dimension[2] = false;
            rules->is_forward_dimension[3] = true;
            game_state->board = parse_board_from_textfile("four_d_3x3x3x3_v3.txt", rules->dimensions, rules->board_shape);
            break;
        case FOUR_D_3X3X3X3_V3_CHESS:
            rules->dimensions = 4;
            rules->board_shape[0] = 3;
            rules->board_shape[1] = 3;
            rules->board_shape[2] = 3;
            rules->board_shape[3] = 3;
            rules->is_forward_dimension[0] = false;
            rules->is_forward_dimension[1] = true;
            rules->is_forward_dimension[2] = false;
            rules->is_forward_dimension[3] = true;
            game_state->board = parse_board_from_textfile("four_d_3x3x3x3_v4.txt", rules->dimensions, rules->board_shape);
            break;
        case FOUR_D_4X4X4X4_V1_CHESS:
            rules->dimensions = 4;
            rules->board_shape[0] = 4;
            rules->board_shape[1] = 4;
            rules->board_shape[2] = 4;
            rules->board_shape[3] = 4;
            rules->is_forward_dimension[0] = false;
            rules->is_forward_dimension[1] = true;
            rules->is_forward_dimension[2] = false;
            rules->is_forward_dimension[3] = true;
            game_state->board = parse_board_from_textfile("four_d_4x4x4x4_v1.txt", rules->dimensions, rules->board_shape);
            break;
        case FOUR_D_4X4X4X4_V2_CHESS:
            rules->dimensions = 4;
            rules->board_shape[0] = 4;
            rules->board_shape[1] = 4;
            rules->board_shape[2] = 4;
            rules->board_shape[3] = 4;
            rules->is_forward_dimension[0] = false;
            rules->is_forward_dimension[1] = true;
            rules->is_forward_dimension[2] = false;
            rules->is_forward_dimension[3] = true;
            game_state->board = parse_board_from_textfile("four_d_4x4x4x4_v2.txt", rules->dimensions, rules->board_shape);
            break;
        case FOUR_D_8X8X8X8_V2_CHESS:
            rules->dimensions = 4;
            rules->board_shape[0] = 8;
            rules->board_shape[1] = 8;
            rules->board_shape[2] = 8;
            rules->board_shape[3] = 8;
            rules->is_forward_dimension[0] = false;
            rules->is_forward_dimension[1] = true;
            rules->is_forward_dimension[2] = false;
            rules->is_forward_dimension[3] = true;
            game_state->board = parse_board_from_textfile("four_d_8x8x8x8_v2.txt", rules->dimensions, rules->board_shape);
            break;
        case SIX_D_2X2X2X2X2X2_CHESS:
            rules->dimensions = 6;
            rules->board_shape[0] = 2;
            rules->board_shape[1] = 2;
            rules->board_shape[2] = 2;
            rules->board_shape[3] = 2;
            rules->board_shape[4] = 2;
            rules->board_shape[5] = 2;
            rules->is_forward_dimension[0] = false;
            rules->is_forward_dimension[1] = true;
            rules->is_forward_dimension[2] = false;
            rules->is_forward_dimension[3] = true;
            rules->is_forward_dimension[4] = false;
            rules->is_forward_dimension[5] = true;
            game_state->board = parse_board_from_textfile("six_d_2x2x2x2x2x2.txt", rules->dimensions, rules->board_shape);
            break;
        case SIX_D_3X3X3X3X3X3_CHESS:
            rules->dimensions = 6;
            rules->board_shape[0] = 3;
            rules->board_shape[1] = 3;
            rules->board_shape[2] = 3;
            rules->board_shape[3] = 3;
            rules->board_shape[4] = 3;
            rules->board_shape[5] = 3;
            rules->is_forward_dimension[0] = false;
            rules->is_forward_dimension[1] = true;
            rules->is_forward_dimension[2] = false;
            rules->is_forward_dimension[3] = true;
            rules->is_forward_dimension[4] = false;
            rules->is_forward_dimension[5] = true;
            game_state->board = parse_board_from_textfile("six_d_3x3x3x3x3x3.txt", rules->dimensions, rules->board_shape);
            break;
        case WRAPPING_12X12_CHESS:
            rules->board_shape[0] = 12;
            rules->board_shape[1] = 12;
            rules->dimension_wrapping[0] = true;
            rules->dimension_wrapping[1] = true;
            game_state->board = parse_board_from_textfile("wrapping_12x12.txt", rules->dimensions, rules->board_shape);
            break;
        case WRAPPING_10X10_CHESS:
            rules->board_shape[0] = 10;
            rules->board_shape[1] = 10;
            rules->dimension_wrapping[0] = true;
            rules->dimension_wrapping[1] = true;
            game_state->board = parse_board_from_textfile("wrapping_10x10.txt", rules->dimensions, rules->board_shape);
            break;
        case WRAPPING_8X14_CHESS:
            rules->board_shape[0] = 8;
            rules->board_shape[1] = 14;
            rules->dimension_wrapping[0] = false;
            rules->dimension_wrapping[1] = true;
            rules->promotion_after_x_steps_in_single_dimension = 7;
            game_state->board = parse_board_from_textfile("wrapping_8x14.txt", rules->dimensions, rules->board_shape);
            break;
        default:
            printf("Tragedy: Variant not defined in set_rules\n");
            return false;
    }

    if (game_state->board == NULL) {
        return false;
    }
    return true;
}

bool initialize_rules(struct Rules *rules, enum Variant variant) {
    rules->dimensions = 2;
    rules->board_shape[0] = 8; 
    rules->board_shape[1] = 8;
    rules->is_forward_dimension[0] = false;
    rules->is_forward_dimension[1] = true;
    rules->win_conditions[0] = CHECKMATE;
    for (int i = 1; i < MAX_NBR_OF_WIN_CONDITIONS; ++i) {
        rules->win_conditions[i] = NULL_WIN_CONDITION;   // not necessary. indexes over dimensions never accessed
    }
    rules->castling_allowed = true;
    rules->moves_per_turn_by_color[PIECE_COLOR_WHITE] = 1;
    rules->moves_per_turn_by_color[PIECE_COLOR_BLACK] = 1;
    rules->same_piece_can_move_twice = false;
    rules->king_invincible = false;
    rules->king_allowed_to_capture = true;
    rules->goal_square_by_piece_color[PIECE_COLOR_WHITE] = -1;  // these are never accessed if not particular variant
    rules->goal_square_by_piece_color[PIECE_COLOR_BLACK] = -1;
    //rules->pieces_two_lives = false;
    for (int i = 0; i < MAX_DIMENSIONS; ++i) {
        rules->dimension_wrapping[i] = false;   // not necessary. indexes over dimensions never accessed
    }
    rules->promotion_after_x_steps_in_single_dimension = 0;
    rules->non_pawn_piece_types_that_can_promote[0] = NULL_PIECE_TYPE;
    rules->simultaneous_moves = false;
    //rules->starting_position_snapshot;
    rules->gravity_dimension = -1;  // no gravity
    rules->gravity_direction = 0;   // no gravity
    rules->can_move_anywhere_unoccupied = false;

    switch (variant) {
        case STANDARD_CHESS:
            break;
        case ANYTHING_CAN_PROMOTE_CHESS:
            rules->non_pawn_piece_types_that_can_promote[0] = ROOK;
            rules->non_pawn_piece_types_that_can_promote[1] = KNIGHT;
            rules->non_pawn_piece_types_that_can_promote[2] = BISHOP;
            break;
        case LONG_RANGE_CHESS:
            rules->board_shape[0] = 8;
            rules->board_shape[1] = 12;
            rules->moves_per_turn_by_color[PIECE_COLOR_WHITE] = 2;
            rules->moves_per_turn_by_color[PIECE_COLOR_BLACK] = 2;
            break;
        case TWO_MOVES_CHESS:
            rules->moves_per_turn_by_color[PIECE_COLOR_WHITE] = 2;
            rules->moves_per_turn_by_color[PIECE_COLOR_BLACK] = 2;
            rules->win_conditions[0] = KING_CAPTURED;
            break;
        case TEN_MOVES_CHESS:
            rules->moves_per_turn_by_color[PIECE_COLOR_WHITE] = 10;
            rules->moves_per_turn_by_color[PIECE_COLOR_BLACK] = 10;
            rules->win_conditions[0] = KING_CAPTURED;
            break;
        case STANDARD_10X10_CHESS:
            rules->board_shape[0] = 10;
            rules->board_shape[1] = 10;
            break;
        case STANDARD_24X24_CHESS:
            rules->board_shape[0] = 24;
            rules->board_shape[1] = 24;
            break;
        case SPARSE_CHESS:
            rules->board_shape[0] = 17;
            rules->board_shape[1] = 12;
            break;
//        case CAPTURE_ALL_PAWNS_CHESS:
//            rules->win_conditions[0] = ALL_PAWNS_CAPTURED;
//            break;
        case KING_MARCH_CHESS:
            rules->king_invincible = true;
            rules->king_allowed_to_capture = false;
            rules->win_conditions[0] = KING_ARRIVED;
            rules->goal_square_by_piece_color[PIECE_COLOR_WHITE] = 60;
            rules->goal_square_by_piece_color[PIECE_COLOR_BLACK] = 4;
            break;
        case RANK_SEVEN_AND_EIGHT_CHESS:
            break;
        case GRAVITY_CHESS:
            rules->gravity_dimension = 0;
            rules->gravity_direction = 1;
            break;
        case MORE_PAWNS_CHESS:
            break;
        case MORE_PAWNS_GRAVITY_CHESS:
            rules->gravity_dimension = 0;
            rules->gravity_direction = 1;
            break;
        case MOVE_TO_ANY_SQUARE_CHESS:
            rules->can_move_anywhere_unoccupied = true;
            break;
        case THREE_D_5X5X5_CHESS:
            rules->dimensions = 3;
            rules->board_shape[0] = 5;
            rules->board_shape[1] = 5;
            rules->board_shape[2] = 5;
            rules->is_forward_dimension[0] = false;
            rules->is_forward_dimension[1] = true;
            rules->is_forward_dimension[2] = false;
            break;
        case FOUR_D_3X3X3X3_V1_CHESS:
            ; // fall through
        case FOUR_D_3X3X3X3_V2_CHESS:
            ; // fall through
        case FOUR_D_3X3X3X3_V4_CHESS:
            ; // fall through
        case FOUR_D_3X3X3X3_V3_CHESS:
            rules->dimensions = 4;
            rules->board_shape[0] = 3;
            rules->board_shape[1] = 3;
            rules->board_shape[2] = 3;
            rules->board_shape[3] = 3;
            rules->is_forward_dimension[0] = false;
            rules->is_forward_dimension[1] = true;
            rules->is_forward_dimension[2] = false;
            rules->is_forward_dimension[3] = true;
            break;
        case FOUR_D_4X4X4X4_V1_CHESS:
            ; // fall through
        case FOUR_D_4X4X4X4_V2_CHESS:
            rules->dimensions = 4;
            rules->board_shape[0] = 4;
            rules->board_shape[1] = 4;
            rules->board_shape[2] = 4;
            rules->board_shape[3] = 4;
            rules->is_forward_dimension[0] = false;
            rules->is_forward_dimension[1] = true;
            rules->is_forward_dimension[2] = false;
            rules->is_forward_dimension[3] = true;
            break;
        case FOUR_D_8X8X8X8_V2_CHESS:
            rules->dimensions = 4;
            rules->board_shape[0] = 8;
            rules->board_shape[1] = 8;
            rules->board_shape[2] = 8;
            rules->board_shape[3] = 8;
            rules->is_forward_dimension[0] = false;
            rules->is_forward_dimension[1] = true;
            rules->is_forward_dimension[2] = false;
            rules->is_forward_dimension[3] = true;
            break;
        case SIX_D_3X3X3X3X3X3_CHESS:
            rules->dimensions = 6;
            rules->board_shape[0] = 3;
            rules->board_shape[1] = 3;
            rules->board_shape[2] = 3;
            rules->board_shape[3] = 3;
            rules->board_shape[4] = 3;
            rules->board_shape[5] = 3;
            rules->is_forward_dimension[0] = false;
            rules->is_forward_dimension[1] = true;
            rules->is_forward_dimension[2] = false;
            rules->is_forward_dimension[3] = true;
            rules->is_forward_dimension[4] = false;
            rules->is_forward_dimension[5] = true;
            break;
        case SIX_D_2X2X2X2X2X2_CHESS:
            rules->dimensions = 6;
            rules->board_shape[0] = 2;
            rules->board_shape[1] = 2;
            rules->board_shape[2] = 2;
            rules->board_shape[3] = 2;
            rules->board_shape[4] = 2;
            rules->board_shape[5] = 2;
            rules->is_forward_dimension[0] = false;
            rules->is_forward_dimension[1] = true;
            rules->is_forward_dimension[2] = false;
            rules->is_forward_dimension[3] = true;
            rules->is_forward_dimension[4] = false;
            rules->is_forward_dimension[5] = true;
            break;
        case WRAPPING_12X12_CHESS:
            rules->board_shape[0] = 12;
            rules->board_shape[1] = 12;
            rules->dimension_wrapping[0] = true;
            rules->dimension_wrapping[1] = true;
            break;
        case WRAPPING_10X10_CHESS:
            rules->board_shape[0] = 10;
            rules->board_shape[1] = 10;
            rules->dimension_wrapping[0] = true;
            rules->dimension_wrapping[1] = true;
            break;
        case WRAPPING_8X14_CHESS:
            rules->board_shape[0] = 8;
            rules->board_shape[1] = 14;
            rules->dimension_wrapping[0] = false;
            rules->dimension_wrapping[1] = true;
            rules->promotion_after_x_steps_in_single_dimension = 7;
            break;
        default:
            printf("Tragedy: Variant not defined in set_rules\n");
            return false;
    }
    return true;
}

bool initialize_game_state(struct GameState *state, enum Variant variant, int dimensions, int *board_shape) {
    state->whos_turn = PIECE_COLOR_WHITE;
    state->moves_made_this_turn = 0;
    for (int piece_color = 0; piece_color < PIECE_COLOR_COUNT; ++piece_color) {
        for (int i = 0; i < MAX_MOVES_PER_TURN; ++i) {
            state->last_moves_by_piece_color[piece_color][i].destination_square = -1;
        }
    }

    struct Square *board;
    switch (variant) {
        case STANDARD_CHESS:
            board = parse_board_from_textfile("standard_8x8.txt", dimensions, board_shape);
            break;
        case ANYTHING_CAN_PROMOTE_CHESS:
            board = parse_board_from_textfile("standard_8x8.txt", dimensions, board_shape);
            break;
        case LONG_RANGE_CHESS:
            board = parse_board_from_textfile("long_range_8x16.txt", dimensions, board_shape);
            state->moves_made_this_turn = 1;
            break;
        case TWO_MOVES_CHESS:
            board = parse_board_from_textfile("standard_8x8.txt", dimensions, board_shape);
            state->moves_made_this_turn = 1;
            break;
        case TEN_MOVES_CHESS:
            board = parse_board_from_textfile("standard_8x8.txt", dimensions, board_shape);
            state->moves_made_this_turn = 5;
            break;
        case STANDARD_10X10_CHESS:
            board = parse_board_from_textfile("standard_10x10.txt", dimensions, board_shape);
            break;
        case STANDARD_24X24_CHESS:
            board = parse_board_from_textfile("standard_24x24.txt", dimensions, board_shape);
            break;
        case SPARSE_CHESS:
            board = parse_board_from_textfile("sparse_17x12.txt", dimensions, board_shape);
            break;
        case KING_MARCH_CHESS:
            board = parse_board_from_textfile("standard_8x8.txt", dimensions, board_shape);
            break;
        case GRAVITY_CHESS:
            board = parse_board_from_textfile("standard_8x8.txt", dimensions, board_shape);
            break;
        case RANK_SEVEN_AND_EIGHT_CHESS:
            board = parse_board_from_textfile("rank_seven_and_eight.txt", dimensions, board_shape);
            break;
        case MORE_PAWNS_CHESS:
            board = parse_board_from_textfile("more_pawns_8x8.txt", dimensions, board_shape);
            break;
        case MORE_PAWNS_GRAVITY_CHESS:
            board = parse_board_from_textfile("more_pawns_8x8.txt", dimensions, board_shape);
            break;
        case MOVE_TO_ANY_SQUARE_CHESS:
            board = parse_board_from_textfile("standard_8x8.txt", dimensions, board_shape);
            break;
        case THREE_D_5X5X5_CHESS:
            board = parse_board_from_textfile("three_d_5x5x5.txt", dimensions, board_shape);
            break;
        case FOUR_D_3X3X3X3_V1_CHESS:
            board = parse_board_from_textfile("four_d_3x3x3x3_v1.txt", dimensions, board_shape);
            break;
        case FOUR_D_3X3X3X3_V2_CHESS:
            board = parse_board_from_textfile("four_d_3x3x3x3_v2.txt", dimensions, board_shape);
            break;
        case FOUR_D_3X3X3X3_V3_CHESS:
            board = parse_board_from_textfile("four_d_3x3x3x3_v3.txt", dimensions, board_shape); //todo 4->3
            break;
        case FOUR_D_3X3X3X3_V4_CHESS:
            board = parse_board_from_textfile("four_d_3x3x3x3_v4.txt", dimensions, board_shape); //todo 4->3
            break;
        case FOUR_D_4X4X4X4_V1_CHESS:
            board = parse_board_from_textfile("four_d_4x4x4x4_v1.txt", dimensions, board_shape);
            break;
        case FOUR_D_4X4X4X4_V2_CHESS:
            board = parse_board_from_textfile("four_d_4x4x4x4_v2.txt", dimensions, board_shape);
            break;
        case FOUR_D_8X8X8X8_V2_CHESS:
            board = parse_board_from_textfile("four_d_8x8x8x8_v2.txt", dimensions, board_shape);
            break;
        case SIX_D_2X2X2X2X2X2_CHESS:
            board = parse_board_from_textfile("six_d_2x2x2x2x2x2.txt", dimensions, board_shape);
            break;
        case SIX_D_3X3X3X3X3X3_CHESS:
            board = parse_board_from_textfile("six_d_3x3x3x3x3x3.txt", dimensions, board_shape);
            break;
        case CAPTURE_ALL_PAWNS_CHESS:
            board = parse_board_from_textfile("standard_8x8.txt", dimensions, board_shape);
            break;
        case WRAPPING_12X12_CHESS:
            board = parse_board_from_textfile("wrapping_12x12.txt", dimensions, board_shape);
            break;
        case WRAPPING_10X10_CHESS:
            board = parse_board_from_textfile("wrapping_10x10.txt", dimensions, board_shape);
            break;
        case WRAPPING_8X14_CHESS:
            board = parse_board_from_textfile("wrapping_8x14.txt", dimensions, board_shape);
            break;
        default:
            printf("Catastrophe: Variant not defined in set_initial_state\n");
            return false;
    }
    if (board == NULL) {
        return false;
    }
    state->board = board;
    return true;
}

static struct Square *parse_board_from_textfile(char *text_file_name, int dimensions, int *board_shape) {
    int length = 1;
    for (int i = 0; i < dimensions; ++i) {
        length *= board_shape[i];
    }
    struct Square *board = malloc(sizeof(*board) * length);
    // TODO: handle malloc failing

    char text_file_path[200] = "starting_positions/";
    strcat(text_file_path, text_file_name);
    FILE *file = fopen(text_file_path, "r");
    if (file == NULL) {
        printf("Debacle: failed to open starting position file\n");
        return NULL;
    }

    int square[dimensions];             // starts at 'top left' just like the position in the text file
    for (int i = 0; i<dimensions; ++i) {
        if (i%2 == 0) {
            square[i] = 0;
        } else {
            square[i] = board_shape[i] - 1;
        }
    }

    int dim_change_order[dimensions];   // [0,2,4,1,3,5] for six dimensional chess
    int i = 0;
    for (int dim = 0; dim<dimensions; dim += 2, ++i) {
        dim_change_order[i] = dim;
    }
    for (int dim = 1; dim<dimensions; dim += 2, ++i) {
        dim_change_order[i] = dim;
    }

    int c, c2, c3;
    while ((c = fgetc(file)) != EOF) 
    {
        if (c == '|' || c == '-' || c == ' ' || c == '\n') { 
            continue;
        }

        if (square[dim_change_order[dimensions-1]] < 0) {   //last dim to change less than 0
            printf("Trouble: to many chars in starting position text file compared to board shape.\n");
            return NULL;
        }

        if (c != 'w' & c != 'b' & c != 'x' & c != '.') {
            printf("Shock and horror: char: %c in starting position text file not valid.\n", c);
            return NULL;
        }

        c2 = fgetc(file);
        if (c2 != 'p' & c2 != 'R' & c2 != 'N' & c2 != 'B' & c2 != 'Q' & c2 != 'K' & c2 != 'F' & c2 != 'x' & c2 != '.') {
            printf("Blunder: char: %c in starting position text file not valid.\n", c2);
            return NULL;
        }

        c3 = fgetc(file);
        if (c3 != 'f' & c3 != 'b' & c3 != 'r' & c3 != 'l' & c3 != '.') {
            ungetc(c3, file);
            c3 = '\0';
        }

        int index = square_to_square_index(square, dimensions, board_shape);
        board[index].part_of_board = true;
        board[index].white_flag = false;
        board[index].black_flag = false;
        board[index].piece.has_moved = false;
        //board[index].piece.has_been_captured_once = false;
        //board[index].piece.start_square = index;
        switch(c) {
            case 'w': board[index].piece.piece_color = PIECE_COLOR_WHITE; break;
            case 'b': board[index].piece.piece_color = PIECE_COLOR_BLACK; break;
            default:  board[index].piece.piece_color = NULL_PIECE_COLOR; break;
        }
        switch (c2) {
            case 'p': board[index].piece.piece_type = PAWN; break;
            case 'R': board[index].piece.piece_type = ROOK; break;
            case 'N': board[index].piece.piece_type = KNIGHT; break;
            case 'B': board[index].piece.piece_type = BISHOP; break;
            case 'K': board[index].piece.piece_type = KING; break;
            case 'Q': board[index].piece.piece_type = QUEEN; break;
            case 'F':
                board[index].piece.piece_type = NULL_PIECE_TYPE;
                if (c == 'w') { 
                    board[index].white_flag = true;
                } else if (c == 'b') {
                    board[index].black_flag = true;
                }
            case 'x': board[index].part_of_board = false; break;
            case '.': board[index].piece.piece_type = NULL_PIECE_TYPE; break;
            default: 
                printf("Square content in starting positions file not valid. Index %d\n", index);
                return NULL;
        }
        switch(c3) {
            case 'f': board[index].piece.direction = FORWARDS; break;
            case 'b': board[index].piece.direction = BACKWARDS; break;
            case 'r': board[index].piece.direction = RIGHT; break;
            case 'l': board[index].piece.direction = LEFT; break;
            default:  board[index].piece.direction = FORWARDS; break;
        }

        square[0] += 1;
        for (int i = 0; i < dimensions-1; ++i) {
            int dim = dim_change_order[i];
            int dim_next = dim_change_order[i+1];
            //printf("info: %d %d %d %d\n", dim, dim_next, square[0], square[1]);
            if (square[dim] < 0 || square[dim] == board_shape[dim]) {
                // increment next dim in dim_change_order and reset current and previous dims
                if (dim_next % 2 == 0) {
                    square[dim_next] += 1;
                } else {
                    square[dim_next] -= 1;
                }
                for (int j = i; j >= 0; --j) {
                    int dim_to_reset = dim_change_order[j];
                    if (dim_to_reset % 2 == 0) {
                        square[dim_to_reset] = 0;
                    } else {
                        square[dim_to_reset] = board_shape[dim_to_reset] - 1;
                    }
                }
            }
        }
    }
    return board;
}

void terminate_game_state(struct GameState *game_state) {
    free(game_state->board);
}

