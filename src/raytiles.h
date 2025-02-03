#pragma once
#include <raylib.h>
#include "stdlib.h"
#include "common.h"

#define NO_BG (Color) {0,0,0,0}
#define NO_SMBL 0
#define CALCULATE_MAX_TILES 0

typedef struct
{
    Color bg_col;
    Color char_col;
    char symbol;
    Font *font;
}Tile_t;

typedef struct
{
	int offset_x;
	int offset_y;
    int on_scr_size_x;
    int on_scr_size_y;
    uint tile_width;
    uint max_tile_count;
    float tile_h_to_w_ratio;
    Color default_col;
    Font *default_font;

    Tile_t *tiles;
} Grid_t;

Grid_t *tl_init_grid(int offset_x, int offset_y, int on_scr_size_x, int on_scr_size_y, 
					uint tile_width, float tile_h_to_w_ratio, uint max_tile_count, Color def_col, Font *def_font);

void tl_deinit_grid(Grid_t *grid);

Pos_t tl_grid_get_size(Grid_t *grid);

void tl_render_grid(Grid_t *grid);

void tl_resize_grid(Grid_t *grid, int new_offset_x, int new_offset_y, int new_scr_size_x, int new_scr_size_y, uint new_tile_width);

void tl_fit_subgrid(Grid_t *top_grid, Grid_t *sub_grid, uint x0, uint y0, uint x1, uint y1);

void tl_draw_tile(Grid_t *grid, uint x, uint y, char symbol, Color char_col, Color bg_col, Font *font);

void tl_draw_rect(Grid_t *grid, uint x0, uint y0, uint width, uint height, char symbol, Color char_col, Color bg_col, Font *font);

void tl_draw_line(Grid_t *grid, uint x0, uint y0, uint x1, uint y1, char symbol, Color char_col, Color bg_col, Font *font);

uint tl_draw_text(Grid_t * grid, uint x, uint y, uint wrap, char *text, uint len, Color char_col, Color bg_col, Font *font);

void tl_draw_prose(uint x, uint y, char *wrapped_text, uint len, Color char_col, Color bg_col, Font *font);

Pos_t tl_screen_to_grid_coords(Grid_t *grid, Pos_t xy);

void tl_set_tile_bg(Grid_t *grid, uint x, uint y, Color bg_col);


