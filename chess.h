#ifndef CHESS_H
#define CHESS_H

#include <stdbool.h>

#define MAX_DIMENSIONS 14
#define MAX_SIDE_LENGTH 100
#define MAX_TOTAL_NBR_OF_SQUARES 19683    // 3^9. 2^14 also fine
#define MAX_NBR_OF_WIN_CONDITIONS 10
#define MAX_MOVES_SINGLE_PIECE 200
#define MAX_MOVES_PER_TURN 10

enum PieceType {
    NULL_PIECE_TYPE = 0, PAWN, ROOK, KNIGHT, BISHOP, KING, QUEEN,
};

enum PieceColor {
    NULL_PIECE_COLOR = -1,
    PIECE_COLOR_WHITE = 0, PIECE_COLOR_BLACK,   // first piece_color should be 0 for array indexing
    PIECE_COLOR_COUNT,                          // number of playable piece colors
    PIECE_COLOR_NEUTRAL
};

enum Direction {    // PieceDirection?
    FORWARDS, BACKWARDS, RIGHT, LEFT,   // RIGHT means forward in non-forward dimension
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
    //bool piece_colors_allowed[PIECE_COLOR_COUNT];                 // TODO
    //bool piece_colors_allowed_to_capture[PIECE_COLOR_COUNT];      // TODO
    struct Piece piece;
    bool white_flag;
    bool black_flag;
};

// Not evaluating promotion for every possible move because it requires computation, instead doing it when one specific move is 
// selected.
struct Move {
    int origin_square;
    int destination_square;
    int pawn_moved_past_square;             // -1 if pawn move but not two squares. Undefined if not pawn move
    bool en_passant_capture;                // true if en passant pawn move
    int castling_with_rook_on_square;       // -1 if not castling move
    int castling_rook_destination_square;
    //enum PieceType promotion_piece_type;
};

struct GameState {
    struct Square *board;   // 1D array representing nD board. Can be large -> malloc
    enum PieceColor whos_turn;
    int moves_made_this_turn;
    struct Move last_moves_by_piece_color[PIECE_COLOR_COUNT][MAX_MOVES_PER_TURN];   // defines legal en passant captures
};

enum WinCondition {
    NULL_WIN_CONDITION,
    CHECKMATE,
    KING_CAPTURED,
    FLAG_CAPTURED,      // Flag brought home to own flags starting square
    KING_ARRIVED,       // Own king arrived to opponents kings starting square
    ALL_PAWNS_CAPTURED,
    PAWN_PROMOTED,
    SIX_IN_A_ROW_DIAGONALLY,
    EVERYTHING_CAPTURED,
    //ALL_PIECES_ATTACKED
    NBR_OF_WIN_CONDITIONS,
};

struct Rules {
    int  dimensions;
    int  board_shape[MAX_DIMENSIONS];
    bool is_forward_dimension[MAX_DIMENSIONS];
    enum WinCondition win_conditions[MAX_NBR_OF_WIN_CONDITIONS+1];      // TODO go over this. Array of length NBR_OF_WIN_CONDITIONS
    bool castling_allowed;
    int  moves_per_turn_by_color[PIECE_COLOR_COUNT];
    bool same_piece_can_move_twice;
    bool king_invincible;
    bool king_allowed_to_capture;
    int  goal_square_by_piece_color[PIECE_COLOR_COUNT];
    bool dimension_wrapping[MAX_DIMENSIONS];
    int  promotion_after_x_steps_in_single_dimension;   // 0 means promotion on last rank of all forward dimensions. -1 no promotion
    enum PieceType non_pawn_piece_types_that_can_promote[3];
    bool simultaneous_moves;
    int  gravity_dimension;      // -1 means no gravity
    int  gravity_direction;      // -1 or 1
    bool can_move_anywhere_unoccupied;
    //bool pieces_two_lives;    // is this fun?
};

