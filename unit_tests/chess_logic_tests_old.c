#include <stdio.h>
#include <stdbool.h>
#include "../chess.h"
#include "../chess_logic.c"

/// testing framework ///

#define TESTFAILED(X) testFailed(__FILE__, __LINE__, __func__, X)
void testFailed(const char* test_file, const int line_number, const char* test_function, const char* message)
{
  fprintf(stderr, "\nTEST FAILED: %s, file: %s, line: %i - %s\n", test_function, test_file, line_number, message);
};

/// testing framework ///


// Usually testing static functions via public api
// Occasionally testing static functions directly. Including c file and compiling only this one.
//

// assumes a1 ends with a -1
// TODO: make work if a1 shorter than a2
static bool int_arrays_same_content(int a1[], int a2[]) {
    int length1 = 0;
    int length2 = 0;
    for ( ; a1[length1] != -1; ++length1) {}
    for ( ; a2[length2] != -1; ++length2) {}
    int length = (length1 > length2) ? length1 : length2;
    
    bool matches[length];
    for(int i = 0; i < length; ++i) {
        matches[i] = false;
    }

    for (int i = 0; i < length; ++i) {
        for (int j = 0; j < length; ++j) {
            if (a1[i] == a2[j]) {
                matches[i] = true;
            }
        }
    }

    for(int i = 0; i < length; i++) {
        if (!matches[i]) {
            return false;
        }
    }
    return true;
}

void test_int_arrays_same_content() {
    printf("\n---%s---\n", __func__);

    int a1[5] = {1,2,3,4,-1};
    int a2[5] = {1,2,3,4,-1};
    if (!int_arrays_same_content(a1, a2)) {
        TESTFAILED("");
    } else {
        printf(".");
    }

    int a3[5] = {1,2,4,3,-1};
    if (!int_arrays_same_content(a1, a3)) {
        TESTFAILED("");
    } else {
        printf(".");
    }

    int a4[5] = {1,2,5,3,-1};
    if (int_arrays_same_content(a1, a4)) {
        TESTFAILED("");
    } else {
        printf(".");
    }

    // one array shorter
    int a5[5] = {1,2,3,-1};
    if (int_arrays_same_content(a1, a5)) {
        TESTFAILED("");
    } else {
        printf(".");
    }

    // the other shorter
    if (int_arrays_same_content(a5, a1)) {
        TESTFAILED("");
    } else {
        printf(".");
    }

    if (!int_arrays_same_content(a1, (int[]){1,2,4,3,-1})) {
        TESTFAILED("");
    } else {
        printf(".");
    }
}

void test_square_to_square_index() {
    printf("\n---%s---\n", __func__);
    int dimensions = 2;
    int board_shape[2] = {8,8};

    int square[2] = {0,0};
    int square_index = square_to_square_index(square, dimensions, board_shape);
    if (square_index != 0) {
        TESTFAILED("");
    } else {
        printf(".");
    }

    square[0] = 1;
    square[1] = 0;
    square_index = square_to_square_index(square, dimensions, board_shape);
    if (square_index != 1) {
        TESTFAILED("");
    } else {
        printf(".");
    }

    square[0] = 0;
    square[1] = 1;
    square_index = square_to_square_index(square, dimensions, board_shape);
    if (square_index != 8) {
        TESTFAILED("failed");
    } else {
        printf(".");
    }
}

void test_square_index_to_square() {
    printf("\n---%s---\n", __func__);
    int dimensions = 2;
    int board_shape[2] = {8,8};
    int square[2];

    int square_index = 0;
    square_index_to_square(square_index, square, dimensions, board_shape);
    if (square[0] != 0 || square[1] != 0) {
        TESTFAILED("");
    } else {
        printf(".");
    }

    square_index = 1;
    square_index_to_square(square_index, square, dimensions, board_shape);
    if (square[0] != 1 || square[1] != 0) {
        TESTFAILED("");
    } else {
        printf(".");
    }

    square_index = 8;
    square_index_to_square(square_index, square, dimensions, board_shape);
    if (square[0] != 0 || square[1] != 1) {
        TESTFAILED("");
    } else {
        printf(".");
    }

    square_index = 63;
    square_index_to_square(square_index, square, dimensions, board_shape);
    if (square[0] != 7 || square[1] != 7) {
        TESTFAILED("");
    } else {
        printf(".");
    }
}

