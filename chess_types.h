#ifndef CHESS_TYPES_H
#define CHESS_TYPES_H

#include <stdbool.h>

#define MAX_DIMENSIONS 14
#define MAX_SIDE_LENGTH 100
#define MAX_TOTAL_NBR_OF_SQUARES 19683    // 3^9. Is larger than 2^14
#define MAX_NBR_OF_WIN_CONDITIONS 10
#define MAX_MOVES_SINGLE_PIECE 200      // TODO figure ut maximum possible number of available moves on largest board
#define MAX_MOVES_PER_TURN 10

enum PieceType {
    NULL_PIECE_TYPE = 0, PAWN, ROOK, KNIGHT, BISHOP, KING, QUEEN,   // OUTSIDE_BOARD (?)
};

enum PieceColor {
    NULL_PIECE_COLOR = -1,
    PIECE_COLOR_WHITE = 0, PIECE_COLOR_BLACK,   // first piece_color should be 0 for array indexing
    PIECE_COLOR_COUNT,                          // number of playable piece colors
    PIECE_COLOR_NEUTRAL
};

enum Direction {    // PieceDirection?
    FORWARDS, BACKWARDS, RIGHT, LEFT,       // RIGHT means forward in non-forward dimension(s)
};

struct Move {
    int origin_square;
    int destination_square;
    int pawn_moved_past_square;             // Square that the pawn moved past. -1 if pawn move but not two squares. Not used if not pawn move.
    bool en_passant_capture;                // true if en passant pawn move
    int castling_with_rook_on_square;       // -1 if not castling move. square of the rook if castling move.
    int castling_rook_destination_square;   // destination square of the king
    //enum PieceType promotion_piece_type;
};

struct Piece {
    enum PieceType piece_type;
    enum PieceColor piece_color;
    enum Direction direction;   // relevant only for pawns
    bool has_moved;
    //bool has_been_captured_once;
};

struct Square {
    bool part_of_board;
    //bool piece_colors_allowed[PIECE_COLOR_COUNT];                 // TODO. Piece colors that are allowed on this square
    //bool piece_colors_allowed_to_capture[PIECE_COLOR_COUNT];      // TODO. Piece colors that are allowed to capture on this square
    struct Piece piece;
    bool white_flag;
    bool black_flag;
};

struct GameState {
    struct Square *board;   // 1D array representing nD board. Can be large -> malloc
    enum PieceColor whos_turn;
    int moves_made_this_turn;
    struct Move last_moves_by_piece_color[PIECE_COLOR_COUNT][MAX_MOVES_PER_TURN];   // defines legal en passant captures
};

enum WinCondition {
    NULL_WIN_CONDITION,
    CHECKMATE,                  // TODO
    KING_CAPTURED,
    FLAG_CAPTURED,          // Flag brought home to own flags starting square               // TODO
    KING_ARRIVED,           // Own king arrived to opponents kings starting square
    ALL_PAWNS_CAPTURED,         // TODO
    PAWN_PROMOTED,              // TODO
    SIX_IN_A_ROW_DIAGONALLY,    // TODO
    EVERYTHING_CAPTURED,        // TODO
    //ALL_PIECES_ATTACKED
    NBR_OF_WIN_CONDITIONS,
};

struct Rules {
    int  dimensions;
    int  board_shape[MAX_DIMENSIONS];
    bool is_forward_dimension[MAX_DIMENSIONS];
    enum WinCondition win_conditions[MAX_NBR_OF_WIN_CONDITIONS+1];      // TODO go over this. Array of length NBR_OF_WIN_CONDITIONS
    bool castling_allowed;      // probably done. double check everywhere
    int  moves_per_turn_by_color[PIECE_COLOR_COUNT];
    bool same_piece_can_move_twice;
    bool king_invincible;
    bool king_allowed_to_capture;
    int  goal_square_by_piece_color[PIECE_COLOR_COUNT];
    bool dimension_wrapping[MAX_DIMENSIONS];
    int  promotion_after_x_steps_in_single_dimension;   // TODO 0 means promotion on last rank of all forward dimensions. -1 no promotion
    enum PieceType non_pawn_piece_types_that_can_promote[3];
    bool simultaneous_moves;        // TODO
    int  gravity_dimension;         // -1 means no gravity
    int  gravity_direction;         // -1 or 1
    bool can_move_anywhere_unoccupied;
    //bool pieces_two_lives;          // is this fun?
};

