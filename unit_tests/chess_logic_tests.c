#include <stdio.h>
#include <stdbool.h>
#include "../chess.h"
#include "../chess_logic.c"     // include c file here and compile only this file

/// TESTING FRAMEWORK ///
#define TESTFAILED(X) testFailed(__FILE__, __LINE__, __func__, X)
void testFailed(const char* test_file, const int line_number, const char* test_function, const char* message)
{
  fprintf(stderr, "\nTEST FAILED: %s, file: %s, line: %i - %s\n", test_function, test_file, line_number, message);
}

#define TEST_TRUTH(X)    test_truth(__FILE__, __LINE__, __func__, X, "")
//#define TEST_TRUTH(X, Y) test_truth(__FILE__, __LINE__, __func__, X, Y)
//#define TEST_TRUTH(...) test_truth(__FILE__, __LINE__, __func__, __VA_ARGS__)
void test_truth(const char* test_file, const int test_line_number, const char* test_function, bool result, const char* message) {
    if (result) {
        printf(".");
    } else {
        printf("\n");
        fprintf(stderr, "TEST FAILED: %s, file: %s, line: %i - %s\n", test_function, test_file, test_line_number, message);
    }
}

// make this work if switching to C++ compiler
void test_truth2(const char* test_file, const int test_line_number, const char* test_function, bool result) {
    test_truth(test_file, test_line_number, test_function, result, "");
}
/// TESTING FRAMEWORK ///


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

// Fundamental tests - tested function is used by other tests
void test_square_to_square_index() {
    printf("\n---%s---\n", __func__);
    int dimensions = 2;
    int board_shape[2] = {8,8};

    int square[2] = {0,0};
    int square_index = square_to_square_index(square, dimensions, board_shape);
    if (square_index != 0) {
        TESTFAILED("");
        printf("FUNDAMENTAL TEST FAILED - FIX THIS FIRST\n");
    } else {
        printf(".");
    }

    square[0] = 1;
    square[1] = 0;
    square_index = square_to_square_index(square, dimensions, board_shape);
    if (square_index != 1) {
        TESTFAILED("");
        printf("FUNDAMENTAL TEST FAILED - FIX THIS FIRST\n");
    } else {
        printf(".");
    }

    square[0] = 0;
    square[1] = 1;
    square_index = square_to_square_index(square, dimensions, board_shape);
    if (square_index != 8) {
        TESTFAILED("failed");
        printf("FUNDAMENTAL TEST FAILED - FIX THIS FIRST\n");
    } else {
        printf(".");
    }
}

// Fundamental tests - tested function is used by other tests
void test_square_index_to_square() {
    printf("\n---%s---\n", __func__);
    int dimensions = 2;
    int board_shape[2] = {8,8};
    int square[2];

    int square_index = 0;
    square_index_to_square(square_index, square, dimensions, board_shape);
    if (square[0] != 0 || square[1] != 0) {
        TESTFAILED("");
        printf("FUNDAMENTAL TEST FAILED - FIX THIS FIRST\n");
    } else {
        printf(".");
    }

    square_index = 1;
    square_index_to_square(square_index, square, dimensions, board_shape);
    if (square[0] != 1 || square[1] != 0) {
        TESTFAILED("");
        printf("FUNDAMENTAL TEST FAILED - FIX THIS FIRST\n");
    } else {
        printf(".");
    }

    square_index = 8;
    square_index_to_square(square_index, square, dimensions, board_shape);
    if (square[0] != 0 || square[1] != 1) {
        TESTFAILED("");
        printf("FUNDAMENTAL TEST FAILED - FIX THIS FIRST\n");
    } else {
        printf(".");
    }

    square_index = 63;
    square_index_to_square(square_index, square, dimensions, board_shape);
    if (square[0] != 7 || square[1] != 7) {
        TESTFAILED("");
        printf("FUNDAMENTAL TEST FAILED - FIX THIS FIRST\n");
    } else {
        printf(".");
    }
}