void test_get_rook_moves() {
    printf("\n---%s---\n", __func__);

    enum Variant variant = STANDARD_CHESS;
    struct Rules rules;
    initialize_rules(&rules, variant);   // assumes these work (fix?)
    struct GameState game_state;
    initialize_game_state(&game_state, variant, rules.dimensions, rules.board_shape);

    int moves[MAX_NBR_OF_MOVES_SINGLE_PIECE];
    int diagonal_pawn_moves[MAX_NBR_OF_MOVES_SINGLE_PIECE];

    int square_index = 0;
    get_rook_moves(moves, square_index, game_state.board, &rules, PIECE_COLOR_WHITE);
    if (moves[0] != -1) {
        TESTFAILED("");
    } else {
        printf(".");
    }

    square_index = 8;
    get_rook_moves(moves, square_index, game_state.board, &rules, PIECE_COLOR_WHITE);
    if (moves[0] != 16 || moves[1] != 24 || moves[2] != 32 || moves[3] != 40 || moves[4] != 48 || moves[5] != -1) {
        TESTFAILED("should be 16, 24, 32, 40, 48, -1");
        printf("was: %d %d %d %d %d\n", moves[0], moves[1], moves[2], moves[3], moves[4]);
    } else {
        printf(".");
    }

    terminate_game_state(&game_state);
}

void test_get_bishop_moves() {
    printf("\n---%s---\n", __func__);

    enum Variant variant = STANDARD_CHESS;
    struct Rules rules;
    initialize_rules(&rules, variant);
    struct GameState game_state;
    initialize_game_state(&game_state, variant, rules.dimensions, rules.board_shape);

    int moves[MAX_NBR_OF_MOVES_SINGLE_PIECE];
    int square_index = 8;
    get_bishop_moves(moves, square_index, game_state.board, &rules, PIECE_COLOR_WHITE);
    int correct_moves[6] = {17,26,35,44,53,-1};
    if (!int_arrays_same_content(moves, correct_moves)) {
        printf("%d %d %d %d %d %d %d %d", moves[0], moves[1], moves[2], moves[3], moves[4], moves[5], moves[6], moves[7]);
        TESTFAILED("");
    } else {
        printf(".");
    }

    rules.dimension_wrapping[0] = true;
    square_index = 8;
    get_bishop_moves(moves, square_index, game_state.board, &rules, PIECE_COLOR_WHITE);
    int correct_moves2[11] = {17,26,35,44,53,23,30,37,44,51,-1};
    if (!int_arrays_same_content(moves, correct_moves2)) {
        printf("%d %d %d %d %d %d %d %d", moves[0], moves[1], moves[2], moves[3], moves[4], moves[5], moves[6], moves[7]);
        TESTFAILED("");
    } else {
        printf(".");
    }

}

void test_get_knight_moves() {
    printf("\n---%s---\n", __func__);

    enum Variant variant = STANDARD_CHESS;
    struct Rules rules;
    initialize_rules(&rules, variant);
    struct GameState game_state;
    initialize_game_state(&game_state, variant, rules.dimensions, rules.board_shape);

    int moves[MAX_NBR_OF_MOVES_SINGLE_PIECE];
    int square_index = 8;
    get_knight_moves(moves, square_index, game_state.board, &rules, PIECE_COLOR_WHITE);
    int correct_moves[3] = {18,25,-1};
    if (!int_arrays_same_content(moves, correct_moves)) {
        printf("%d %d %d %d %d %d %d %d", moves[0], moves[1], moves[2], moves[3], moves[4], moves[5], moves[6], moves[7]);
        TESTFAILED("");
    } else {
        printf(".");
    }

    rules.dimension_wrapping[0] = true;
    square_index = 8;
    get_bishop_moves(moves, square_index, game_state.board, &rules, PIECE_COLOR_WHITE);
    int correct_moves2[11] = {17,26,35,44,53,23,30,37,44,51,-1};
    if (!int_arrays_same_content(moves, correct_moves2)) {
        printf("%d %d %d %d %d %d %d %d", moves[0], moves[1], moves[2], moves[3], moves[4], moves[5], moves[6], moves[7]);
        TESTFAILED("");
    } else {
        printf(".");
    }

}