enum Variant {
    STANDARD_CHESS,
    CAPTURE_THE_FLAG_CHESS,
    LONG_RANGE_CHESS,               // two moves per turn, can be with same piece
    KING_MARCH_CHESS,
    STANDARD_10X10_CHESS,
    STANDARD_24X24_CHESS,
    STANDARD_DIAMOND_CHESS,
    SPARSE_CHESS,
    SWAP2_CHESS,                    // Gomoku rule to equalize the two sides
    TWO_MOVES_CHESS,                // with different pieces
    TEN_MOVES_CHESS,
    TWO_PLUS_ONE_MOVE_CHESS,        // make two moves for self then one for opponent
    RANDOM_STARTING_POSITION_CHESS,
    RANDOM_SYMETRICAL_STARTING_POSITION_CHESS,
    MORE_PAWNS_CHESS,
    GRAVITY_CHESS,                  // 90 degrees rotated, pieces fall down
    MORE_PAWNS_GRAVITY_CHESS,
    NO_RETREATING_MOVES_CHESS,
    START_AS_OPPONENT_CHESS,        // first five moves made as opponent
    ANYTHING_CAN_PROMOTE_CHESS,
    MOVE_TO_ANY_SQUARE_CHESS,
    CONTROL_OPPONENTS_KING_CHESS, //

    THREE_D_5X5X5_CHESS,
    THREE_D_8X8X8_CHESS,
    FOUR_D_3X3X3X3_V1_CHESS,
    FOUR_D_3X3X3X3_V2_CHESS,
    FOUR_D_3X3X3X3_V3_CHESS,
    FOUR_D_3X3X3X3_V4_CHESS,
    FOUR_D_4X4X4X4_V1_CHESS,
    FOUR_D_4X4X4X4_V2_CHESS,
    FOUR_D_8X8X8X8_V1_CHESS, //
    FOUR_D_8X8X8X8_V2_CHESS,
    FIVE_D_3X3X3X3X3_CHESS,
    SIX_D_2X2X2X2X2X2_CHESS,
    SIX_D_3X3X3X3X3X3_CHESS,
    WRAPPING_10X10_CHESS,
    WRAPPING_12X12_CHESS,
    WRAPPING_8X14_CHESS,

    THREE_D_SPHERE_CHESS,
    FOUR_D_SPHERE_CHESS,
    HOLLOW_CUBE_CHESS,
    DONUT_CHESS,

    SIMULTANEOUS_CHESS,
    TOWER_DEFENSE_CHESS,
    MONSTER_CHESS,

    CAPTURE_ALL_PAWNS_CHESS,
    CONNECT_SIX_DIAGONALLY_CHESS,
    RANK_SEVEN_AND_EIGHT_CHESS,
    KNIGHT_KING_CHESS,
    PAWN_PROMOTION_CHESS,
    PIECES_TWO_LIVES_CHESS,

    // some variant(s) where only one side allowed to be on specific part of board
    // some variant(s) where only one side allow to capture on specific part of board
    // some variant(s) with neutral pieces. Can be moved by both sides
};

// chess_initialize.c
bool initialize_rules_and_game_state (struct Rules *rules, struct GameState *GameState, enum Variant variant);
void terminate_game_state   (struct GameState *game_state);

// chess_logic.c
void get_moves              (struct Move moves[MAX_MOVES_SINGLE_PIECE], struct Move diagonal_pawn_moves[MAX_MOVES_SINGLE_PIECE], 
                             int square_index, struct GameState *game_state, struct Rules *rules);
struct Move validate_selected_move (int origin_square, int destination_square, struct Move possible_moves[], 
                             struct GameState *game_state, struct Rules *rules);
bool evaluate_promotion     (int square_index_from, int square_index_moving_to, struct GameState *game_state, struct Rules *rules);
void make_move              (struct Move move, enum PieceType promotion_piece_type, struct GameState *game_state, 
                             struct Rules *rules);
bool evaluate_win_conditions(struct Move last_move, struct GameState *game_state, struct Rules *rules);

// chess_utils.c
int  square_to_square_index (int square[], int dimensions, int board_shape[]);
void square_index_to_square (int square_index, int square[], int dimensions, int board_shape[]);
bool check_if_int_in_array  (int integer, int int_array[]);
bool check_if_move_among_moves(struct Move move, struct Move moves[]);
void copy_int_array         (int *from, int *to, int length);
#endif // CHESS_H