void test_square_is_attacked() {
    printf("\n---%s---\n", __func__);
    struct Rules rules;
    struct GameState game_state;
    initialize_rules_and_game_state(&rules, &game_state, STANDARD_CHESS);

    // attacked by pawn diagonally
    int square[2] = {0,5};
    int square_index = square_to_square_index(square, rules.dimensions, rules.board_shape);
    bool square_attacked = square_is_attacked(square_index, PIECE_COLOR_WHITE, game_state.board, &rules);
    TEST_TRUTH(square_attacked);

    // attacked by bishop, queen or king (diagonally, one step)
    square[0] = 1;
    square[1] = 6;
    square_index = square_to_square_index(square, rules.dimensions, rules.board_shape);
    square_attacked = square_is_attacked(square_index, PIECE_COLOR_WHITE, game_state.board, &rules);
    TEST_TRUTH(square_attacked);
    square[0] = 2;
    square[1] = 6;
    square_index = square_to_square_index(square, rules.dimensions, rules.board_shape);
    square_attacked = square_is_attacked(square_index, PIECE_COLOR_WHITE, game_state.board, &rules);
    TEST_TRUTH(square_attacked);
    square[0] = 3;
    square[1] = 6;
    square_index = square_to_square_index(square, rules.dimensions, rules.board_shape);
    square_attacked = square_is_attacked(square_index, PIECE_COLOR_WHITE, game_state.board, &rules);
    TEST_TRUTH(square_attacked);

    // square that is not attacked
    square[0] = 4;
    square[1] = 1;
    square_index = square_to_square_index(square, rules.dimensions, rules.board_shape);
    square_attacked = square_is_attacked(square_index, PIECE_COLOR_WHITE, game_state.board, &rules);
    TEST_TRUTH(!square_attacked);
    
    // attacked by rook, queen or king horizontally one step
    square[0] = 0;
    square[1] = 6;
    square_index = square_to_square_index(square, rules.dimensions, rules.board_shape);
    square_attacked = square_is_attacked(square_index, PIECE_COLOR_WHITE, game_state.board, &rules);
    TEST_TRUTH(square_attacked);
    square[0] = 2;
    square[1] = 7;
    square_index = square_to_square_index(square, rules.dimensions, rules.board_shape);
    square_attacked = square_is_attacked(square_index, PIECE_COLOR_WHITE, game_state.board, &rules);
    TEST_TRUTH(square_attacked);
    square[0] = 5;
    square[1] = 7;
    square_index = square_to_square_index(square, rules.dimensions, rules.board_shape);
    square_attacked = square_is_attacked(square_index, PIECE_COLOR_WHITE, game_state.board, &rules);
    TEST_TRUTH(square_attacked);

    // remove pawns on d7 and e7
    square[0] = 3;
    square[1] = 6;
    square_index = square_to_square_index(square, rules.dimensions, rules.board_shape);
    game_state.board[square_index].piece.piece_type = NULL_PIECE_TYPE;
    game_state.board[square_index+1].piece.piece_type = NULL_PIECE_TYPE;
    // attacked by queen several steps horizontally or bishop several steps diagonally
    square[0] = 3;
    square[1] = 1;
    square_index = square_to_square_index(square, rules.dimensions, rules.board_shape);
    square_attacked = square_is_attacked(square_index, PIECE_COLOR_WHITE, game_state.board, &rules);
    TEST_TRUTH(square_attacked);
    square[0] = 1;  // b4
    square[1] = 3;
    square_index = square_to_square_index(square, rules.dimensions, rules.board_shape);
    square_attacked = square_is_attacked(square_index, PIECE_COLOR_WHITE, game_state.board, &rules);
    TEST_TRUTH(square_attacked);

    // H2 should not be attacked
    square[0] = 7;
    square[1] = 1;
    square_index = square_to_square_index(square, rules.dimensions, rules.board_shape);
    square_attacked = square_is_attacked(square_index, PIECE_COLOR_WHITE, game_state.board, &rules);
    TEST_TRUTH(!square_attacked);

    // put black knight on d4 and check that squares are attacked
    square[0] = 3;
    square[1] = 3;
    square_index = square_to_square_index(square, rules.dimensions, rules.board_shape);
    game_state.board[square_index].piece.piece_type = KNIGHT;
    game_state.board[square_index].piece.piece_color = PIECE_COLOR_BLACK;
    // b3 and c2 should now be attacked, but not for example c3
    square[0] = 1;
    square[1] = 2;
    square_index = square_to_square_index(square, rules.dimensions, rules.board_shape);
    square_attacked = square_is_attacked(square_index, PIECE_COLOR_WHITE, game_state.board, &rules);
    TEST_TRUTH(square_attacked);
    square[0] = 2;
    square[1] = 1;
    square_index = square_to_square_index(square, rules.dimensions, rules.board_shape);
    square_attacked = square_is_attacked(square_index, PIECE_COLOR_WHITE, game_state.board, &rules);
    TEST_TRUTH(square_attacked);
    square[0] = 2;
    square[1] = 2;
    square_index = square_to_square_index(square, rules.dimensions, rules.board_shape);
    square_attacked = square_is_attacked(square_index, PIECE_COLOR_WHITE, game_state.board, &rules);
    TEST_TRUTH(!square_attacked);

    // pawn moving sideways att
    terminate_game_state(&game_state);
    initialize_rules_and_game_state(&rules, &game_state, STANDARD_CHESS);

    // attacked by pawn moving sideways
    square[0] = 4;
    square[1] = 4;
    square_index = square_to_square_index(square, rules.dimensions, rules.board_shape);
    game_state.board[square_index].piece.piece_type = PAWN;
    game_state.board[square_index].piece.piece_color = PIECE_COLOR_BLACK;
    game_state.board[square_index].piece.direction = RIGHT;
    square[0] = 3;
    square[1] = 3;
    square_index = square_to_square_index(square, rules.dimensions, rules.board_shape);
    square_attacked = square_is_attacked(square_index, PIECE_COLOR_WHITE, game_state.board, &rules);
    TEST_TRUTH(square_attacked);

    // TODO - test in more than two dimensions, test for wrapping boards, non rectangle board shapes (knight moves)
}