enum Variant {
    STANDARD_CHESS,
    CAPTURE_THE_FLAG_CHESS,         // TODO
    LONG_RANGE_CHESS,               // two moves per turn, can be with same piece       // done
    KING_MARCH_CHESS,               // done
    STANDARD_10X10_CHESS,           // done
    STANDARD_24X24_CHESS,           // done
    STANDARD_DIAMOND_CHESS,         // TODO
    SPARSE_CHESS,                   // done
    SWAP2_CHESS,                    // TODO Gomoku rule to equalize the two sides
    TWO_MOVES_CHESS,                // done. with different pieces
    TEN_MOVES_CHESS,                // done
    TWO_PLUS_ONE_MOVE_CHESS,        // TODO make two moves for self then one for opponent
    RANDOM_STARTING_POSITION_CHESS,             // TODO. Should have check if position is withing x points according to engine, at certain depth.
    RANDOM_SYMETRICAL_STARTING_POSITION_CHESS,  // TODO. Piees only on own side, or anywhere on board?
    MORE_PAWNS_CHESS,               // done
    GRAVITY_CHESS,                  // 90 degrees rotated, pieces fall down. //TODO: make king moves legal when the king is going to fall out of check?
    MORE_PAWNS_GRAVITY_CHESS,
    NO_RETREATING_MOVES_CHESS,
    START_AS_OPPONENT_CHESS,        // first five moves made as opponent
    ANYTHING_CAN_PROMOTE_CHESS,     // done
    MOVE_TO_ANY_SQUARE_CHESS,       // done
    CONTROL_OPPONENTS_KING_CHESS,

    THREE_D_5X5X5_CHESS,        // done
    THREE_D_8X8X8_CHESS,            // TODO
    FOUR_D_3X3X3X3_V1_CHESS,        // done
    FOUR_D_3X3X3X3_V2_CHESS,        // done
    FOUR_D_3X3X3X3_V3_CHESS,        // done
    FOUR_D_3X3X3X3_V4_CHESS,        // done
    FOUR_D_4X4X4X4_V1_CHESS,        // done
    FOUR_D_4X4X4X4_V2_CHESS,        // done
    FOUR_D_8X8X8X8_V1_CHESS,        // TODO
    FOUR_D_8X8X8X8_V2_CHESS,        // done
    FIVE_D_3X3X3X3X3_CHESS,         // TODO
    SIX_D_2X2X2X2X2X2_CHESS,        // done
    SIX_D_3X3X3X3X3X3_CHESS,        // done
    WRAPPING_10X10_CHESS,           // done
    WRAPPING_12X12_CHESS,           // done
    WRAPPING_8X14_CHESS,            // done

    THREE_D_SPHERE_CHESS,           // TODO
    FOUR_D_SPHERE_CHESS,            // TODO
    HOLLOW_CUBE_CHESS,              // TODO
    DONUT_CHESS,                    // TODO

    SIMULTANEOUS_CHESS,             // TODO
    TOWER_DEFENSE_CHESS,            // TODO
    MONSTER_CHESS,                  // TODO

    CAPTURE_ALL_PAWNS_CHESS,        // TODO
    CONNECT_SIX_DIAGONALLY_CHESS,   // TODO
    RANK_SEVEN_AND_EIGHT_CHESS,     // done
    KNIGHT_KING_CHESS,              // TODO
    PAWN_PROMOTION_CHESS,           // TODO
    PIECES_TWO_LIVES_CHESS,         // TODO

    // some variant(s) where only one side allowed to be on specific part of board
    // some variant(s) where only one side allow to capture on specific part of board
    // some variant(s) with neutral pieces. Can be moved by both sides
};

#endif // CHESS_TYPES_H
