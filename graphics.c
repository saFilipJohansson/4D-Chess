#include <stdbool.h>
#include "SDL.h"
#include <SDL_image.h>
#include "graphics.h"
#include "chess.h"

int get_square_index_at_coordinates(int x, int y, struct GraphicsSquare *graphics_board, int graphics_board_length, int square_width) {
    for (int i = 0; i < graphics_board_length; ++i) {
        if ((x >= graphics_board[i].x) && (y >= graphics_board[i].y) && (x < graphics_board[i].x + square_width) && (y < graphics_board[i].y + square_width)) {
            return i;
        }
    }
    return -1;
}

bool draw_board(struct GraphicsContext *graphics_context, struct GameState *game_state, int selected_square, struct Move moves[], 
                struct Move diagonal_pawn_moves[]) {
    SDL_Renderer *renderer = graphics_context->renderer;

    SDL_SetRenderDrawColor(graphics_context->renderer, 33, 37, 41, 0xFF);    // background color
    SDL_RenderClear(graphics_context->renderer);    // fills the screen with color set by SDL_SetRenderDrawColor

    // draw the board
    for (int square_index = 0; square_index < graphics_context->graphics_board_length; ++square_index) {
        bool light_squares = graphics_context->graphics_board[square_index].light_squares;
        if (light_squares) {
            SDL_SetRenderDrawColor(graphics_context->renderer, 237, 214, 176, 0xFF);    // light squares color
        } else {
            SDL_SetRenderDrawColor(graphics_context->renderer, 184, 134, 98, 0xFF);     // standard dark squares color
        }
        SDL_Rect rect = {graphics_context->graphics_board[square_index].x, graphics_context->graphics_board[square_index].y, 
                         graphics_context->square_width, graphics_context->square_width};
        SDL_RenderFillRect(renderer, &rect);
    }

    // draw highlighted squares
    // TODO: draw something to show the last move
    if (selected_square != -1) {
        SDL_SetRenderDrawColor(graphics_context->renderer, 50, 205, 205, 0xFF);    // turquoise
        SDL_Rect rect = {graphics_context->graphics_board[selected_square].x, graphics_context->graphics_board[selected_square].y, 
                         graphics_context->square_width, graphics_context->square_width};
        SDL_RenderFillRect(renderer, &rect);
    }

    SDL_SetRenderDrawColor(graphics_context->renderer, 130, 160, 222, 0xFF);    // lighter blue. This before blue
    for (int i = 0; diagonal_pawn_moves[i].destination_square != -1; ++i) {
        SDL_Rect rect = {graphics_context->graphics_board[diagonal_pawn_moves[i].destination_square].x,
                         graphics_context->graphics_board[diagonal_pawn_moves[i].destination_square].y, 
                         graphics_context->square_width, graphics_context->square_width};
        SDL_RenderFillRect(renderer, &rect);
    }

    SDL_SetRenderDrawColor(graphics_context->renderer, 50, 100, 205, 0xFF);    // blue
    for (int i = 0; moves[i].destination_square != -1; ++i) {
        SDL_Rect rect = {graphics_context->graphics_board[moves[i].destination_square].x, 
                         graphics_context->graphics_board[moves[i].destination_square].y, 
                         graphics_context->square_width, graphics_context->square_width};
        SDL_RenderFillRect(renderer, &rect);
    }
    
    // draw pieces
    for (int square_index = 0; square_index < graphics_context->graphics_board_length; ++square_index) {
        SDL_Rect rect = {graphics_context->graphics_board[square_index].x, graphics_context->graphics_board[square_index].y, 
                         graphics_context->square_width, graphics_context->square_width};
        enum PieceColor piece_color = game_state->board[square_index].piece.piece_color;
        switch (game_state->board[square_index].piece.piece_type) {
            case NULL_PIECE_TYPE:
                break;
            case PAWN: 
                if (piece_color == PIECE_COLOR_WHITE) {
                    SDL_RenderCopy(graphics_context->renderer, graphics_context->textures[TEXTURE_WHITE_PAWN], NULL, &rect);
                } else {
                    SDL_RenderCopy(graphics_context->renderer, graphics_context->textures[TEXTURE_BLACK_PAWN], NULL, &rect);
                }
                break;
            case ROOK: 
                if (piece_color == PIECE_COLOR_WHITE) {
                    SDL_RenderCopy(graphics_context->renderer, graphics_context->textures[TEXTURE_WHITE_ROOK], NULL, &rect);
                } else {
                    SDL_RenderCopy(graphics_context->renderer, graphics_context->textures[TEXTURE_BLACK_ROOK], NULL, &rect);
                }
                break;
            case KNIGHT:
                if (piece_color == PIECE_COLOR_WHITE) {
                    SDL_RenderCopy(graphics_context->renderer, graphics_context->textures[TEXTURE_WHITE_KNIGHT], NULL, &rect);
                } else {
                    SDL_RenderCopy(graphics_context->renderer, graphics_context->textures[TEXTURE_BLACK_KNIGHT], NULL, &rect);
                }
                break;
            case BISHOP:
                if (piece_color == PIECE_COLOR_WHITE) {
                    SDL_RenderCopy(graphics_context->renderer, graphics_context->textures[TEXTURE_WHITE_BISHOP], NULL, &rect);
                } else {
                    SDL_RenderCopy(graphics_context->renderer, graphics_context->textures[TEXTURE_BLACK_BISHOP], NULL, &rect);
                }
                break;
            case KING:
                if (piece_color == PIECE_COLOR_WHITE) {
                    SDL_RenderCopy(graphics_context->renderer, graphics_context->textures[TEXTURE_WHITE_KING], NULL, &rect);
                } else {
                    SDL_RenderCopy(graphics_context->renderer, graphics_context->textures[TEXTURE_BLACK_KING], NULL, &rect);
                }
                break;
            case QUEEN:
                if (piece_color == PIECE_COLOR_WHITE) {
                    SDL_RenderCopy(graphics_context->renderer, graphics_context->textures[TEXTURE_WHITE_QUEEN], NULL, &rect);
                } else {
                    SDL_RenderCopy(graphics_context->renderer, graphics_context->textures[TEXTURE_BLACK_QUEEN], NULL, &rect);
                }
                break;
        }
    }
    // Update window
    SDL_RenderPresent(graphics_context->renderer);
    return true;
}