void test_player_is_checkmated() {
    printf("\n---%s---\n", __func__);
    struct Rules rules;
    struct GameState game_state;
    initialize_rules_and_game_state(&rules, &game_state, STANDARD_CHESS);

    TEST_TRUTH(!player_is_checkmated(PIECE_COLOR_WHITE, &game_state, &rules));
    TEST_TRUTH(!player_is_checkmated(PIECE_COLOR_BLACK, &game_state, &rules));

    // set up checkmate for white. The mate in four
    int square[2] = {5,6};
    int square_index = square_to_square_index(square, rules.dimensions, rules.board_shape);
    game_state.board[square_index].piece.piece_type = QUEEN;
    game_state.board[square_index].piece.piece_color = PIECE_COLOR_WHITE;
    square[0] = 2;
    square[1] = 3;
    square_index = square_to_square_index(square, rules.dimensions, rules.board_shape);
    game_state.board[square_index].piece.piece_type = BISHOP;
    game_state.board[square_index].piece.piece_color = PIECE_COLOR_WHITE;
    TEST_TRUTH(!player_is_checkmated(PIECE_COLOR_WHITE, &game_state, &rules));
    TEST_TRUTH(player_is_checkmated(PIECE_COLOR_BLACK, &game_state, &rules));
}

int main() {
    // "fundamental" tests
    test_int_arrays_same_content();
    test_square_to_square_index();
    test_square_index_to_square();

    // static functions tests
    test_square_is_attacked();
    test_player_is_checkmated();

    printf("\n");
}
