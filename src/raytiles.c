#include <raylib.h>
#include "raytiles.h"
#include "math.h"
#include "stdio.h"
#include "assert.h"
#include "string.h"

#define MIN(a, b) (a < b)? a : b
#define grid_width(grid) ((grid->on_scr_size_x) / grid->tile_p_w)		// max 255
#define tile_pixel_height(grid) (int)(grid->tile_h_to_w_ratio * (float)grid->tile_p_w)
#define grid_height(grid) ((grid->on_scr_size_y) / tile_pixel_height(grid))		// max 255
#define grid_size(grid) (grid_width(grid) * grid_height(grid))
#define rendered_grid_size_x(grid) (grid_width(grid) * grid->tile_p_w - grid->offset_x)
#define rendered_grid_size_y(grid) (grid_height(grid) * tile_pixel_height(grid) - grid->offset_y)


typedef struct
{
	uchar_t g_w;
	uchar_t g_h;
	int t_w;
	int t_h;
	int offset_x;
	int offset_y;
	Font *g_fonts;
	float font_size;
}grid_rendering_data_t;

void sanitize_tile_p_w(grid_t *grid)
{
	int g_w = grid_width(grid);
	int g_h = grid_width(grid);

	if (g_w <= MAX_GRID_W && g_h <= MAX_GRID_H)
		return; // Valid tile_p_w

	int min_p_w_x = (grid->on_scr_size_x / MAX_GRID_W) + 1;
	int min_p_w_y = ((grid->on_scr_size_y / MAX_GRID_H) / grid->tile_h_to_w_ratio) + 1;

	grid->tile_p_w = max(min_p_w_x, min_p_w_y);
}

// r <= 7, g <= 7, b <= 3, otherwise undefined behaviour
color8b_t col8bt(char r, char b, char g)
{
	return ( r<<5 ) + ( g<<2 ) + b;
}

const char convert_3b_to_8b[8] = {0, 32, 64, 96, 128, 160, 192, 255};
const char convert_2b_to_8b[4] = {0, 64, 128, 255};

Color tl_color8b_to_Color(color8b_t col)
{
							// 1110000						00011100							00000011
	return c(convert_3b_to_8b[(col & 224)>>5], convert_3b_to_8b[(col & 28)>>2], convert_2b_to_8b[col & 3]);
}

color8b_t tl_Color_to_color8b(Color col)
{
	return ( (char)(((float)col.r/255.0f) * 7.0f)<<5 ) + ( (char)(((float)col.g/255.0f) * 7.0f)<<2 ) + ( (char)(((float)col.b/255.0f) * 3.0f) );
}

grid_t *tl_init_grid(int offset_x, int offset_y, int on_scr_size_x, int on_scr_size_y, uint_t tile_p_w, float tile_h_to_w_ratio, 
					Font *fonts, uint_t starting_instruction_capacity)
{
    grid_t *grid = calloc(1, sizeof(grid_t));

    grid->offset_x = offset_x;
    grid->offset_y = offset_y;
    grid->on_scr_size_x = on_scr_size_x;
    grid->on_scr_size_y = on_scr_size_y;
	grid->tile_p_w = tile_p_w;
	grid->tile_h_to_w_ratio = tile_h_to_w_ratio;

	sanitize_tile_p_w(grid);
	
	grid->txt_padding_prc_h = 0.15f;  // 15% padding by default
	grid->txt_padding_prc_v = 0.0f;  // 0% padding by default
	grid->font_size_multiplier = 1.0f; // 100% 

	grid->fonts = fonts;

	grid->instructions_capacity = starting_instruction_capacity;
	grid->instructions_count = 0;
    grid->instructions = calloc(starting_instruction_capacity, sizeof(instruction_t));
    
    return grid;
}

Pos_t tl_grid_get_dimensions(grid_t *grid)
{
    return (Pos_t){.x = grid_width(grid), .y = grid_height(grid)};
}

void tl_deinit_grid(grid_t *grid)
{
    free(grid->instructions);
    free(grid);
}

