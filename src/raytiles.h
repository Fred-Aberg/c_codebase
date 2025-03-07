#pragma once
#include <raylib.h>
#include "stdlib.h"
#include "common.h"

#define NO_COL (Color) {0,0,0,0}
#define NO_SMBL 0
#define CALCULATE_MAX_TILES 0

#define MAX_GRID_W 256
#define MAX_GRID_H 256
#define MAX_GRID_SIZE 65536

#define BLACK8B 0
#define WHITE8B 255

typedef unsigned char color8b_t;

color8b_t col8bt(char r, char b, char g);

Color tl_color8b_to_Color(color8b_t col);

color8b_t tl_Color_to_color8b(Color col);

typedef struct 
{
	uchar_t x0;
	uchar_t y0;
	uchar_t x1;
	uchar_t y1;
}rect_t;						// 32 bits = 4 bytes

typedef struct
{
	char type_bit;				// 8 bits
	char padding;				// 8 bits
	color8b_t bg_col;			// 8 bit colors
	rect_t rect;				// 32 bits
}bg_instruction_t;				// = 56 bits = 7 bytes

typedef struct
{
	char type_and_font_bits;	// 8 bits 1b + 7b
	uchar_t smbl;				// 8 bits
	color8b_t smbl_col;			// 8 bit colors
	rect_t rect;				// 32 bits
}smbl_instruction_t;			// = 56 bits = 7 bytes

#define SMBL 0
#define BG 1

typedef struct
{
	char type_bit;
	char padding;
	color8b_t color;
	rect_t rect;
}instruction_t;		// Generic type covering smbl and bg instruction types

typedef struct
{
	int offset_x;
	int offset_y;
    int on_scr_size_x;
    int on_scr_size_y;
    
    int tile_p_w;
    float tile_h_to_w_ratio;
	float txt_padding_prc_v;
	float txt_padding_prc_h;
	float font_size_multiplier;
	
	uint_t instructions_count;
	uint_t instructions_capacity;
	instruction_t *instructions;
	
    Font *fonts; // max 128 font-indexes
} grid_t;

grid_t *tl_init_grid(int offset_x, int offset_y, int on_scr_size_x, int on_scr_size_y, uint_t tile_p_w, float tile_h_to_w_ratio, 
					Font *fonts, uint_t starting_instruction_capacity);

void tl_deinit_grid(grid_t *grid);

Pos_t tl_grid_get_dimensions(grid_t *grid);

Pos_t tl_render_grid(grid_t *grid);

void tl_center_grid_on_screen(grid_t *grid, int scr_size_x, int scr_size_y);

void tl_change_tile_pw(grid_t *grid, int pw_change);

void tl_set_tile_pw(grid_t *grid, int new_tile_pw);

void tl_resize_grid(grid_t *grid, int new_offset_x, int new_offset_y, int new_scr_size_x, int new_scr_size_y, uint_t new_tile_width);

void tl_fit_subgrid(grid_t *top_grid, grid_t *sub_grid, uchar_t x0, uchar_t y0, uchar_t x1, uchar_t y1);

// Plot functions //
	// Use sparingly, for isolated graphical elements and text

smbl_instruction_t *tl_plot_smbl(grid_t *grid, uchar_t x, uchar_t y, uchar_t symbol, color8b_t char_col, char font);

bg_instruction_t *tl_plot_bg(grid_t *grid, uchar_t x, uchar_t y, color8b_t bg_col);

void tl_plot_smbl_w_bg(grid_t *grid, uchar_t x, uchar_t y, uchar_t symbol, color8b_t char_col, color8b_t bg_col, char font);

// Draw functions //
	// Generates instructions for drawing in batches

smbl_instruction_t *tl_draw_rect_smbl(grid_t *grid, uchar_t x0, uchar_t y0, uchar_t x1, uchar_t y1, char symbol, color8b_t char_col, char font);

bg_instruction_t *tl_draw_rect_bg(grid_t *grid, uchar_t x0, uchar_t y0, uchar_t x1, uchar_t y1, color8b_t bg_col);

void tl_draw_rect_smbl_w_bg(grid_t *grid, uchar_t x0, uchar_t y0, uchar_t x1, uchar_t y1, char symbol, color8b_t char_col, color8b_t bg_col, char font);

// NOTE: Non-cardinal lines will use plot -> less efficient -> returns NULL instead of an instruction

smbl_instruction_t *tl_draw_line_smbl(grid_t *grid, uchar_t x0, uchar_t y0, uchar_t x1, uchar_t y1, uchar_t symbol, color8b_t char_col, char font);

bg_instruction_t *tl_draw_line_bg(grid_t *grid, uchar_t x0, uchar_t y0, uchar_t x1, uchar_t y1, color8b_t bg_col);

void tl_draw_line_smbl_w_bg(grid_t *grid, uchar_t x0, uchar_t y0, uchar_t x1, uchar_t y1, uchar_t symbol, color8b_t char_col, color8b_t bg_col, char font);

// Misc.

Pos_t tl_screen_to_grid_coords(grid_t *grid, Pos_t xy);

void tl_grid_set_txt_padding(grid_t *grid, float pp);

void tl_print_grid_info(grid_t *grid);

