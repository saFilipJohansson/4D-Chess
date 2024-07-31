#include <stdio.h>
#include <stdlib.h>
#include "chess.h"

static bool increment_dim_of_square_if_legal(int square[], int dim, int incr, bool dimension_wrapping, int board_shape_dim);
static void get_pawn_moves  (struct Move moves[], struct Move diagonal_pawn_moves[], int square_index, struct Square board[], 
                             struct Move last_moves_by_piece_color[PIECE_COLOR_COUNT][MAX_MOVES_PER_TURN], struct Rules *rules);
//static void get_direct_pawn_captures (struct Move moves[], int square_index, struct Square board[], struct Rules *rules);
static void get_rook_moves  (struct Move moves[MAX_MOVES_SINGLE_PIECE], int square_index, struct Square board[], 
                             struct Rules *rules);
static void get_bishop_moves(struct Move moves[MAX_MOVES_SINGLE_PIECE], int square_index, struct Square board[], 
                             struct Rules *rules);
static void get_knight_moves(struct Move moves[MAX_MOVES_SINGLE_PIECE], int square_index, struct Square board[], 
                             struct Rules *rules);
static void get_king_moves  (struct Move moves[MAX_MOVES_SINGLE_PIECE], int square_index, bool in_check, struct Square board[], 
                             struct Rules *rules);
static int  get_castling_moves (struct Move moves[], int king_square_index, struct Square board[], struct Rules *rules);
static void get_queen_moves (struct Move moves[MAX_MOVES_SINGLE_PIECE], int square_index, struct Square board[], 
                             struct Rules *rules);
static void get_all_moves_to_unoccupied (struct Move moves[], int square_index, struct Square board[], struct Rules *rules);

//static bool piece_color_in_check(enum PieceColor piece_color, int king_square, struct Square *board, struct Rules *rules);
//static bool move_puts_own_king_in_check(struct Move move, struct Square *board, struct Rules *rules);
static bool square_is_attacked(int square_index, enum PieceColor attacked_piece_color, struct Square *board, struct Rules *rules);
static bool player_is_checkmated(enum PieceColor piece_color, struct GameState *game_state, struct Rules *rules);
static bool piece_color_king_captured(bool piece_color, struct GameState *game_state, struct Rules *rules);

static void evaluate_gravity(struct Square board[], struct Rules *rules);

void get_moves(struct Move moves[MAX_MOVES_SINGLE_PIECE], struct Move diagonal_pawn_moves[MAX_MOVES_SINGLE_PIECE], 
               int square_index, struct GameState *game_state, struct Rules *rules) {
    enum PieceType piece_type = game_state->board[square_index].piece.piece_type;
    diagonal_pawn_moves[0].destination_square = -1;
    switch (piece_type) {
        case NULL_PIECE_TYPE:
            moves[0].destination_square = -1;
            diagonal_pawn_moves[0].destination_square = -1;
            return;
        case PAWN:
            get_pawn_moves(moves, diagonal_pawn_moves, square_index, game_state->board, game_state->last_moves_by_piece_color, 
                           rules);
            break;
        case ROOK:
            get_rook_moves(moves, square_index, game_state->board, rules);
            break;
        case KNIGHT:
            get_knight_moves(moves, square_index, game_state->board, rules);
            break;
        case BISHOP:
            get_bishop_moves(moves, square_index, game_state->board, rules);
            break;
        case KING:
            get_king_moves(moves, square_index, false, game_state->board, rules);
            break;
        case QUEEN:
            get_queen_moves(moves, square_index, game_state->board, rules);
            break;
    }

    // Add moves if can_move_anywhere_unoccupied flag set
    if (rules->can_move_anywhere_unoccupied && piece_type != KING) {
        get_all_moves_to_unoccupied(moves, square_index, game_state->board, rules);
    }

    // Is checkmate a win condition?
    //bool check_mate_win_condition = false;
    //for (int i = 0; rules->win_conditions[i] != NULL_WIN_CONDITION; ++i) {
        //if (rules->win_conditions[i] == CHECKMATE) {
            //check_mate_win_condition = true;
        //}
    //}

    for (int i = 0; moves[i].destination_square != -1; ++i) {
        // King invincible: Remove moves that capture opponents king
        if ((rules->king_invincible)) {
            if (game_state->board[moves[i].destination_square].piece.piece_type == KING) {
                for (int j = i; moves[j].destination_square != -1; ++j) {
                    moves[j] = moves[j+1];
                }
                --i;
            }
        // King not invincible: Remove moves that put own king in check
        // TODO:
        } //else if (check_mate_win_condition && move_puts_own_king_in_check(moves[i], game_state->board, rules)) {
        //    for (int j = i; moves[j].destination_square != -1; ++j) {
        //        moves[j] = moves[j+1];
        //    }
        //    --i;
        //}
    }
}

struct Move validate_selected_move(int origin_square, int destination_square, struct Move moves[], struct GameState *game_state, 
                                   struct Rules *rules) {
    struct Move move;
    move.destination_square = -1;
    enum PieceColor piece_color = game_state->board[origin_square].piece.piece_color;

    // No piece at origin square
    if (game_state->board[origin_square].piece.piece_type == NULL_PIECE_TYPE) {     // defensive programming?
        return move;
    }

    // Not this players turn
    if (game_state->board[origin_square].piece.piece_color != game_state->whos_turn) {
        return move;
    }

    // TODO check rules first
    // Disqualify moves of pieces that have already moved this turn
    if (!rules->same_piece_can_move_twice && rules->moves_per_turn_by_color[piece_color] > 1 && !rules->same_piece_can_move_twice) {
        for (int i = 0; i < game_state->moves_made_this_turn; ++i) {
            if (game_state->last_moves_by_piece_color[piece_color][i].destination_square == origin_square) {
                return move;
            }
        }
    }

    // Check if move in moves
    for (int i = 0; moves[i].destination_square != -1; ++i) {
        if (destination_square == moves[i].destination_square) {
            move = moves[i];
            return move;
        }
    }
    return move;
}

void make_move(struct Move move, enum PieceType promotion_piece_type, struct GameState *game_state, struct Rules *rules) {
    enum PieceType piece_type = game_state->board[move.origin_square].piece.piece_type;

    // Make the move
    game_state->board[move.destination_square].piece = game_state->board[move.origin_square].piece;
    game_state->board[move.origin_square].piece.piece_type = NULL_PIECE_TYPE;

    // Gravity if gravity
    if (rules->gravity_dimension != -1) {
        evaluate_gravity(game_state->board, rules);
    }

    // En passant: Remove captured pawn
    enum PieceColor piece_color = game_state->board[move.origin_square].piece.piece_color;
    if (piece_type == PAWN && move.en_passant_capture) {
        for (enum PieceColor opponent_piece_color = 0; opponent_piece_color < PIECE_COLOR_COUNT; ++opponent_piece_color) {
            if (opponent_piece_color == piece_color) {
                continue;
            }
            for (int i = 0; i < rules->moves_per_turn_by_color[opponent_piece_color]; ++i) {
                if (    game_state->last_moves_by_piece_color[opponent_piece_color][i].pawn_moved_past_square == 
                        move.destination_square) {
                    int square = game_state->last_moves_by_piece_color[opponent_piece_color][i].destination_square;
                    game_state->board[square].piece.piece_type = NULL_PIECE_TYPE;
                }
            }
        }
    }

    // Deal with castling king move
    if (piece_type == KING && move.castling_with_rook_on_square != -1) {
        game_state->board[move.castling_rook_destination_square].piece = game_state->board[move.castling_with_rook_on_square].piece;
        game_state->board[move.castling_with_rook_on_square].piece.piece_type = NULL_PIECE_TYPE;
    }
    
    // Move the flag if it was on the square

    // Carry out promotion
    if (promotion_piece_type != NULL_PIECE_TYPE) {
        game_state->board[move.destination_square].piece.piece_type = promotion_piece_type;
    }

    // The piece has now moved
    game_state->board[move.destination_square].piece.has_moved = true;

    // Add move to list of last moves by piece color
    game_state->last_moves_by_piece_color[game_state->whos_turn][game_state->moves_made_this_turn] = move;

    ++game_state->moves_made_this_turn;
    if (game_state->moves_made_this_turn >= rules->moves_per_turn_by_color[game_state->whos_turn]) {
        if (game_state->whos_turn == PIECE_COLOR_WHITE) {
            game_state->whos_turn = PIECE_COLOR_BLACK;
        } else {
            game_state->whos_turn = PIECE_COLOR_WHITE;
        }
        game_state->moves_made_this_turn = 0;
    }
}