void draw_rect(grid_rendering_data_t g_data, uchar_t x0, uchar_t y0, uchar_t x1, uchar_t y1, color8b_t bg_col)
{
	DrawRectangle(g_data.offset_x + x0 * g_data.t_w, g_data.offset_y + y0 * g_data.t_h,
				  g_data.t_w * (x1 - x0 + 1), g_data.t_h * (y1 - y0 + 1), tl_color8b_to_Color(bg_col));
}

#define ins_type(instruction) (instruction.type_bit & 1)
#define font_index(smbl_instruction) (int)((smbl_instruction.type_and_font_bits & 254)>>1)

uint_t render_smbl_instruction(grid_rendering_data_t g_data, smbl_instruction_t smbl_instruction)
{
	uint_t draw_calls = 0;
	for (uchar_t _y = smbl_instruction.rect.y0; _y <= smbl_instruction.rect.y1; _y++)
		for (uchar_t _x = smbl_instruction.rect.x0; _x <= smbl_instruction.rect.x1; _x++)
		{
			DrawTextCodepoint(g_data.g_fonts[font_index(smbl_instruction)] , smbl_instruction.smbl,
							(Vector2){g_data.offset_x + _x * g_data.t_w, g_data.offset_y + _y * g_data.t_h},
							g_data.font_size, tl_color8b_to_Color(smbl_instruction.smbl_col));
			draw_calls++;
		}
	return draw_calls;
}

Pos_t render_instructions(grid_t *grid)
{
	Pos_t draw_calls = pos(0,0);
	
	grid_rendering_data_t g_data =
	{
		grid_width(grid),
		grid_height(grid),
		grid->tile_p_w,
		tile_pixel_height(grid),
		grid->offset_x,
		grid->offset_y,
		grid->fonts,
		grid->tile_p_w * grid->font_size_multiplier
	};
	instruction_t *instructions = grid->instructions;
	instruction_t c_instruction;
	
	bg_instruction_t *bg_instruction;

	for (uint_t i = 0; i < grid->instructions_count; i++)
	{
		c_instruction = instructions[i];
		if(ins_type(c_instruction))
		{
			bg_instruction = (bg_instruction_t *)(&c_instruction);
			draw_rect(g_data, bg_instruction->rect.x0, bg_instruction->rect.y0,  bg_instruction->rect.x1, bg_instruction->rect.y1, bg_instruction->bg_col);
			draw_calls.x++;
		}
		else
		{
			draw_calls.y += render_smbl_instruction(g_data, *(smbl_instruction_t *)(&c_instruction));
		}
	}
	
	return draw_calls;
}


Pos_t tl_render_grid(grid_t *grid)
{
	// Pos_t dcs = pos(render_background(grid), render_symbols(grid));
	Pos_t dcs = render_instructions(grid);
	grid->instructions_count = 0;
    return dcs;
}

void tl_center_grid_on_screen(grid_t *grid, int scr_size_x, int scr_size_y)
{
	grid->offset_x = (scr_size_x - rendered_grid_size_x(grid))/2;
	grid->offset_y = (scr_size_y - rendered_grid_size_y(grid))/2;
}


void tl_change_tile_pw(grid_t *grid, int pw_change)
{
	grid->tile_p_w += pw_change;
	sanitize_tile_p_w(grid);
}

void tl_set_tile_pw(grid_t *grid, int new_tile_pw)
{
	grid->tile_p_w = new_tile_pw;
	sanitize_tile_p_w(grid);
}

//new_tile_width = 0 -> minimum will be calculated
void tl_resize_grid(grid_t *grid, int new_offset_x, int new_offset_y, int new_scr_size_x, int new_scr_size_y, uint_t new_tile_width)
{
    grid->offset_x = new_offset_x;
    grid->offset_y = new_offset_y;
    grid->on_scr_size_x = new_scr_size_x;
    grid->on_scr_size_y = new_scr_size_y;
	grid->tile_p_w = new_tile_width;
	sanitize_tile_p_w(grid);
}

