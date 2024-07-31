#include "chess.h"

// used to check if move is among diagonal pawn moves
bool check_if_int_in_array(int integer, int int_array[]) {
    for (int i = 0; int_array[i] != -1; ++i) {
        if (integer == int_array[i]) {
            return true;
        }
    }
    return false;
}

bool check_if_move_among_moves(struct Move move, struct Move moves[]) {
    for (int i = 0; moves[i].destination_square != -1; ++i) {
        if (move.destination_square == moves[i].destination_square) {
            return true;
        }
    }
    return false;
}

int square_to_square_index(int square[], int dimensions, int board_shape[]) {
    int idx = 0;
    int factor = 1;
    for (int i = 0; i < dimensions; ++i) {
        idx = idx + square[i] * factor;
        factor *= board_shape[i];
    }
    return idx;
}

void square_index_to_square(int index, int square[], int dimensions, int board_shape[]) {
    int factors[dimensions];
    factors[0] = 1;
    factors[1] = board_shape[0];
    for (int dim = 2; dim < dimensions; ++dim) {
        factors[dim] = factors[dim-1] * board_shape[dim];
    }
    for (int dim = dimensions-1; dim >= 0; --dim) {
        int pos = index / factors[dim];
        square[dim] = pos;
        index = index - pos * factors[dim];
    }
}

void copy_int_array(int *from, int *to, int length){
    for (int i = 0; i < length; ++i) {
        to[i] = from[i];
    }
}