void test_get_rook_moves_wrapping() {
    printf("\n---%s---\n", __func__);

    enum Variant variant = STANDARD_CHESS;
    struct Rules rules;
    initialize_rules(&rules, variant);   // assumes these work (fix?)
    rules.dimension_wrapping[0] = true;
    rules.dimension_wrapping[1] = true;
    struct GameState game_state;
    initialize_game_state(&game_state, variant, rules.dimensions, rules.board_shape);

    int moves[MAX_NBR_OF_MOVES_SINGLE_PIECE];   // should test calling function several times with this array
    int diagonal_pawn_moves[MAX_NBR_OF_MOVES_SINGLE_PIECE];

    int square_index = 0;
    get_rook_moves(moves, square_index, game_state.board, &rules, PIECE_COLOR_WHITE);
    if (moves[0] != 56) {
        TESTFAILED("should be 56");
    } else {
        printf(".");
    }

    square_index = 56;
    get_rook_moves(moves, square_index, game_state.board, &rules, PIECE_COLOR_WHITE);
    // compare here
    if (!int_arrays_same_content(moves, (int[]){57,48,63,-1} )) {
        TESTFAILED("should be 57, 48, 63");
        printf("was: %d %d %d\n", moves[0], moves[1], moves[2]);
    } else {
        printf(".");
    }

    terminate_game_state(&game_state);
}