// TODO: should also check if the square is part of the board
// and if that piece color is allowed on that square
// TODO make increment_dim_of_square_if_legal deal with one dimension wrapping and non-rectangle board shapes. It should increment both dims simultaneously and then move if wrapping
static bool increment_dim_of_square_if_legal(int square[], int dim, int incr, bool dimension_wrapping, int board_shape_dim) {
    // todo: non-rectangle shaped boards
    square[dim] += incr;
    if (square[dim] >= 0 && square[dim] < board_shape_dim) {
        return true;
    } else if (!dimension_wrapping) {
        square[dim] -= incr;
        return false;
    } else if (square[dim] < 0) {
        square[dim] = board_shape_dim - 1;
        return true;
    } else {
        square[dim] = 0;
        return true;
    }
}

// assumes there is a pawn at the square
static void get_pawn_moves(struct Move moves[], struct Move diagonal_pawn_moves[], int square_index, struct Square board[], 
                           struct Move last_moves_by_piece_color[PIECE_COLOR_COUNT][MAX_MOVES_PER_TURN], struct Rules *rules) {
    enum PieceColor piece_color = board[square_index].piece.piece_color;
    int dimensions = rules->dimensions;
    int origin_square[dimensions];
    square_index_to_square(square_index, origin_square, dimensions, rules->board_shape);
    int destination_square[dimensions];

    int counter = 0;
    int counter_diag = 0;
    for (int dim = 0; dim < dimensions; ++dim) {
        copy_int_array(origin_square, destination_square, dimensions);

        // Figure out direction for this pawn of this piece color in this dimension.
        enum Direction piece_direction = board[square_index].piece.direction;
        bool dim_is_forward_dimension = rules->is_forward_dimension[dim];
        int incr;
        if (piece_direction == FORWARDS && dim_is_forward_dimension) {
            incr = 1;
        } else if (piece_direction == BACKWARDS && dim_is_forward_dimension) {
            incr = -1;
        } else if (piece_direction == RIGHT && !dim_is_forward_dimension) {
            incr = 1;
        } else if (piece_direction == LEFT && !dim_is_forward_dimension) {
            incr = -1;
        } else {
            continue;
        }
        if (piece_color == PIECE_COLOR_BLACK) {
            incr *= -1;
        }

        bool increment_legal = increment_dim_of_square_if_legal(destination_square, dim, incr, rules->dimension_wrapping[dim], 
                                                                rules->board_shape[dim]);
        if (!increment_legal) {
            continue;
        }

        int destination_square_index = square_to_square_index(destination_square, dimensions, rules->board_shape);
        if (board[destination_square_index].piece.piece_type == NULL_PIECE_TYPE) {
            moves[counter].origin_square = square_index;
            moves[counter].destination_square = destination_square_index;
            moves[counter].pawn_moved_past_square = -1;
            moves[counter].en_passant_capture = false;
            ++counter;

            if (!board[square_index].piece.has_moved) {
                int two_squares_destination_square[dimensions];
                copy_int_array(destination_square, two_squares_destination_square, dimensions);
                increment_legal = increment_dim_of_square_if_legal(two_squares_destination_square, dim, incr, 
                                                                   rules->dimension_wrapping[dim], rules->board_shape[dim]);
                int two_squares_destination_square_index = square_to_square_index(two_squares_destination_square, dimensions, 
                                                                                  rules->board_shape);
                if (increment_legal && board[two_squares_destination_square_index].piece.piece_type == NULL_PIECE_TYPE) {
                    moves[counter].origin_square = square_index;
                    moves[counter].destination_square = two_squares_destination_square_index;
                    moves[counter].pawn_moved_past_square = destination_square_index;
                    moves[counter].en_passant_capture = false;
                    ++counter;
                }
            }
        }

        // Adding pawn captures to moves and all diagonal pawn moves to diagonal_pawn_moves
        // Diagongal pawn moves are moves one step forward in forward dimension for pawn and one step in one non-forward dimension
        for (int dim2 = 0; dim2 < dimensions; ++dim2) {
            if ((piece_direction == FORWARDS || piece_direction == BACKWARDS) && rules->is_forward_dimension[dim2]) {
                continue;
            }
            if ((piece_direction == RIGHT || piece_direction == LEFT) && !rules->is_forward_dimension[dim2]) {
                continue;
            }
            for (incr = -1; incr <= 1; incr += 2) {
                int destination_square_diag[dimensions];
                copy_int_array(destination_square, destination_square_diag, dimensions);
                bool increment_legal = increment_dim_of_square_if_legal(destination_square_diag, dim2, incr, 
                                                                        rules->dimension_wrapping[dim2], rules->board_shape[dim2]);
                if (!increment_legal) {
                    continue;
                }

                destination_square_index = square_to_square_index(destination_square_diag, dimensions, rules->board_shape);
                diagonal_pawn_moves[counter_diag].origin_square = square_index;
                diagonal_pawn_moves[counter_diag].destination_square = destination_square_index;
                diagonal_pawn_moves[counter_diag].pawn_moved_past_square = -1;
                diagonal_pawn_moves[counter_diag].en_passant_capture = false;
                ++counter_diag;
                if (    board[destination_square_index].piece.piece_type != NULL_PIECE_TYPE && 
                        board[destination_square_index].piece.piece_color != piece_color) {
                    moves[counter].origin_square = square_index;
                    moves[counter].destination_square = destination_square_index;
                    moves[counter].pawn_moved_past_square = -1;
                    moves[counter].en_passant_capture = false;
                    ++counter;
                }
                // En passant moves
                // WORKS FOR MANY PLAYERS (piece colors). stupid?
                for (enum PieceColor opponent_piece_color = 0; opponent_piece_color < PIECE_COLOR_COUNT; ++opponent_piece_color) {
                    if (opponent_piece_color == piece_color) {
                        continue;
                    }
                    for (int i = 0; i < rules->moves_per_turn_by_color[opponent_piece_color]; ++i) {
                        if (last_moves_by_piece_color[opponent_piece_color][i].pawn_moved_past_square 
                                == destination_square_index) {
                            moves[counter].origin_square = square_index;
                            moves[counter].destination_square = destination_square_index;
                            moves[counter].pawn_moved_past_square = -1;
                            moves[counter].en_passant_capture = true;
                            ++counter;
                        }
                    }
                }
            }
        }
    }

    // terminate the arrays
    moves[counter].destination_square = -1;
    diagonal_pawn_moves[counter_diag].destination_square = -1;
}
    