void tl_fit_subgrid(grid_t *top_grid, grid_t *sub_grid, uchar_t x0, uchar_t y0, uchar_t x1, uchar_t y1)
{
	int new_offset_x = x0 * top_grid->tile_p_w + top_grid->offset_x;
	int new_offset_y = y0 * tile_pixel_height(top_grid) + top_grid->offset_y;
	int new_scr_size_x = (x1-x0 + 1) * top_grid->tile_p_w;
	int new_scr_size_y = (y1-y0 + 1) * tile_pixel_height(top_grid);
	tl_resize_grid(sub_grid, new_offset_x, new_offset_y, new_scr_size_x, new_scr_size_y, sub_grid->tile_p_w);
}

// Adding instructions //

#define r(x0, y0, x1, y1) (rect_t){x0, y0, x1, y1}

#define REALLOC_PERCENTAGE 1.5f
void check_realloc(grid_t *grid)
{
	if(grid->instructions_capacity == grid->instructions_count)
	{
		grid->instructions_capacity = grid->instructions_capacity * REALLOC_PERCENTAGE + 1;
		grid->instructions = realloc(grid->instructions, grid->instructions_capacity * sizeof(bg_instruction_t));
	}
}

bg_instruction_t *add_bg_instruction(grid_t *grid, uchar_t x0, uchar_t y0, uchar_t x1, uchar_t y1, color8b_t bg_col)
{
	check_realloc(grid);
	bg_instruction_t new_bg_instruction = (bg_instruction_t){BG, 0, bg_col, r(x0, y0, x1, y1)};

	bg_instruction_t *bg_loc = (bg_instruction_t *)&grid->instructions[grid->instructions_count];

	memcpy(bg_loc, &new_bg_instruction, sizeof(instruction_t));

	grid->instructions_count++;

	return bg_loc;
}

smbl_instruction_t *add_smbl_instruction(grid_t *grid, uchar_t x0, uchar_t y0, uchar_t x1, uchar_t y1, char font, uchar_t smbl, color8b_t smbl_col)
{
	check_realloc(grid);
	char type_and_font_bits = (char)SMBL | (font << 1);

	
	smbl_instruction_t new_smbl_instruction = (smbl_instruction_t){type_and_font_bits, smbl, smbl_col, r(x0, y0, x1, y1)};

	smbl_instruction_t *smbl_loc = (smbl_instruction_t *)&grid->instructions[grid->instructions_count];

	memcpy(smbl_loc, &new_smbl_instruction, sizeof(instruction_t));

	grid->instructions_count++;

	return smbl_loc;
}

/// Plotting functions

smbl_instruction_t *tl_plot_smbl(grid_t *grid, uchar_t x, uchar_t y, uchar_t symbol, color8b_t char_col, char font)
{
	return add_smbl_instruction(grid,  x,  y,  x,  y, font, symbol, char_col);
}

bg_instruction_t *tl_plot_bg(grid_t *grid, uchar_t x, uchar_t y, color8b_t bg_col)
{
	return add_bg_instruction(grid,  x,  y,  x,  y, bg_col);
}


void tl_plot_smbl_w_bg(grid_t *grid, uchar_t x, uchar_t y, uchar_t symbol, color8b_t char_col, color8b_t bg_col, char font)
{
	add_bg_instruction(grid,  x,  y,  x,  y, bg_col);
	add_smbl_instruction(grid,  x,  y,  x,  y, font, symbol, char_col);
}

/// Drawing functions

smbl_instruction_t *tl_draw_rect_smbl(grid_t *grid, uchar_t x0, uchar_t y0, uchar_t x1, uchar_t y1, char symbol, color8b_t char_col, char font)
{
	return add_smbl_instruction(grid,  x0,  y0,  x1,  y1, font, symbol, char_col);
}

bg_instruction_t *tl_draw_rect_bg(grid_t *grid, uchar_t x0, uchar_t y0, uchar_t x1, uchar_t y1, color8b_t bg_col)
{
	return add_bg_instruction(grid,  x0,  y0,  x1,  y1, bg_col);
}
void tl_draw_rect_smbl_w_bg(grid_t *grid, uchar_t x0, uchar_t y0, uchar_t x1, uchar_t y1, char symbol, color8b_t char_col, color8b_t bg_col, char font)
{
	add_bg_instruction(grid,  x0,  y0,  x1,  y1, bg_col);
	add_smbl_instruction(grid,  x0,  y0,  x1,  y1, font, symbol, char_col);
}