void test_get_pawn_moves() {
    printf("\n---%s---\n", __func__);

    enum Variant variant = STANDARD_CHESS;
    struct Rules rules;
    initialize_rules(&rules, variant);   // assumes these work (fix?)
    struct GameState game_state;
    initialize_game_state(&game_state, variant, rules.dimensions, rules.board_shape);

    int moves[MAX_NBR_OF_MOVES_SINGLE_PIECE];
    int diagonal_pawn_moves[MAX_NBR_OF_MOVES_SINGLE_PIECE];

    int square_where_en_passant_legal = -1;
    int square_en_passant_pawn_moved_to = -1;

    // moves, diagonal_pawn_moves
    int square_index = 8;
    get_pawn_moves(moves, diagonal_pawn_moves, square_index, game_state.board, square_where_en_passant_legal, square_en_passant_pawn_moved_to, &rules, PIECE_COLOR_WHITE);
    if (!int_arrays_same_content(moves, (int[]){16,24,-1} )) {
        TESTFAILED("1");
    } else if (!int_arrays_same_content(diagonal_pawn_moves, (int[]){17,-1} )) {
        TESTFAILED("2");
        printf("moves: %d %d %d %d %d\n", diagonal_pawn_moves[0], diagonal_pawn_moves[1], diagonal_pawn_moves[2], diagonal_pawn_moves[3], diagonal_pawn_moves[4]);
    } else {
        printf(".");
    }

    // already moved pawn
    square_index = 16;
    game_state.board[square_index].piece.has_moved = true;
    get_pawn_moves(moves, diagonal_pawn_moves, square_index, game_state.board, square_where_en_passant_legal, square_en_passant_pawn_moved_to, &rules, PIECE_COLOR_WHITE);
    if (!int_arrays_same_content(moves, (int[]){24,-1} )) {
        printf("moves: %d %d\n", moves[0], moves[1]);
        TESTFAILED("1");
    } else if (!int_arrays_same_content(diagonal_pawn_moves, (int[]){25,-1} )) {
        TESTFAILED("2");
        printf("diagonal_pawn_moves: %d %d %d %d %d\n", diagonal_pawn_moves[0], diagonal_pawn_moves[1], diagonal_pawn_moves[2], diagonal_pawn_moves[3], diagonal_pawn_moves[4]);
    } else {
        printf(".");
    }

    // captures
    square_index = 40;
    get_pawn_moves(moves, diagonal_pawn_moves, square_index, game_state.board, square_where_en_passant_legal, square_en_passant_pawn_moved_to, &rules, PIECE_COLOR_WHITE);
    if (!int_arrays_same_content(moves, (int[]){49,-1} )) {
        TESTFAILED("1");
    } else if (!int_arrays_same_content(diagonal_pawn_moves, (int[]){49,-1} )) {
        TESTFAILED("2");
        printf("moves: %d %d %d %d %d\n", diagonal_pawn_moves[0], diagonal_pawn_moves[1], diagonal_pawn_moves[2], diagonal_pawn_moves[3], diagonal_pawn_moves[4]);
    } else {
        printf(".");
    }
    
    // black pawns
    square_index = 52;  // e7
    get_pawn_moves(moves, diagonal_pawn_moves, square_index, game_state.board, square_where_en_passant_legal, square_en_passant_pawn_moved_to, &rules, PIECE_COLOR_BLACK);
    if (!int_arrays_same_content(moves, (int[]){44, 36,-1} )) {
        TESTFAILED("1");
    } else if (!int_arrays_same_content(diagonal_pawn_moves, (int[]){43, 45,-1} )) {
        TESTFAILED("2");
    } else {
        printf(".");
    }

    // black pawns, en passant
    square_where_en_passant_legal = 20;   // e3
    square_en_passant_pawn_moved_to = 28; // e4
    square_index = 27;  // d4
    game_state.board[square_index].piece.has_moved = true;
    get_pawn_moves(moves, diagonal_pawn_moves, square_index, game_state.board, square_where_en_passant_legal, square_en_passant_pawn_moved_to, &rules, PIECE_COLOR_BLACK);
    if (!int_arrays_same_content(moves, (int[]){19,20,-1} )) {
        TESTFAILED("1");
        printf("moves: %d %d %d %d %d\n", moves[0], moves[1], moves[2], moves[3], moves[4]);
    } else if (!int_arrays_same_content(diagonal_pawn_moves, (int[]){18,20,-1} )) {
        TESTFAILED("2");
        printf("moves: %d %d %d %d %d\n", diagonal_pawn_moves[0], diagonal_pawn_moves[1], diagonal_pawn_moves[2], diagonal_pawn_moves[3], diagonal_pawn_moves[4]);
    printf("moves: %d %d %d %d %d\n", diagonal_pawn_moves[0], diagonal_pawn_moves[1], diagonal_pawn_moves[2], diagonal_pawn_moves[3], diagonal_pawn_moves[4]);
    } else {
        printf(".");
    }

    // wrapping
    rules.dimension_wrapping[0] = true;
    square_index = 8;
    get_pawn_moves(moves, diagonal_pawn_moves, square_index, game_state.board, square_where_en_passant_legal, square_en_passant_pawn_moved_to, &rules, PIECE_COLOR_WHITE);
    if (!int_arrays_same_content(moves, (int[]){16,24,-1} )) {
        TESTFAILED("1");
    } else if (!int_arrays_same_content(diagonal_pawn_moves, (int[]){17,23,-1} )) {
        TESTFAILED("2");
        printf("moves: %d %d %d %d %d\n", diagonal_pawn_moves[0], diagonal_pawn_moves[1], diagonal_pawn_moves[2], diagonal_pawn_moves[3], diagonal_pawn_moves[4]);
    } else {
        printf(".");
    }

    // wrapping 2
    rules.dimension_wrapping[1] = true;
    square_index = 56;
    get_pawn_moves(moves, diagonal_pawn_moves, square_index, game_state.board, square_where_en_passant_legal, square_en_passant_pawn_moved_to, &rules, PIECE_COLOR_WHITE);
    if (!int_arrays_same_content(moves, (int[]){-1} )) {
        TESTFAILED("1");
    } else if (!int_arrays_same_content(diagonal_pawn_moves, (int[]){1,7,-1} )) {
        TESTFAILED("2");
        printf("moves: %d %d %d %d %d\n", diagonal_pawn_moves[0], diagonal_pawn_moves[1], diagonal_pawn_moves[2], diagonal_pawn_moves[3], diagonal_pawn_moves[4]);
    } else {
        printf(".");
    }

    terminate_game_state(&game_state);
}
void test_increment_dim_of_square_if_legal() {
    printf("\n---%s---\n", __func__);
    int square[2] = {0,0};   // A1
    int dim = 0;
    int increment = 1;
    bool dimension_wrapping = false;
    int side_length = 8;
    int still_on_board = increment_dim_of_square_if_legal(square, dim, increment, dimension_wrapping, side_length);
    if (square[0] != 1) {
        TESTFAILED("should be 1");
    } else {
        printf(".");
    }
    
    square[0] = 4;
    square[1] = 7;
    still_on_board = increment_dim_of_square_if_legal(square, 1, 1, true, 8);
    if (square[1] != 0) {
        TESTFAILED("should be 0");
    } else {
        printf(".");
    }

    square[0] = 4;
    square[1] = 7;
    still_on_board = increment_dim_of_square_if_legal(square, 1, 1, false, 8);
    if (still_on_board || square[1] != 7) {
        TESTFAILED("should be outside board and no increment should been made");
    } else {
        printf(".");
    }

    square[0] = 7;
    square[1] = 7;
    still_on_board = increment_dim_of_square_if_legal(square, 0, 1, true, 8);
    if (!still_on_board || square[0] != 0) {
        TESTFAILED("should be outside board");
    } else {
        printf(".");
    }

    still_on_board = increment_dim_of_square_if_legal(square, 1, 1, true, 8);
    if (!still_on_board || square[1] != 0) {
        TESTFAILED("should be outside board");
    } else {
        printf(".");
    }

    square[0] = 0;
    square[1] = 1;
    still_on_board = increment_dim_of_square_if_legal(square, 1, 1, true, 8);
    if (!still_on_board || square[1] != 2) {
        TESTFAILED("should be 2");
    } else {
        printf(".");
    }

    still_on_board = increment_dim_of_square_if_legal(square, 1, 1, true, 8);
    if (!still_on_board || square[1] != 3) {
        TESTFAILED("should be 3");
    } else {
        printf(".");
    }
}

