#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "chess.h"
#include "SDL.h"

enum Texture {
    TEXTURE_WHITE_PAWN,
    TEXTURE_WHITE_ROOK,
    TEXTURE_WHITE_KNIGHT,
    TEXTURE_WHITE_BISHOP,
    TEXTURE_WHITE_KING,
    TEXTURE_WHITE_QUEEN,
    TEXTURE_BLACK_PAWN,
    TEXTURE_BLACK_ROOK,
    TEXTURE_BLACK_KNIGHT,
    TEXTURE_BLACK_BISHOP,
    TEXTURE_BLACK_KING,
    TEXTURE_BLACK_QUEEN,
    TEXTURES_COUNT
};

struct RgbaColor {
    int red;
    int green;
    int blue;
    int alpha;
};

enum GraphicsColor {
    LIGHT_SQUARES_COLOR,
    DARK_SQUARES_COLOR_BROWN,
    DARK_SQUARES_COLOR_BLUE,
    DARK_SQUARES_COLOR_GREEN,
    DARK_SQUARES_COLOR_RED,
    DARK_SQUARES_COLOR_YELLOW,
    BACKGROUND_COLOR,
    BLUE,
    LIGHTER_BLUE,
    TURQOUISE,
    GRAPHICS_COLOR_COUNT
};

struct GraphicsSquare {
    int x;
    int y;
    bool light_squares;
};

struct GraphicsContext {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *textures[TEXTURES_COUNT];
    //SDL_Surface *window_surface;
    //SDL_Surface *surfaces[SURFACES_COUNT];    // TODO change from surface to texture and use renderer
    int square_width;
    int window_height;
    int window_width;
    int board_total_height;
    int board_total_width;
    struct GraphicsSquare *graphics_board;       // 1D array representing nD board. Can be large -> malloc
    int graphics_board_length;
    struct RgbaColor rgba_colors[GRAPHICS_COLOR_COUNT];     // not used?
    //int separation_width_for_dim[MAX_DIMENSIONS];   // remove this
    //int coordinates_for_dim[MAX_DIMENSIONS][MAX_SIDE_LENGTH];   // remove this
};

bool initialize_graphics(struct GraphicsContext *graphics_context, int dimensions, int board_shape[]);
bool load_graphics_media(SDL_Texture *textures[TEXTURES_COUNT], SDL_Renderer *renderer);
bool draw_board(struct GraphicsContext *graphics_context, struct GameState *game_state, int selected_square, struct Move moves[], 
                struct Move diagonal_pawn_moves[]);
int  get_square_index_at_coordinates(int x, int y, struct GraphicsSquare *graphics_board, int graphics_board_length, 
                                     int square_width);
void terminate_graphics(struct GraphicsContext *graphics_context);

// returns n by 3 array with coordinates of where it drew the promotion options and which PieceType
// [[x, y, PieceType], ... ]
void draw_promotion_options(int square, enum PieceType promotion_options[], struct GraphicsContext *graphics_context,
                            int coordinates_of_piece_types_drawn[][3]); 

#endif // GRAPHICS_H