// NOTE: Non-cardinal lines will use plot -> less efficient -> returns NULL instead of an instruction
void tl_draw_line_non_orthogonal(grid_t *grid, uchar_t x0, uchar_t y0, uchar_t x1, uchar_t y1, uchar_t symbol, uchar_t char_col, color8b_t bg_col, char font, char scenario);

smbl_instruction_t *tl_draw_line_smbl(grid_t *grid, uchar_t x0, uchar_t y0, uchar_t x1, uchar_t y1, uchar_t symbol, color8b_t char_col, char font)
{
	if (!(x0 == x1 || y0 == y1)) // Non-cardinal
		tl_draw_line_non_orthogonal(grid, x0, y0, x1, y1, symbol, char_col, 0, font, 3);
	return add_smbl_instruction(grid,  x0,  y0,  x1,  y1, font, symbol, char_col);
}

bg_instruction_t *tl_draw_line_bg(grid_t *grid, uchar_t x0, uchar_t y0, uchar_t x1, uchar_t y1, color8b_t bg_col)
{
	if (!(x0 == x1 || y0 == y1)) // Non-cardinal
		tl_draw_line_non_orthogonal(grid, x0, y0, x1, y1, 0, 0, bg_col, 0, 1);
	return add_bg_instruction(grid,  x0,  y0,  x1,  y1, bg_col);
}

void tl_draw_line_smbl_w_bg(grid_t *grid, uchar_t x0, uchar_t y0, uchar_t x1, uchar_t y1, uchar_t symbol, color8b_t char_col, color8b_t bg_col, char font)
{
	if (!(x0 == x1 || y0 == y1)) // Non-cardinal
		tl_draw_line_non_orthogonal(grid, x0, y0, x1, y1, symbol, char_col, bg_col, font, 0);
	add_bg_instruction(grid,  x0,  y0,  x1,  y1, bg_col);
	add_smbl_instruction(grid,  x0,  y0,  x1,  y1, font, symbol, char_col);
}

void tl_draw_line_non_orthogonal(grid_t *grid, uchar_t x0, uchar_t y0, uchar_t x1, uchar_t y1, uchar_t symbol, uchar_t char_col, color8b_t bg_col, char font, char scenario)
{
    // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
    int dx = abs(x1 - x0);			// Don't believe the linter's lies, the abs() are needed!
    int sx = (x0 < x1)? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = (y0 < y1)? 1 : -1;
    int error = dx + dy;
    
    while (true)
    {
    	if(scenario == 0)
    	{
        	add_bg_instruction(grid,  x0,  y0,  x0,  y0, bg_col);
    		add_smbl_instruction(grid,  x0,  y0,  x0,  y0, font, symbol, char_col);
    	}
    	else if(scenario == 1)
        	add_bg_instruction(grid,  x0,  y0,  x0,  y0, bg_col);
    	else
    		add_smbl_instruction(grid,  x0,  y0,  x0,  y0, font, symbol, char_col);

        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * error;

        if (e2 >= dy)
        {
            if (x0 == x1) break;
            error += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            if (y0 == y1) break;
            error += dx;
            y0 += sy;
        }
    }
}

Pos_t tl_screen_to_grid_coords(grid_t *grid, Pos_t xy)
{
	xy.x = uclamp(grid->offset_x, xy.x, grid->offset_x + grid->on_scr_size_x);
	xy.y = uclamp(grid->offset_y, xy.y, grid->offset_y + grid->on_scr_size_y);

    return pos((xy.x - grid->offset_x) / grid->tile_p_w, (xy.y - grid->offset_y) / tile_pixel_height(grid));
}

void tl_grid_set_txt_padding(grid_t *grid, float pp)
{
	grid->txt_padding_prc_h = pp;
}

void tl_print_grid_info(grid_t *grid)
{
	printf("\n-G[%ux%u]\n--T[%ux%u]\n--F[%dp]\n",
			grid_width(grid),
			grid_height(grid),
			grid->tile_p_w,
			tile_pixel_height(grid),
			(int)(grid->tile_p_w * grid->font_size_multiplier));
}