// For computing checkmate
//static void get_direct_pawn_captures(struct Move moves[], int square_index, struct Square board[], struct Rules *rules) {
//    enum PieceColor piece_color = board[square_index].piece.piece_color;
//    int dimensions = rules->dimensions;
//    int origin_square[dimensions];
//    square_index_to_square(square_index, origin_square, dimensions, rules->board_shape);
//    int destination_square[dimensions];
//
//    int counter = 0;
//    for (int dim = 0; dim < dimensions; ++dim) {
//        copy_int_array(origin_square, destination_square, dimensions);
//
//        // Figure out direction for this pawn of this piece color in this dimension.
//        enum Direction piece_direction = board[square_index].piece.direction;
//        bool dim_is_forward_dimension = rules->is_forward_dimension[dim];
//        int incr;
//        if (piece_direction == FORWARDS && dim_is_forward_dimension) {
//            incr = 1;
//        } else if (piece_direction == BACKWARDS && dim_is_forward_dimension) {
//            incr = -1;
//        } else if (piece_direction == RIGHT && !dim_is_forward_dimension) {
//            incr = 1;
//        } else if (piece_direction == LEFT && !dim_is_forward_dimension) {
//            incr = -1;
//        } else {
//            continue;
//        }
//        if (piece_color == PIECE_COLOR_BLACK) {
//            incr *= -1;
//        }
//
//        bool increment_legal = increment_dim_of_square_if_legal(destination_square, dim, incr, rules->dimension_wrapping[dim], 
//                                                                rules->board_shape[dim]);
//        if (!increment_legal) {
//            continue;
//        }
//
//        int destination_square_index = square_to_square_index(destination_square, dimensions, rules->board_shape);
//
//        for (int dim2 = 0; dim2 < dimensions; ++dim2) {
//            if ((piece_direction == FORWARDS || piece_direction == BACKWARDS) && rules->is_forward_dimension[dim2]) {
//                continue;
//            }
//            if ((piece_direction == RIGHT || piece_direction == LEFT) && !rules->is_forward_dimension[dim2]) {
//                continue;
//            }
//            for (incr = -1; incr <= 1; incr += 2) {
//                int destination_square_diag[dimensions];
//                copy_int_array(destination_square, destination_square_diag, dimensions);
//                bool increment_legal = increment_dim_of_square_if_legal(destination_square_diag, dim2, incr, 
//                                                                        rules->dimension_wrapping[dim2], rules->board_shape[dim2]);
//                if (!increment_legal) {
//                    continue;
//                }
//
//                destination_square_index = square_to_square_index(destination_square_diag, dimensions, rules->board_shape);
//                if (    board[destination_square_index].piece.piece_type != NULL_PIECE_TYPE && 
//                        board[destination_square_index].piece.piece_color != piece_color) {
//                    moves[counter].origin_square = square_index;
//                    moves[counter].destination_square = destination_square_index;
//                    moves[counter].pawn_moved_past_square = -1;
//                    moves[counter].en_passant_capture = false;
//                    ++counter;
//                }
//            }
//        }
//    }
//
//    // terminate the arrays
//    moves[counter].destination_square = -1;
//}

static void get_rook_moves(struct Move moves[MAX_MOVES_SINGLE_PIECE], int square_index, struct Square board[], struct Rules *rules) {
    // Variables
    enum PieceColor piece_color = board[square_index].piece.piece_color;
    int dimensions = rules->dimensions;
    int square[dimensions];
    square_index_to_square(square_index, square, dimensions, rules->board_shape);
    int destination_square[dimensions];

    int counter = 0;
    for (int dim = 0; dim < dimensions; ++dim) {
        for (int direction = -1; direction <= 1; direction += 2) {
            copy_int_array(square, destination_square, dimensions);
            bool stop = false;
            while (!stop) {
                bool increment_legal = increment_dim_of_square_if_legal(destination_square, dim, direction, rules->dimension_wrapping[dim], rules->board_shape[dim]);
                if (!increment_legal) {
                    stop = true;
                    continue;
                }
                int destination_square_index = square_to_square_index(destination_square, dimensions, rules->board_shape);

                if (destination_square_index == square_index) {
                    break;
                } else if (board[destination_square_index].piece.piece_type == NULL_PIECE_TYPE) {
                    moves[counter].origin_square = square_index;
                    moves[counter].destination_square = destination_square_index;
                    ++counter;
                } else if (board[destination_square_index].piece.piece_color != piece_color) {
                    moves[counter].origin_square = square_index;
                    moves[counter].destination_square = destination_square_index;
                    ++counter;
                    break;
                } else {
                    break;
                }
            }
        }
    }
    moves[counter].destination_square = -1;
}