void test_get_queen_moves() {
    printf("\n---%s---\n", __func__);

    enum Variant variant = STANDARD_CHESS;
    struct Rules rules;
    initialize_rules(&rules, variant);
    struct GameState game_state;
    initialize_game_state(&game_state, variant, rules.dimensions, rules.board_shape);

    int moves[MAX_NBR_OF_MOVES_SINGLE_PIECE];
    int square_index = 8;
    get_queen_moves(moves, square_index, game_state.board, &rules, PIECE_COLOR_WHITE);
    int correct_moves[11] = {17,26,35,44,53,16,24,32,40,48,-1};
    if (!int_arrays_same_content(moves, correct_moves)) {
        printf("%d %d %d %d %d %d %d %d %d %d %d", moves[0], moves[1], moves[2], moves[3], moves[4], moves[5], moves[6], moves[7], moves[8], moves[9], moves[10]);
        TESTFAILED("");
    } else {
        printf(".");
    }
}

void test_get_king_moves() {
    printf("\n---%s---\n", __func__);

    enum Variant variant = STANDARD_CHESS;
    struct Rules rules;
    initialize_rules(&rules, variant);
    struct GameState game_state;
    initialize_game_state(&game_state, variant, rules.dimensions, rules.board_shape);

    int moves[MAX_NBR_OF_MOVES_SINGLE_PIECE];
    int square_index = 8;
    get_king_moves(moves, square_index, game_state.board, &rules, PIECE_COLOR_WHITE);
    int correct_moves[11] = {16,17,-1};
    if (!int_arrays_same_content(moves, correct_moves)) {
        printf("%d %d %d %d %d %d %d %d %d %d %d", moves[0], moves[1], moves[2], moves[3], moves[4], moves[5], moves[6], moves[7], moves[8], moves[9], moves[10]);
        TESTFAILED("");
    } else {
        printf(".");
    }

    rules.dimension_wrapping[0] = true;
    square_index = 8;
    get_king_moves(moves, square_index, game_state.board, &rules, PIECE_COLOR_WHITE);
    int correct_moves2[] = {16,17,23,-1};
    if (!int_arrays_same_content(moves, correct_moves2)) {
        printf("\n%d %d %d %d %d %d %d %d %d %d %d", moves[0], moves[1], moves[2], moves[3], moves[4], moves[5], moves[6], moves[7], moves[8], moves[9], moves[10]);
        TESTFAILED("");
    } else {
        printf(".");
    }

    rules.dimension_wrapping[0] = true;
    rules.dimension_wrapping[1] = true;
    square_index = 56;
    get_king_moves(moves, square_index, game_state.board, &rules, PIECE_COLOR_BLACK);
    int correct_moves3[] = {0,1,7,-1};
    if (!int_arrays_same_content(moves, correct_moves3)) {
        printf("\n%d %d %d %d %d %d %d %d %d %d %d", moves[0], moves[1], moves[2], moves[3], moves[4], moves[5], moves[6], moves[7], moves[8], moves[9], moves[10]);
        TESTFAILED("");
    } else {
        printf(".");
    }
}

