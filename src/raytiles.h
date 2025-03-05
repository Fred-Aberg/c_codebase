#pragma once
#include <raylib.h>
#include "stdlib.h"
#include "common.h"

#define NO_COL (Color) {0,0,0,0}
#define NO_SMBL 0
#define CALCULATE_MAX_TILES 0

typedef struct
{
	int offset_x;
	int offset_y;
    int on_scr_size_x;
    int on_scr_size_y;
    uint_t tile_width;
    uint_t max_tile_count;
    float tile_h_to_w_ratio;
	float txt_padding_prc_v;
	float txt_padding_prc_h;
	float font_size_multiplier;
    Color default_col;
    Font *default_font;

	char *symbols;			// Textures? more efficient than fonts?
	Color *symbol_colors;
	Color *bg_colors;
    Font **fonts;
} Grid_t;

Grid_t *tl_init_grid(int offset_x, int offset_y, int on_scr_size_x, int on_scr_size_y, 
					uint_t tile_width, float tile_h_to_w_ratio, uint_t max_tile_count, Color def_col, Font *def_font);

void tl_deinit_grid(Grid_t *grid);

Pos_t tl_grid_get_size(Grid_t *grid);

Pos_t tl_render_grid(Grid_t *grid);

void tl_center_grid_on_screen(Grid_t *grid, uint_t scr_size_x, uint_t scr_size_y);

void tl_resize_grid(Grid_t *grid, int new_offset_x, int new_offset_y, int new_scr_size_x, int new_scr_size_y, uint_t new_tile_width);

void tl_fit_subgrid(Grid_t *top_grid, Grid_t *sub_grid, uint_t x0, uint_t y0, uint_t x1, uint_t y1);

void tl_draw_tile(Grid_t *grid, uint_t x, uint_t y, char symbol, Color char_col, Color bg_col, Font *font);

void tl_draw_rect(Grid_t *grid, uint_t x0, uint_t y0, uint_t width, uint_t height, char symbol, Color char_col, Color bg_col, Font *font);

void tl_draw_line(Grid_t *grid, uint_t x0, uint_t y0, uint_t x1, uint_t y1, char symbol, Color char_col, Color bg_col, Font *font);

uint_t tl_draw_text(Grid_t * grid, uint_t x, uint_t y, uint_t wrap, char *text, uint_t len, Color char_col, Color bg_col, Font *font);

void tl_draw_prose(uint_t x, uint_t y, char *wrapped_text, uint_t len, Color char_col, Color bg_col, Font *font);

Pos_t tl_screen_to_grid_coords(Grid_t *grid, Pos_t xy);

void tl_set_tile_bg(Grid_t *grid, uint_t x, uint_t y, Color bg_col);

void tl_set_tile_char_col(Grid_t *grid, uint_t x, uint_t y, Color char_col);

void tl_tile_invert_colors(Grid_t *grid, uint_t x, uint_t y);

void tl_grid_set_txt_padding(Grid_t *grid, float pp);


