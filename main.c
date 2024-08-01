#include <stdio.h>
#include "chess.h"
#include "graphics.h"
//#include <SDL.h>
#include "SDL.h"
#include <stdbool.h>

int main() {
    //enum Variant variant = STANDARD_10X10_CHESS;          //
    //enum Variant variant = LONG_RANGE_CHESS;              //
    //enum Variant variant = KING_MARCH_CHESS;
    //enum Variant variant = FOUR_D_4X4X4X4_V1_CHESS;       //
    //enum Variant variant = GRAVITY_CHESS;
    //enum Variant variant = TWO_MOVES_CHESS;
    enum Variant variant = TEN_MOVES_CHESS;
    //enum Variant variant = MOVE_TO_ANY_SQUARE_CHESS;
    
    //enum Variant variant = STANDARD_CHESS;
    //enum Variant variant = ANYTHING_CAN_PROMOTE_CHESS;
    //enum Variant variant = STANDARD_24X24_CHESS;
    //enum Variant variant = SPARSE_CHESS;
    //enum Variant variant = RANK_SEVEN_AND_EIGHT_CHESS;
    //enum Variant variant = MORE_PAWNS_CHESS;
    //enum Variant variant = MORE_PAWNS_GRAVITY_CHESS;

    //enum Variant variant = WRAPPING_8X14_CHESS;
    //enum Variant variant = WRAPPING_10X10_CHESS;
    //enum Variant variant = WRAPPING_12X12_CHESS;          //
    //enum Variant variant = THREE_D_5X5X5_CHESS;           //
    //enum Variant variant = FOUR_D_3X3X3X3_V1_CHESS;
    //enum Variant variant = FOUR_D_3X3X3X3_V2_CHESS;       //
    //enum Variant variant = FOUR_D_3X3X3X3_V3_CHESS;       //
    //enum Variant variant = FOUR_D_3X3X3X3_V4_CHESS;
    //enum Variant variant = FOUR_D_4X4X4X4_V2_CHESS;
    //enum Variant variant = FOUR_D_8X8X8X8_V2_CHESS;
    //enum Variant variant = SIX_D_2X2X2X2X2X2_CHESS;
    //enum Variant variant = SIX_D_3X3X3X3X3X3_CHESS;

    struct Rules rules;
    struct GameState game_state;
    if(!initialize_rules_and_game_state(&rules, &game_state, variant)){
        printf("initialize_rules_and_game_state failed\n");
        return -1;
    }

    struct GraphicsContext graphics_context;
    if(!initialize_graphics(&graphics_context, rules.dimensions, rules.board_shape)) {
        printf("initialize_graphics failed\n");
        return -1;
    }
    if(!load_graphics_media(graphics_context.textures, graphics_context.renderer)) {
        printf("load_graphics_media failed\n");
        return -1;
    }

    int selected_square_index = -1;
    struct Move moves[MAX_MOVES_SINGLE_PIECE];
    moves[0].destination_square = -1;
    struct Move diagonal_pawn_moves[MAX_MOVES_SINGLE_PIECE];
    diagonal_pawn_moves[0].destination_square = -1;
    SDL_Event event;
    int frameDelay = 16;    // 16*60=960

    bool quit = false;
    bool promotion_choice_flag = false;
    while (!quit) {
        int frameStart = SDL_GetTicks();
        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
                case SDL_QUIT: 
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_DOWN) {
                        ;
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button != SDL_BUTTON_LEFT) { break; }
                    if (promotion_choice_flag) {
                        // they just clicked on one of the choices, or outside it
                        continue;
                    }

                    int square_index = get_square_index_at_coordinates(event.button.x, event.button.y, 
                                                        graphics_context.graphics_board, graphics_context.graphics_board_length, 
                                                        graphics_context.square_width);
                    if (square_index == -1 || square_index == selected_square_index) {  // outside board -> reset highlighted squares
                        selected_square_index = -1;
                        moves[0].destination_square = -1;
                        diagonal_pawn_moves[0].destination_square = -1;
                    } else if (selected_square_index == -1) {   // on board -> highlight squares
                        selected_square_index = square_index;
                        get_moves(moves, diagonal_pawn_moves, square_index, &game_state, &rules);
                    } else {                                    // on board and squares already highlighted -> make move or highlight new square
                        get_moves(moves, diagonal_pawn_moves, selected_square_index, &game_state, &rules);
                        struct Move move = validate_selected_move(selected_square_index, square_index, moves, &game_state, &rules);
                        if (move.destination_square != -1) {
                            enum PieceType promote_to_piece_type;
                            if (evaluate_promotion(move.origin_square, move.destination_square, &game_state, &rules)) {
                                // todo: ask what they want to promote to
                                promote_to_piece_type = QUEEN;
                            } else {
                                promote_to_piece_type = NULL_PIECE_TYPE;
                            }

                            make_move(move, promote_to_piece_type, &game_state, &rules);
                            selected_square_index = -1;
                            moves[0].destination_square = -1;
                            diagonal_pawn_moves[0].destination_square = -1;
                            if (evaluate_win_conditions(move, &game_state, &rules)) {
                                quit = true;
                            }
                        } else {
                            selected_square_index = square_index;
                            get_moves(moves, diagonal_pawn_moves, selected_square_index, &game_state, &rules);
                        }
                    }
                    break;
            }
        }
        draw_board(&graphics_context, &game_state, selected_square_index, moves, diagonal_pawn_moves);

        int frameTime = SDL_GetTicks() - frameStart;
        if(frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    terminate_graphics(&graphics_context);
    terminate_game_state(&game_state);
    return 0;
}