static void get_bishop_moves(struct Move moves[MAX_MOVES_SINGLE_PIECE], int square_index, struct Square board[], struct Rules *rules) {
    // Variables
    enum PieceColor piece_color = board[square_index].piece.piece_color;
    int dimensions = rules->dimensions;
    int square[dimensions];
    square_index_to_square(square_index, square, dimensions, rules->board_shape);
    int destination_square[dimensions];

    int counter = 0;
    for (int dim1 = 0; dim1 < dimensions; ++dim1) {
        for (int dim2 = dim1 + 1; dim2 < dimensions; ++dim2) {
            int increments[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
            for (int i = 0; i < 4; ++i) {
                copy_int_array(square, destination_square, dimensions);
                bool stop = false;
                while (!stop) {
                    bool increment1_legal = increment_dim_of_square_if_legal(destination_square, dim1, increments[i][0], 
                                                                    rules->dimension_wrapping[dim1], rules->board_shape[dim1]);
                    bool increment2_legal = increment_dim_of_square_if_legal(destination_square, dim2, increments[i][1], 
                                                                    rules->dimension_wrapping[dim2], rules->board_shape[dim2]);
                    if (!increment1_legal || !increment2_legal) {
                        stop = true;
                        continue;
                    }
                    int destination_square_index = square_to_square_index(destination_square, dimensions, rules->board_shape);

                    if (destination_square_index == square_index) {
                        break;
                    } else if (board[destination_square_index].piece.piece_type == NULL_PIECE_TYPE) {
                        moves[counter].origin_square = square_index;
                        moves[counter].destination_square = destination_square_index;
                        ++counter;
                    } else if (board[destination_square_index].piece.piece_color != piece_color) {
                        moves[counter].origin_square = square_index;
                        moves[counter].destination_square = destination_square_index;
                        ++counter;
                        break;
                    } else {
                        break;
                    }
                }
            }
        }
    }
    moves[counter].destination_square = -1;
}

static void get_knight_moves(struct Move moves[MAX_MOVES_SINGLE_PIECE], int square_index, struct Square board[], struct Rules *rules) {
    enum PieceColor piece_color = board[square_index].piece.piece_color;
    int dimensions = rules->dimensions;
    int square[dimensions];
    square_index_to_square(square_index, square, dimensions, rules->board_shape);
    int destination_square[dimensions];
    int counter = 0;
    for (int dim1 = 0; dim1 < dimensions; ++dim1) {
        for (int dim2 = 0; dim2 < dimensions; ++dim2) {
            if (dim1 == dim2) {
                continue;
            }
            int increments[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
            for (int i = 0; i < 4; ++i) {
                copy_int_array(square, destination_square, dimensions);
                bool increment1_legal = increment_dim_of_square_if_legal(destination_square, dim1, increments[i][0], rules->dimension_wrapping[dim1], rules->board_shape[dim1]); // TODO make increment_dim_of_square_if_legal deal with one dimension wrapping and non-rectangle board shapes. It should increment both dims simultaneously and then move if wrapping
                bool increment2_legal = increment_dim_of_square_if_legal(destination_square, dim1, increments[i][0], rules->dimension_wrapping[dim1], rules->board_shape[dim1]); // TODO make increment_dim_of_square_if_legal deal with one dimension wrapping and non-rectangle board shapes. It should increment both dims simultaneously and then move if wrapping
                bool increment3_legal = increment_dim_of_square_if_legal(destination_square, dim2, increments[i][1], rules->dimension_wrapping[dim2], rules->board_shape[dim2]);
                if (!increment1_legal || !increment2_legal || !increment3_legal) {
                    continue;
                }
                int destination_square_index = square_to_square_index(destination_square, dimensions, rules->board_shape);
                if (    board[destination_square_index].piece.piece_type == NULL_PIECE_TYPE ||
                        board[destination_square_index].piece.piece_color != piece_color) {
                    moves[counter].origin_square = square_index;
                    moves[counter].destination_square = destination_square_index;
                    ++counter;
                }
            }
        }
    }
    moves[counter].destination_square = -1;
}

static void get_queen_moves(struct Move moves[MAX_MOVES_SINGLE_PIECE], int square_index, struct Square board[], struct Rules *rules) {
    get_rook_moves(moves, square_index, board, rules);
    int i;
    for(i = 0; moves[i].destination_square != -1; ++i) {};
    get_bishop_moves(moves + i, square_index, board, rules);
}

static void get_king_moves(struct Move moves[MAX_MOVES_SINGLE_PIECE], int square_index, bool in_check, struct Square board[], struct Rules *rules) {
    // Variables
    enum PieceColor piece_color = board[square_index].piece.piece_color;
    int dimensions = rules->dimensions;
    int square[dimensions];
    square_index_to_square(square_index, square, dimensions, rules->board_shape);
    int destination_square[dimensions];

    int counter = 0;
    for (int dim1 = 0; dim1 < dimensions; ++dim1) {
        for (int dim2 = dim1 + 1; dim2 < dimensions; ++dim2) {
            if (dim1 == dim2) {
                continue;
            }
            int j;
            int increments[8][2];
            if (dimensions == 2) {
                j = 8;
                increments[0][0] = 1;   increments[0][1] = 0;
                increments[1][0] = -1;  increments[1][1] = 0;
                increments[2][0] = 0;   increments[2][1] = 1;
                increments[3][0] = 0;   increments[3][1] = -1;
                increments[4][0] = 1;   increments[4][1] = 1;
                increments[5][0] = 1;   increments[5][1] = -1;
                increments[6][0] = -1;  increments[6][1] = 1;
                increments[7][0] = -1;  increments[7][1] = -1;
            } else {
                j = 4;
                increments[0][0] = 1;   increments[0][1] = 0;
                increments[1][0] = -1;  increments[1][1] = 0;
                increments[2][0] = 0;   increments[2][1] = 1;
                increments[3][0] = 0;   increments[3][1] = -1;
            }
            for (int i = 0; i < j; ++i) {
                copy_int_array(square, destination_square, dimensions);
                bool increment1_legal = increment_dim_of_square_if_legal(destination_square, dim1, increments[i][0], rules->dimension_wrapping[dim1], rules->board_shape[dim1]); // TODO make increment_dim_of_square_if_legal deal with one dimension wrapping and non-rectangle board shapes. It should increment both dims simultaneously and then move if wrapping
                bool increment2_legal = increment_dim_of_square_if_legal(destination_square, dim2, increments[i][1], rules->dimension_wrapping[dim2], rules->board_shape[dim2]);
                if (!increment1_legal || !increment2_legal) {
                    continue;
                }
                int destination_square_index = square_to_square_index(destination_square, dimensions, rules->board_shape);

                if (board[destination_square_index].piece.piece_type == NULL_PIECE_TYPE) {
                    moves[counter].origin_square = square_index;
                    moves[counter].destination_square = destination_square_index;
                    moves[counter].castling_with_rook_on_square = -1;
                    ++counter;
                } else if (board[destination_square_index].piece.piece_color != piece_color && rules->king_allowed_to_capture) {
                    moves[counter].origin_square = square_index;
                    moves[counter].destination_square = destination_square_index;
                    moves[counter].castling_with_rook_on_square = -1;
                    ++counter;
                }
            }
        }
    }

    // evaluate castling moves
    int nbr_castling_moves = 0;
    if (!in_check && rules->castling_allowed) {
        nbr_castling_moves = get_castling_moves(moves + counter, square_index, board, rules);
    }

    moves[counter + nbr_castling_moves].destination_square = -1;
}

static int get_castling_moves(struct Move moves[], int king_square_index, struct Square board[], struct Rules *rules) {
    enum PieceColor piece_color = board[king_square_index].piece.piece_color;
    int king_square[rules->dimensions];
    square_index_to_square(king_square_index, king_square, rules->dimensions, rules->board_shape);
    int destination_square[rules->dimensions];

    int counter = 0;
    // TODO both positive and negative direction
    for (int dim = 0; dim < rules->dimensions; ++dim) {
        for (int increment = -1; increment <= 1; increment += 2) {
            copy_int_array(king_square, destination_square, rules->dimensions);
            // go forward until edge of board, if own rook that hasn't moved after three or more squares -> compute castling move
            int distance = 1;
            int squares_passed[20];
            int counter2 = 0;
            while (true) {
                bool increment_legal = increment_dim_of_square_if_legal(destination_square, dim, increment, 
                                                                        rules->dimension_wrapping[dim], rules->board_shape[dim]);
                if (!increment_legal) {
                    break;
                }
                
                int destination_square_index = square_to_square_index(destination_square, rules->dimensions, rules->board_shape);
                squares_passed[counter2++] = destination_square_index;
                if (    board[destination_square_index].piece.piece_type == ROOK &&
                        board[destination_square_index].piece.piece_color == piece_color &&
                        distance >= 3) {
                    int gap1;
                    int gap2;
                    int gap3;
                    int gaps_sum = distance - 3;
                    gap3 = (gaps_sum + 2) / 3;          // division with 3 and rounding up
                    gap1 = (gaps_sum - gap3 + 1) / 2;   // division with 2 and rounding up
                    gap2 = gaps_sum - gap3 - gap1;

                    // TODO: check that squares the king has to pass through is not attacked
                    for (int i = 0; i < (gap1+1+gap2+1); ++i) {
                        // check that square not attacked
                    }

                    moves[counter].origin_square = king_square_index;
                    moves[counter].castling_rook_destination_square = squares_passed[gap1 + 1 - 1];
                    moves[counter].destination_square = squares_passed[gap1 + 1 + gap2 + 1 - 1];
                    moves[counter].castling_with_rook_on_square = destination_square_index;
                    ++counter;
                    break;
                } else if (board[destination_square_index].piece.piece_type != NULL_PIECE_TYPE) {
                    break;
                }

                ++distance;
            }
        }
    }
    return counter;
}

static void get_all_moves_to_unoccupied(struct Move moves[], int origin_square, struct Square board[], struct Rules *rules) {
    int board_length = 1;
    for (int i = 0; i < rules->dimensions; ++i) {
        board_length *= rules->board_shape[i];
    }

    // Variables
    int counter;
    int nbr_moves_before;
    for (counter = 0; moves[counter].destination_square != -1; ++counter) {}
    nbr_moves_before = counter;
    bool already_among_moves = false;

    for (int square = 0; square < board_length; ++square) {
        if (board[square].piece.piece_type == NULL_PIECE_TYPE && square != origin_square) {
            for (int i = 0; i < nbr_moves_before; ++i) {
                if (moves[i].destination_square == square) {
                    already_among_moves = true;
                    break;
                }
            }

            if (!already_among_moves) {
                moves[counter].origin_square = origin_square;
                moves[counter].destination_square = square;
                ++counter;
            }
            already_among_moves = false;
        }
    }
    moves[counter].destination_square = -1;
}

//static bool piece_color_in_check(enum PieceColor piece_color, int king_square, struct Square *board, struct Rules *rules) {
//    // Compute board length
//    int board_length = 1;
//    for (int i = 0; i < dimensions; ++i) {
//        board_length *= board_shape[i];
//    }
//
//    for (int square_index = 0; square_index < board_length && !result; ++square_index) {
//        enum PieceType piece_type = board[square_index].piece.piece_type;
//        enum PieceColor piece_color = board[square_index].piece.piece_color;
//        if (piece_color != origin_square_piece.piece_color) {
//            struct Move moves[MAX_MOVES_SINGLE_PIECE];
//            moves[0].destination_square = -1;
//            switch (piece_type) {
//                case PAWN:
//                    get_direct_pawn_captures(moves, square_index, board, rules);
//                    break;
//                case ROOK:
//                    get_rook_moves(moves, square_index, board, rules);
//                    break;
//                case KNIGHT:
//                    get_knight_moves(moves, square_index, board, rules);
//                    break;
//                case BISHOP:
//                    get_bishop_moves(moves, square_index, board, rules);
//                    break;
//                case KING:
//                    get_king_moves(moves, square_index, true, board, rules);    // no reason to consider castling moves
//                    break;
//                case QUEEN:
//                    get_queen_moves(moves, square_index, board, rules);
//                    break;
//                default:
//                    continue;
//            }
//
//            for (int i = 0; moves[i].destination_square != -1; ++i) {
//                if (moves[i].destination_square == own_king_square) {
//                    result = true;
//                    break;
//                }
//            }
//        }
//    }
//}

// works for own pieces other than the king
// moves and diagonal_pawn_moves
//static bool move_puts_own_king_in_check(struct Move move, struct Square *board, struct Rules *rules) {
//    // Variables
//    int origin_square = move.origin_square;
//    int destination_square = move.destination_square;
//    int dimensions = rules->dimensions;
//    int *board_shape = rules->board_shape;
//    bool result = false;
//
//    // If the move captures a king (enemy king or own king for diagonal pawn moves) the move is always fine
//    // what about invincible own king?
//    if (board[destination_square].piece.piece_type == KING) {
//        return false;
//    }
//
//    // Update board. Reverting this at the end of the function. Might reconsider this
//    struct Piece origin_square_piece = board[origin_square].piece;
//    struct Piece destination_square_piece = board[destination_square].piece;
//    board[origin_square].piece.piece_type = NULL_PIECE_TYPE;
//    board[destination_square].piece = origin_square_piece;
//
//    // Compute board length
//    int board_length = 1;
//    for (int i = 0; i < dimensions; ++i) {
//        board_length *= board_shape[i];
//    }
//
//    // Find own king. Assumes it exists
//    int own_king_square = -1;
//    for (int square_index = 0; square_index < board_length; ++square_index) {
//        if (board[square_index].piece.piece_color == origin_square_piece.piece_color && board[square_index].piece.piece_type == KING) {
//            own_king_square = square_index;
//            break;
//        }
//    }
//    if (own_king_square == -1) {
//        printf("bug, move_puts_own_king_in_check: this can't happen\n");
//        printf("piece_color, board[4], board[60] %d %d %d\n", origin_square_piece.piece_color, board[4].piece.piece_type, board[60].piece.piece_type);
//    }
//
//    // Evaluate all opponents moves. If they attack the king then set result to true and move on
//    for (int square_index = 0; square_index < board_length && !result; ++square_index) {
//        enum PieceType piece_type = board[square_index].piece.piece_type;
//        enum PieceColor piece_color = board[square_index].piece.piece_color;
//        if (piece_color != origin_square_piece.piece_color) {
//            struct Move moves[MAX_MOVES_SINGLE_PIECE];
//            moves[0].destination_square = -1;
//            switch (piece_type) {
//                case PAWN:
//                    get_direct_pawn_captures(moves, square_index, board, rules);
//                    break;
//                case ROOK:
//                    get_rook_moves(moves, square_index, board, rules);
//                    break;
//                case KNIGHT:
//                    get_knight_moves(moves, square_index, board, rules);
//                    break;
//                case BISHOP:
//                    get_bishop_moves(moves, square_index, board, rules);
//                    break;
//                case KING:
//                    get_king_moves(moves, square_index, true, board, rules);    // no reason to consider castling moves
//                    break;
//                case QUEEN:
//                    get_queen_moves(moves, square_index, board, rules);
//                    break;
//                default:
//                    continue;
//            }
//
//            for (int i = 0; moves[i].destination_square != -1; ++i) {
//                if (moves[i].destination_square == own_king_square) {
//                    result = true;
//                    break;
//                }
//            }
//        }
//    }
//    
//    // Reverting board to the state it was in
//    board[origin_square].piece.piece_type = origin_square_piece.piece_type;
//    board[destination_square].piece = destination_square_piece;
//
//    return result;
//}

//TODO reverse engineer this instead
//static bool piece_color_is_checkmated_old(enum PieceColor piece_color, struct GameState *game_state, struct Rules *rules) {
    //// Variables
    //struct Square *board = game_state->board;
    //int dimensions = rules->dimensions;
    //int *board_shape = rules->board_shape;
    
    //// Compute board length
    //int board_length = 1;
    //for (int i = 0; i < dimensions; ++i) {
        //board_length *= board_shape[i];
    //}

    //// Find own king. Assumes it exists
    //int own_king_square = -1;
    //for (int square_index = 0; square_index < board_length; ++square_index) {
        //if (board[square_index].piece.piece_type == KING && board[square_index].piece.piece_color == piece_color) {
            //own_king_square = square_index;
            //break;
        //}
    //}
    //if (own_king_square == -1) {
        ////this can happen in gravity chess
        ////printf("bug, piece_color_is_checkmate: this can't happen\n");
        ////printf("piece_color, board[4], board[60] %d %d %d\n", piece_color, board[4].piece.piece_type, board[60].piece.piece_type);
        //return true;
    //}

    //// Loop through opponents pieces moves to see if king in check
    //bool king_in_check = false;
    //for (int square_index = 0; square_index < board_length && !king_in_check; ++square_index) {
        //enum PieceType to_square_piece_type = board[square_index].piece.piece_type;
        //enum PieceColor to_square_piece_color = board[square_index].piece.piece_color;
        //if (to_square_piece_color == piece_color) {
            //continue;
        //}
        //struct Move moves[MAX_MOVES_SINGLE_PIECE];
        //moves[0].destination_square = -1;
        //switch (to_square_piece_type) {
            //printf("pt %d\n", to_square_piece_type);
            //case PAWN:
                //get_direct_pawn_captures(moves, square_index, board, rules);
                //break;
            //case ROOK:
                //get_rook_moves(moves, square_index, board, rules);
                //break;
            //case KNIGHT:
                //get_knight_moves(moves, square_index, board, rules);
                //break;
            //case BISHOP:
                //get_bishop_moves(moves, square_index, board, rules);
                //break;
            //case KING:
                //get_king_moves(moves, square_index, true, board, rules);    // no reason to consider castling moves
                //break;
            //case QUEEN:
                //get_queen_moves(moves, square_index, board, rules);
                //printf("queen moves: %d %d %d %d %d\n", moves[0].destination_square, moves[1].destination_square, moves[2].destination_square, moves[3].destination_square, moves[4].destination_square);
                //break;
            //default:
                //continue;
        //}

        //printf("oks %d\n", own_king_square);

        //for (int i = 0; moves[i].destination_square != -1; ++i) {
            //if (moves[i].destination_square == own_king_square) {
                //king_in_check = true;
                //break;
            //}
        //}
    //}
    //// If king not in check its not checkmate
    //if (!king_in_check) {
        //printf("test1\n");
        //return false;
    //}

    //// Loop through all own pieces, get their moves and run move_puts_own_king_in_check
    //for (int square_index = 0; square_index < board_length && king_in_check; ++square_index) {
        //enum PieceType square_piece_type = board[square_index].piece.piece_type;
        //enum PieceColor square_piece_color = board[square_index].piece.piece_color;
        //if (square_piece_color != piece_color) {
            //continue;
        //}
        //struct Move moves[MAX_MOVES_SINGLE_PIECE];
        //moves[0].destination_square = -1;
        //struct Move diagonal_pawn_moves[MAX_MOVES_SINGLE_PIECE];
        //switch (square_piece_type) {
            //printf("piece_color %d\n", square_piece_type);
            //case PAWN:
                //diagonal_pawn_moves[0].destination_square = -1;
                //get_pawn_moves(moves, diagonal_pawn_moves, square_index, board, game_state->last_moves_by_piece_color, rules);
                //break;
            //case ROOK:
                //get_rook_moves(moves, square_index, board, rules);
                //break;
            //case KNIGHT:
                //get_knight_moves(moves, square_index, board, rules);
                //break;
            //case BISHOP:
                //get_bishop_moves(moves, square_index, board, rules);
                //break;
            //case KING:
                //get_king_moves(moves, square_index, true, board, rules);
                //printf("king moves: %d %d %d %d %d\n", moves[0].destination_square, moves[1].destination_square, moves[2].destination_square, moves[3].destination_square, moves[4].destination_square);
                //break;
            //case QUEEN:
                //get_queen_moves(moves, square_index, board, rules);
                //break;
            //default:
                //continue;
        //}
        
        //// loop through moves and see if any of them gets out of check
        //for (int i = 0; moves[i].destination_square != -1; ++i) {
            //if (!move_puts_own_king_in_check(moves[i], board, rules)) {
                //return false;
            //}
        //}
    //}
    //return true;    // checkmate
//}

static bool square_is_attacked(int square_index, enum PieceColor attacked_piece_color, struct Square *board, struct Rules *rules) {
    // Variables
    int dimensions = rules->dimensions;
    int square_backup[dimensions];
    square_index_to_square(square_index, square_backup, dimensions, rules->board_shape);
    int square[dimensions];

    // Diagonal captures. One step - king or pawn with correct orientation
    for (int dim1 = 0; dim1 < dimensions; ++dim1) {
        for (int dim2 = dim1 + 1; dim2 < dimensions; ++dim2) {
            int increments[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
            for (int i = 0; i < 4; ++i) {
                copy_int_array(square_backup, square, dimensions);
                
                // One step - kings, bishops, queens, pawns with right orientation
                bool increment1_legal = increment_dim_of_square_if_legal(square, dim1, increments[i][0], 
                                                                    rules->dimension_wrapping[dim1], rules->board_shape[dim1]);
                bool increment2_legal = increment_dim_of_square_if_legal(square, dim2, increments[i][1], 
                                                                    rules->dimension_wrapping[dim2], rules->board_shape[dim2]);
                if (!increment1_legal || !increment2_legal) {
                    continue;
                }

                int destination_square_index = square_to_square_index(square, dimensions, rules->board_shape);
                enum PieceType piece_type = board[destination_square_index].piece.piece_type;
                enum PieceColor piece_color = board[destination_square_index].piece.piece_color;

                // todo: change this for team chess
                if (piece_type != NULL_PIECE_TYPE && piece_color == attacked_piece_color) { 
                    continue;   // piece of own piece_color, continue
                }

                if (piece_type == BISHOP || piece_type == QUEEN || (piece_type == KING && rules->dimensions > 2)) {
                    return true;    // square is attacked
                } else if (piece_type == PAWN) {
                    bool dim1_forward_dim = rules->is_forward_dimension[dim1];
                    bool dim2_forward_dim = rules->is_forward_dimension[dim2];
                    enum Direction piece_direction = board[destination_square_index].piece.direction;
                    if (dim1_forward_dim) {
                        if (    (increments[dim1][0] ==  1 && piece_direction == BACKWARDS && piece_color == PIECE_COLOR_WHITE) ||
                                (increments[dim1][0] ==  1 && piece_direction == FORWARDS  && piece_color == PIECE_COLOR_BLACK) ||
                                (increments[dim1][0] == -1 && piece_direction == FORWARDS  && piece_color == PIECE_COLOR_WHITE) ||
                                (increments[dim1][0] == -1 && piece_direction == BACKWARDS && piece_color == PIECE_COLOR_BLACK)) {
                            return true;
                        }
                    } else {
                        if (    (increments[dim1][0] ==  1 && piece_direction == LEFT  && piece_color == PIECE_COLOR_WHITE) ||
                                (increments[dim1][0] ==  1 && piece_direction == RIGHT && piece_color == PIECE_COLOR_BLACK) ||
                                (increments[dim1][0] == -1 && piece_direction == RIGHT && piece_color == PIECE_COLOR_WHITE) ||
                                (increments[dim1][0] == -1 && piece_direction == LEFT  && piece_color == PIECE_COLOR_BLACK)) {
                            return true;
                        }
                    }
                    if (dim2_forward_dim) {
                        if (    (increments[dim2][1] ==  1 && piece_direction == BACKWARDS && piece_color == PIECE_COLOR_WHITE) ||
                                (increments[dim2][1] ==  1 && piece_direction == FORWARDS  && piece_color == PIECE_COLOR_BLACK) ||
                                (increments[dim2][1] == -1 && piece_direction == FORWARDS  && piece_color == PIECE_COLOR_WHITE) ||
                                (increments[dim2][1] == -1 && piece_direction == BACKWARDS && piece_color == PIECE_COLOR_BLACK)) {
                            return true;
                        }
                    } else {
                        if (    (increments[dim2][1] ==  1 && piece_direction == LEFT  && piece_color == PIECE_COLOR_WHITE) ||
                                (increments[dim2][1] ==  1 && piece_direction == RIGHT && piece_color == PIECE_COLOR_BLACK) ||
                                (increments[dim2][1] == -1 && piece_direction == RIGHT && piece_color == PIECE_COLOR_WHITE) ||
                                (increments[dim2][1] == -1 && piece_direction == LEFT  && piece_color == PIECE_COLOR_BLACK)) {
                            return true;
                        }
                    }
                }

                // Steps beyond first step - only bishops and queens
                while (true) {
                    bool increment1_legal = increment_dim_of_square_if_legal(square, dim1, increments[i][0], 
                                                                    rules->dimension_wrapping[dim1], rules->board_shape[dim1]);
                    bool increment2_legal = increment_dim_of_square_if_legal(square, dim2, increments[i][1], 
                                                                    rules->dimension_wrapping[dim2], rules->board_shape[dim2]);
                    if (!increment1_legal || !increment2_legal) {
                        break;  // edge of board
                    }

                    int destination_square_index = square_to_square_index(square, dimensions, rules->board_shape);
                    enum PieceType piece_type = board[destination_square_index].piece.piece_type;
                    enum PieceColor piece_color = board[destination_square_index].piece.piece_color;

                    if (destination_square_index == square_index) {
                        break;      // we are back to original square
                    }
                    if (piece_type == NULL_PIECE_TYPE) {
                        continue;   // square empty, continue
                    }
                    if (piece_color == attacked_piece_color) {
                        break;      // piece of own piece_color
                    }
                    if (piece_type == BISHOP || piece_type == QUEEN) {
                        return true;    // square is attacked
                    }
                    break;      // opponents piece but not bishop or queen
                }
            }
        }
    }

    // Horizontal captures. One step - king
    for (int dim = 0; dim < dimensions; ++dim) {
        for (int direction = -1; direction <= 1; direction += 2) {
            copy_int_array(square_backup, square, dimensions);
                
            // One step - kings
            bool increment_legal = increment_dim_of_square_if_legal(square, dim, direction, 
                                                                    rules->dimension_wrapping[dim], rules->board_shape[dim]);
            if (!increment_legal) {
                continue;
            }
            int destination_square_index = square_to_square_index(square, dimensions, rules->board_shape);
            enum PieceType piece_type = board[destination_square_index].piece.piece_type;
            enum PieceColor piece_color = board[destination_square_index].piece.piece_color;

            if (piece_type != NULL_PIECE_TYPE && piece_color == attacked_piece_color) { 
                continue;   // piece of own piece_color, continue
            }
            if (piece_type == ROOK || piece_type == QUEEN || piece_type == KING) {
                return true;    // square is attacked
            }

            while (true) {
                bool increment_legal = increment_dim_of_square_if_legal(square, dim, direction, 
                                                                    rules->dimension_wrapping[dim], rules->board_shape[dim]);
                if (!increment_legal) {
                    break;
                }
                int destination_square_index = square_to_square_index(square, dimensions, rules->board_shape);
                enum PieceType piece_type = board[destination_square_index].piece.piece_type;
                enum PieceColor piece_color = board[destination_square_index].piece.piece_color;

                if (destination_square_index == square_index) {
                    break;      // we are back to original square
                } 
                if (piece_type == NULL_PIECE_TYPE) {
                    continue;   // square empty, continue
                } 
                if (piece_color == attacked_piece_color) {
                    break;      // piece of own piece color
                } 
                if (piece_type == BISHOP || piece_type == QUEEN) {
                    return true;    // square is attacked
                }
                break;      // opponents piece but not bishop or queen
            }
        }
    }

    // Knight captures
    for (int dim1 = 0; dim1 < dimensions; ++dim1) {
        for (int dim2 = 0; dim2 < dimensions; ++dim2) {
            if (dim1 == dim2) {
                continue;
            }
            int increments[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
            for (int i = 0; i < 4; ++i) {
                copy_int_array(square_backup, square, dimensions);
                
                // One step - kings, bishops, queens, pawns with right orientation
                bool increment1_legal = increment_dim_of_square_if_legal(square, dim1, increments[i][0], 
                                                                    rules->dimension_wrapping[dim1], rules->board_shape[dim1]);
                bool increment2_legal = increment_dim_of_square_if_legal(square, dim1, increments[i][0], 
                                                                    rules->dimension_wrapping[dim1], rules->board_shape[dim1]);
                bool increment3_legal = increment_dim_of_square_if_legal(square, dim2, increments[i][1], 
                                                                    rules->dimension_wrapping[dim2], rules->board_shape[dim2]);
                if (!increment1_legal || !increment2_legal || !increment3_legal) {
                    continue;
                }

                int destination_square_index = square_to_square_index(square, dimensions, rules->board_shape);
                enum PieceType piece_type = board[destination_square_index].piece.piece_type;
                enum PieceColor piece_color = board[destination_square_index].piece.piece_color;

                if (piece_type == KNIGHT && piece_color != attacked_piece_color) {
                    return true;
                }
            }
        }
    }
    return false;
}

//TODO
static bool player_is_checkmated(enum PieceColor piece_color, struct GameState *game_state, struct Rules *rules) {
    // Variables
    struct Square *board = game_state->board;
    int dimensions = rules->dimensions;
    int *board_shape = rules->board_shape;
    int board_length = 1;
    for (int i = 0; i < dimensions; ++i) {
        board_length *= board_shape[i];
    }

    // Find own king. Assumes it exists
    int own_king_square = -1;
    for (int square_index = 0; square_index < board_length; ++square_index) {
        if (board[square_index].piece.piece_type == KING && board[square_index].piece.piece_color == piece_color) {
            own_king_square = square_index;
            break;
        }
    }
    if (own_king_square == -1) {
        return true;    //this can happen in gravity chess (?) how?
    }

    // Check if own king attacked
    if (!square_is_attacked(own_king_square, piece_color, board, rules)) {
        return false;
    }

    // Check if all squares king can move to attacked
    struct Move king_moves[MAX_MOVES_SINGLE_PIECE];
    get_king_moves(king_moves, own_king_square, true, board, rules);

    for (int i = 0; king_moves[i].destination_square != -1; ++i) {
        if(!square_is_attacked(king_moves[i].destination_square, piece_color, board, rules)) {
            return false;
        }
    }
    return true;
}

static bool piece_color_king_captured(bool piece_color, struct GameState *game_state, struct Rules *rules) {
    // Variables
    struct Square *board = game_state->board;
    int dimensions = rules->dimensions;
    int *board_shape = rules->board_shape;
    
    // Compute board length
    int board_length = 1;
    for (int i = 0; i < dimensions; ++i) {
        board_length *= board_shape[i];
    }

    // Find own king. Assumes it exists
    for (int square_index = 0; square_index < board_length; ++square_index) {
        if (board[square_index].piece.piece_type == KING && board[square_index].piece.piece_color == piece_color) {
            return false;
        }
    }
    return true;
}

static bool piece_color_king_arrived(bool piece_color, struct GameState *game_state, struct Rules *rules) {
    int goal_square = rules->goal_square_by_piece_color[piece_color];
    enum PieceType piece_type_goal_square = game_state->board[goal_square].piece.piece_type;
    enum PieceType piece_color_goal_square = game_state->board[goal_square].piece.piece_color;
    if (piece_type_goal_square == KING && piece_color_goal_square == piece_color) {
        return true;
    }
    return false;
}

// If any one win condition is satisfied, the game is over
// returns true if game over. Whoever made the last move won
bool evaluate_win_conditions(struct Move last_move, struct GameState *game_state, struct Rules *rules) {
    enum PieceColor piece_color_last_move = game_state->board[last_move.destination_square].piece.piece_color;
    bool win_condition_satisfied = false;
    for (int i = 0; rules->win_conditions[i] != NULL_WIN_CONDITION && !win_condition_satisfied; ++i) {
        enum PieceColor piece_color_to_evaluate;
        switch (rules->win_conditions[i]) {
            case NULL_WIN_CONDITION:
                printf("bug, this can't happen");
                break;
            case CHECKMATE:
                if (piece_color_last_move == PIECE_COLOR_WHITE) {
                    piece_color_to_evaluate = PIECE_COLOR_BLACK;
                } else {
                    piece_color_to_evaluate = PIECE_COLOR_WHITE;
                }
                //TODO
                win_condition_satisfied = player_is_checkmated(piece_color_to_evaluate, game_state, rules);
                break;
            case KING_CAPTURED:
                if (piece_color_last_move == PIECE_COLOR_WHITE) {
                    piece_color_to_evaluate = PIECE_COLOR_BLACK;
                } else {
                    piece_color_to_evaluate = PIECE_COLOR_WHITE;
                }
                win_condition_satisfied = piece_color_king_captured(piece_color_to_evaluate, game_state, rules);
                break;
            case FLAG_CAPTURED:
                //TODO
                break;
            case KING_ARRIVED:
                win_condition_satisfied = piece_color_king_arrived(piece_color_last_move, game_state, rules);
                break;
            case ALL_PAWNS_CAPTURED:
                //TODO
                break;
            case PAWN_PROMOTED:
                //TODO
                break;
            case SIX_IN_A_ROW_DIAGONALLY:
                //TODO
                break;
            case EVERYTHING_CAPTURED:
                // TODO
                break;
            case NBR_OF_WIN_CONDITIONS:
                // just suppressing warning message
                break;
        }
    }
    if (!win_condition_satisfied) {
        return false;
    }

    if (piece_color_last_move == PIECE_COLOR_WHITE) {
        printf("Game over, white won\n");
    } else {
        printf("Game over, black won\n");
    }
    return true;
}

// returns true if the piece moving from that square should promote
bool evaluate_promotion(int square_index_from, int square_index_moving_to, struct GameState *game_state, struct Rules *rules) {
    // Return if no promotion rule
    if (rules->promotion_after_x_steps_in_single_dimension == -1) {
        return false;
    } 

    // Variables
    int dimensions = rules->dimensions;
    int *board_shape = rules->board_shape;
    enum PieceType piece_type = game_state->board[square_index_from].piece.piece_type;
    enum PieceColor piece_color = game_state->board[square_index_from].piece.piece_color;
    enum Direction direction = game_state->board[square_index_from].piece.direction;
    int square_moving_to[dimensions];
    square_index_to_square(square_index_moving_to, square_moving_to, dimensions, board_shape);

    // Check if this piece type can promote
    bool can_promote = false;
    for (int i = 0; rules->non_pawn_piece_types_that_can_promote[i] != NULL_PIECE_TYPE; ++i) {
        if (piece_type == rules->non_pawn_piece_types_that_can_promote[i]) {
            can_promote = true;
        }
    }
    if (piece_type != PAWN && !can_promote) {
        return false;
    }

    // Promotion when on last rank of every forward dimensions
    if (rules->promotion_after_x_steps_in_single_dimension == 0) {
        for (int dim = 0; dim < dimensions; ++dim) {
            // Continue if not forward dim for the piece. Non pawn pieces have forward dims as forward dims (no "direction")
            if ((rules->is_forward_dimension[dim] && piece_type == PAWN && (direction == RIGHT || direction == LEFT)) ||
                (!rules->is_forward_dimension[dim] && piece_type == PAWN && (direction == FORWARDS || direction == BACKWARDS)) ||
                (!rules->is_forward_dimension[dim] && piece_type != PAWN)) {
                continue;
            }

            bool positive_direction;
            if (piece_type == PAWN && (direction == FORWARDS || direction == RIGHT)) {
                positive_direction = true;
            } else if (piece_type == PAWN) {
                positive_direction = false;
            } else {
                positive_direction = true;
            }
            if (piece_color == PIECE_COLOR_BLACK) {
                positive_direction = (positive_direction) ? false : true;
            }

            // return if not on last rank
            if (positive_direction && square_moving_to[dim] != (board_shape[dim] - 1)) {
                return false;
            }
            if (!positive_direction && square_moving_to[dim] != 0) {
                return false;
            }
        }
        // was on last rank of all forward dimensions if we got to here
        return true;
    }

    // Promotion after specific number of steps
    // TODO
    return false;
}

static void evaluate_gravity(struct Square board[], struct Rules *rules) {
    int square[rules->dimensions];
    int square_copy[rules->dimensions];
    int gravity_dimension = rules->gravity_dimension;
    int gravity_direction = rules->gravity_direction;

    int board_length = 1;
    for (int i = 0; i < rules->dimensions; ++i) {
        board_length *= rules->board_shape[i];
    }

    for (int square_index = 0; square_index < board_length; ++square_index) {
        square_index_to_square(square_index, square, rules->dimensions, rules->board_shape);
        if (gravity_direction == 1 && square[gravity_dimension] == (rules->board_shape[gravity_dimension] - 1)) {
            // "upwards":
            for (; square[gravity_dimension] >= 0; --square[gravity_dimension]) {
                int square_index2 = square_to_square_index(square, rules->dimensions, rules->board_shape);
                if (board[square_index2].piece.piece_type == NULL_PIECE_TYPE) {
                    continue;
                }

                copy_int_array(square, square_copy, rules->dimensions);
                int square_index_prev = square_to_square_index(square_copy, rules->dimensions, rules->board_shape);
                
                ++square_copy[gravity_dimension];
                int square_index_this = square_to_square_index(square_copy, rules->dimensions, rules->board_shape);
                if (board[square_index_this].piece.piece_type != NULL_PIECE_TYPE) {
                    continue;
                }
                // back "down":
                for (; square_copy[gravity_dimension] < rules->board_shape[gravity_dimension]; ++square_copy[gravity_dimension]) {
                    square_index_this = square_to_square_index(square_copy, rules->dimensions, rules->board_shape);
                    if (board[square_index_this].piece.piece_type != NULL_PIECE_TYPE) {
                        board[square_index_prev].piece = board[square_index2].piece;
                        board[square_index2].piece.piece_type = NULL_PIECE_TYPE;
                        break;
                    } else if (square_copy[gravity_dimension] == (rules->board_shape[gravity_dimension] - 1)) {
                        board[square_index_this].piece = board[square_index2].piece;
                        board[square_index2].piece.piece_type = NULL_PIECE_TYPE;
                        break;
                    }
                    square_index_prev = square_index_this;
                }
            }
        } else if (gravity_direction == -1 && square[gravity_dimension] == 0) {
            // todo
            ;
        }
    }
}