void test_move_puts_own_king_in_check() {
    printf("\n---%s---\n", __func__);
    enum Variant variant = STANDARD_CHESS;
    struct Rules rules;
    initialize_rules(&rules, variant);
    struct GameState game_state;
    initialize_game_state(&game_state, variant, rules.dimensions, rules.board_shape);

    bool result;
    result = move_puts_own_king_in_check(11, 27, game_state.board, &rules);  // it should not
    if (result) {
        TESTFAILED("");
    } else {
        printf(".");
    }
    
    make_move(58, 32, false, NULL_PIECE_TYPE, &game_state, &rules);
    result = move_puts_own_king_in_check(11, 27, game_state.board, &rules);  // it should
    if (!result || game_state.board[11].piece.piece_type != PAWN || game_state.board[27].piece.piece_type != NULL_PIECE_TYPE) {
        TESTFAILED("");
    } else {
        printf(".");
    }
}

void test_evaluate_promotion() {
    printf("\n---%s---\n", __func__);
    enum Variant variant = STANDARD_CHESS;
    struct Rules rules;
    initialize_rules(&rules, variant);
    struct GameState game_state;
    initialize_game_state(&game_state, variant, rules.dimensions, rules.board_shape);

    if (evaluate_promotion(8, 16, &game_state, &rules)) {
        TESTFAILED("should be false");
    } else {
        printf(".");
    }

    game_state.board[48].piece.piece_type = PAWN;
    game_state.board[48].piece.piece_color = PIECE_COLOR_WHITE;
    game_state.board[56].piece.piece_type = NULL_PIECE_TYPE;

    if (!evaluate_promotion(48, 56, &game_state, &rules)) {
        TESTFAILED("should be true. promotion move");
    } else {
        printf(".");
    }
}