bool initialize_graphics(struct GraphicsContext *graphics_context, int dimensions, int *board_shape) {
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Fiasco: SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {  // binary math
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }

    // Window height and width
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);
    int window_height = display_mode.h - 50;    // header thing has width 50
    int window_width = display_mode.w;

    graphics_context->window_height = window_height;
    graphics_context->window_width = window_width;

    graphics_context->window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_SHOWN);
    if(graphics_context->window == NULL) {
        printf("Disarray: Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    //graphics_context->window_surface = SDL_GetWindowSurface(graphics_context->window);
    graphics_context->renderer = SDL_CreateRenderer(graphics_context->window, -1, SDL_RENDERER_ACCELERATED);
    if (graphics_context->renderer == NULL) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    // Set how the alpha channel is used
    //SDL_SetRenderDrawBlendMode(graphics_context->renderer, SDL_BLENDMODE_BLEND);

    // Compute square_width for board to fill as much of screen as possible
    float separation_width_for_dim[MAX_DIMENSIONS] = {0.0, 0.0, 0.5, 0.5, 1.0, 1.0, 1.5, 1.5, 2.0, 2.0, 2.5, 2.5, 3.0, 3.0};    // change this hardcoding if MAX_DIMENSIONS changes
    float board_width_in_squares = 1.0;
    float board_height_in_squares = 1.0;
    for (int dim = 0; dim < dimensions; ++dim) {
        float this_dim = 0.0;
        if (dim % 2 == 0) {
            for (int i = 0; i < board_shape[dim]; ++i) {
                this_dim += board_width_in_squares;
                this_dim += separation_width_for_dim[dim];
            }
            board_width_in_squares = this_dim - separation_width_for_dim[dim];
        } else {
            for (int i = 0; i < board_shape[dim]; ++i) {
                this_dim += board_height_in_squares;
                this_dim += separation_width_for_dim[dim];
            }
            board_height_in_squares = this_dim - separation_width_for_dim[dim];
        }
    }

    int width_ratio = (int)(window_width / board_width_in_squares);
    int height_ratio = (int)(window_height / board_height_in_squares);
    int square_width = width_ratio < height_ratio ? width_ratio : height_ratio;
    graphics_context->square_width = square_width;

    for (int i = 2; i < MAX_DIMENSIONS; ++i) {
        separation_width_for_dim[i] *= square_width;
    }

    // Computing coordinates for all the dimensions by themselves
    // 0, 0 is bottom left  (or top left)
    int x_coord = square_width;
    int y_coord = square_width;
    int coordinates_for_dim[MAX_DIMENSIONS][MAX_SIDE_LENGTH];
    for (int dim = 0; dim < dimensions; ++dim) {
        int dim_coord = 0;
        coordinates_for_dim[dim][0] = dim_coord;
        if (dim % 2 == 0) {
            x_coord += separation_width_for_dim[dim];
            for (int i = 1; i < board_shape[dim] + 1; ++i) {
                dim_coord += x_coord;
                coordinates_for_dim[dim][i] = dim_coord;
            }
            x_coord = dim_coord - separation_width_for_dim[dim];
        } else {
            y_coord += separation_width_for_dim[dim];
            for (int i = 1; i < board_shape[dim] + 1; ++i) {
                dim_coord += y_coord;
                coordinates_for_dim[dim][i] = dim_coord;
            }
            y_coord = dim_coord - separation_width_for_dim[dim];
        }
    }
    int board_total_width = x_coord;
    int board_total_height = y_coord;

    graphics_context->board_total_width = board_total_width;
    graphics_context->board_total_height = board_total_height;

    // Computing exact coordinates for all squares
    // coordinates 0, 0 is top left of window
    // no rotation: 0 right, 1000 down
    // 90° rotation: 1000 down, 0 right
    int x_offset = (graphics_context->window_width - graphics_context->board_total_width) / 2;
    int y_offset = (graphics_context->window_height - graphics_context->board_total_height) / 2;
    int length = 1;
    for (int dim = 0; dim < dimensions; ++dim) {
        length *= board_shape[dim];
    }
    struct GraphicsSquare *graphics_board = malloc(sizeof(*graphics_board) * length);       // 1D array representing nD board. Can be large -> malloc
    int square[dimensions];
    for (int index = 0; index < length; ++index) {
        square_index_to_square(index, square, dimensions, board_shape);
        //printf("%d %d\n", square[0], square[1]);
        int x = 0;
        int y = board_total_height - square_width;  // top left corner of bottom left square
        int light_square_dark_square = 0;
        for (int dim = dimensions-1; dim >= 0; --dim) {
            light_square_dark_square += square[dim];
            if (dim % 2 == 0) {
                x += coordinates_for_dim[dim][square[dim]];
            } else {
                y -= coordinates_for_dim[dim][square[dim]];
            }
        }
        //printf("%d\n", x);
        graphics_board[index].x = x + x_offset;
        graphics_board[index].y = y + y_offset;
        graphics_board[index].light_squares = (light_square_dark_square % 2 == 1);
    }
    graphics_context->graphics_board = graphics_board;
    graphics_context->graphics_board_length = length;

    //SDL_PixelFormat *sdl_pixel_format = graphics_context->window_surface->format;
    //graphics_context->sdl_colors[LIGHT_SQUARES_COLOR] = SDL_MapRGB(sdl_pixel_format, 237, 214, 176);
    //graphics_context->sdl_colors[DARK_SQUARES_COLOR_BROWN] = SDL_MapRGB(sdl_pixel_format, 184, 134, 98);
    //graphics_context->sdl_colors[BACKGROUND_COLOR] = SDL_MapRGB(sdl_pixel_format, 255, 255, 255);

    return true;
}

static SDL_Texture *load_png_image(char path[], SDL_Renderer *renderer) {
    SDL_Texture *texture = IMG_LoadTexture(renderer, path);
    if (texture == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
        return NULL;
    }
    return texture;
}

//static SDL_Surface *load_png_image(char path[], SDL_PixelFormat *sdl_pixel_format) {
//    SDL_Surface *surface = IMG_Load(path);  // use load_texture instead, and use renderer thing
//    if (surface == NULL) {
//        printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
//        return NULL;
//    }
//
//    SDL_Surface *optimized_surface = SDL_ConvertSurface(surface, sdl_pixel_format, 0);
//    if (optimized_surface == NULL) {
//        printf("Unable to optimize image %s! SDL Error: %s\n", path, SDL_GetError());
//        return NULL;
//    }
//    SDL_FreeSurface(surface);
//    return optimized_surface;
//}

bool load_graphics_media(SDL_Texture *textures[], SDL_Renderer *renderer) {
    if((textures[TEXTURE_WHITE_PAWN] = load_png_image("images/w_pawn.png", renderer)) == NULL) {
        return false;
    }
    if((textures[TEXTURE_WHITE_ROOK] = load_png_image("images/w_rook.png", renderer)) == NULL) {
        return false;
    }
    if((textures[TEXTURE_WHITE_KNIGHT] = load_png_image("images/w_knight.png", renderer)) == NULL) {
        return false;
    }
    if((textures[TEXTURE_WHITE_BISHOP] = load_png_image("images/w_bishop.png", renderer)) == NULL) {
        return false;
    }
    if((textures[TEXTURE_WHITE_KING] = load_png_image("images/w_king.png", renderer)) == NULL) {
        return false;
    }
    if((textures[TEXTURE_WHITE_QUEEN] = load_png_image("images/w_queen.png", renderer)) == NULL) {
        return false;
    }

    if((textures[TEXTURE_BLACK_PAWN] = load_png_image("images/b_pawn.png", renderer)) == NULL) {
        return false;
    }
    if((textures[TEXTURE_BLACK_ROOK] = load_png_image("images/b_rook.png", renderer)) == NULL) {
        return false;
    }
    if((textures[TEXTURE_BLACK_KNIGHT] = load_png_image("images/b_knight.png", renderer)) == NULL) {
        return false;
    }
    if((textures[TEXTURE_BLACK_BISHOP] = load_png_image("images/b_bishop.png", renderer)) == NULL) {
        return false;
    }
    if((textures[TEXTURE_BLACK_KING] = load_png_image("images/b_king.png", renderer)) == NULL) {
        return false;
    }
    if((textures[TEXTURE_BLACK_QUEEN] = load_png_image("images/b_queen.png", renderer)) == NULL) {
        return false;
    }
    return true;
}

void terminate_graphics(struct GraphicsContext *graphics_context) {
    for (int i = 0; i < TEXTURES_COUNT; ++i) {
        SDL_DestroyTexture(graphics_context->textures[i]);
        graphics_context->textures[i] = NULL;
    }

    SDL_DestroyRenderer(graphics_context->renderer);
    graphics_context->renderer = NULL;

    SDL_DestroyWindow(graphics_context->window);
    graphics_context->window = NULL;

    IMG_Quit();
    SDL_Quit();
}