void test_get_moves_king_invincible() {
    printf("\n---%s---\n", __func__);
    enum Variant variant = STANDARD_CHESS;
    struct Rules rules;
    initialize_rules(&rules, variant);
    struct GameState game_state;
    initialize_game_state(&game_state, variant, rules.dimensions, rules.board_shape);

    // King not invincible
    int square_index = 32;
    game_state.board[4].piece.piece_type = NULL_PIECE_TYPE;
    game_state.board[square_index].piece.piece_color = PIECE_COLOR_WHITE;
    game_state.board[square_index].piece.piece_type = KING;

    int moves[MAX_NBR_OF_MOVES_SINGLE_PIECE];
    int diagonal_pawn_moves[MAX_NBR_OF_MOVES_SINGLE_PIECE];
    get_moves(moves, diagonal_pawn_moves, square_index, &game_state, &rules);
    int correct_moves[] = {33,24,25,-1};
    if (!int_arrays_same_content(moves, correct_moves)) {
        printf("%d %d %d %d %d %d %d\n", moves[0], moves[1], moves[2], moves[3], moves[4], moves[5], moves[6]);
        TESTFAILED("");
    } else {
        printf(".");
    }

    // King invincible
    rules.king_invincible = true;
    rules.king_allowed_to_capture = false;

    get_moves(moves, diagonal_pawn_moves, square_index, &game_state, &rules);
    int correct_moves2[] = {40,41,33,24,25,-1};
    if (!int_arrays_same_content(moves, correct_moves2)) {
        printf("%d %d %d %d %d %d %d %d %d %d %d", moves[0], moves[1], moves[2], moves[3], moves[4], moves[5], moves[6], moves[7], moves[8], moves[9], moves[10]);
        TESTFAILED("");
    } else {
        printf(".");
    }

    // King not invincible, capture opponents king
    rules.king_invincible = false;
    rules.king_allowed_to_capture = true;

    square_index = 59;
    game_state.board[square_index].piece.piece_color = PIECE_COLOR_WHITE;
    game_state.board[square_index].piece.piece_type = ROOK;

    get_moves(moves, diagonal_pawn_moves, square_index, &game_state, &rules);
    int correct_moves3[] = {58,60,51,-1};
    if (!int_arrays_same_content(moves, correct_moves3)) {
        printf("%d %d %d %d %d %d %d\n", moves[0], moves[1], moves[2], moves[3], moves[4], moves[5], moves[6]);
        TESTFAILED("");
    } else {
        printf(".");
    }

    // King invincible, capture opponents king
    rules.king_invincible = true;
    rules.king_allowed_to_capture = false;

    square_index = 59;
    game_state.board[square_index].piece.piece_color = PIECE_COLOR_WHITE;
    game_state.board[square_index].piece.piece_type = ROOK;
    //printf("black king on square 60: %d\n", game_state.board[60].piece.piece_type);

    get_moves(moves, diagonal_pawn_moves, square_index, &game_state, &rules);
    int correct_moves4[] = {58,51,-1};
    if (!int_arrays_same_content(moves, correct_moves4)) {
        printf("%d %d %d %d %d %d %d\n", moves[0], moves[1], moves[2], moves[3], moves[4], moves[5], moves[6]);
        TESTFAILED("");
    } else {
        printf(".");
    }

    // King not allowed to capture
    rules.king_invincible = true;
    rules.king_allowed_to_capture = false;

    square_index = 48;
    game_state.board[square_index].piece.piece_color = PIECE_COLOR_WHITE;
    game_state.board[square_index].piece.piece_type = KING;

    get_moves(moves, diagonal_pawn_moves, square_index, &game_state, &rules);
    int correct_moves5[] = {40,41,-1};
    if (!int_arrays_same_content(moves, correct_moves5)) {
        printf("%d %d %d %d %d %d %d\n", moves[0], moves[1], moves[2], moves[3], moves[4], moves[5], moves[6]);
        TESTFAILED("");
    } else {
        printf(".");
    }
}

int main() {
    test_int_arrays_same_content();
    test_square_to_square_index();
    test_square_index_to_square();
    test_get_rook_moves();
    test_get_rook_moves_wrapping();
    test_get_pawn_moves();
    test_get_bishop_moves();
    test_get_knight_moves();
    test_increment_dim_of_square_if_legal();
    test_get_queen_moves();
    test_get_king_moves();
    test_move_puts_own_king_in_check();
    test_evaluate_promotion();
    test_get_moves_king_invincible();

    printf("\n");
}
