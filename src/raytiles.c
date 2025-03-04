#include <raylib.h>
#include "raytiles.h"
#include "math.h"
#include "stdio.h"
#include "assert.h"
#include "string.h"

#define MIN(a, b) (a < b)? a : b
#define grid_width(grid) ((grid->on_scr_size_x) / grid->tile_width)
#define tile_height(grid) (uint_t)(grid->tile_h_to_w_ratio * (float)grid->tile_width)
#define grid_height(grid) ((grid->on_scr_size_y) / tile_height(grid))
#define grid_size(grid) (grid_width(grid) * grid_height(grid))
#define rendered_grid_size_x(grid) (grid_width(grid) * grid->tile_width)
#define rendered_grid_size_y(grid) (grid_height(grid) * tile_height(grid))

static uint_t pos_to_i(Grid_t *grid, uint_t x, uint_t y)
{
	if(x >= grid_width(grid) && y >= grid_height(grid))
		ERRORF("\nRAYTILES: coord overflow in pos_to_i(%u, %u)\n", x, y)
	return x + y * grid_width(grid);
}

// Convert array index of cell to cell's coordinates
static Pos_t i_to_pos(Grid_t *grid, int i)
{
    uint_t y = i / grid_width(grid); // C autimatically floors the result
    uint_t x = i - (y * grid_width(grid));
	return (Pos_t){.x = x, .y = y};
}

Grid_t *tl_init_grid(int offset_x, int offset_y, int on_scr_size_x, int on_scr_size_y, 
					uint_t tile_width, float tile_h_to_w_ratio, uint_t max_tile_count, Color def_col, Font *def_font)
{
    Grid_t *grid = calloc(1, sizeof(Grid_t));

    grid->offset_x = offset_x;
    grid->offset_y = offset_y;
    grid->on_scr_size_x = on_scr_size_x;
    grid->on_scr_size_y = on_scr_size_y;
	grid->tile_width = tile_width;
	grid->tile_h_to_w_ratio = tile_h_to_w_ratio;
	grid->txt_padding_prc_h = 0.15f;  // 15% padding by default
	grid->txt_padding_prc_v = 0.0f;  // 0% padding by default
	grid->font_size_multiplier = 1.0f; // 100% 
	grid->max_tile_count = max_tile_count;
    grid->default_col = def_col;
    grid->default_font = def_font;

	// Which approach will result in fewer bugs?
	// assert(grid_size(grid) <= max_tile_count);
	grid->max_tile_count = umax(grid->max_tile_count, grid_size(grid));
	
    grid->symbols = calloc(grid->max_tile_count, sizeof(char));
    grid->symbol_colors = calloc(grid->max_tile_count, sizeof(Color));
    grid->bg_colors = calloc(grid->max_tile_count, sizeof(Color));
    grid->fonts = calloc(grid->max_tile_count, sizeof(Font *));

    fprintf(stderr, "tl_grid initialized - size: %d x %d -> %d tiles (MAX[%u]) - coords: (0-%d, 0-%d)\n",
    		grid_width(grid), grid_height(grid), grid_size(grid), max_tile_count,
    		grid_width(grid) - 1, grid_height(grid) - 1);
    
    return grid;
}

Pos_t tl_grid_get_size(Grid_t *grid)
{
    return (Pos_t){.x = grid_width(grid), .y = grid_height(grid)};
}

void tl_deinit_grid(Grid_t *grid)
{
    free(grid->symbols);
    free(grid->symbol_colors);
    free(grid->bg_colors);
    free(grid->fonts);
    free(grid);
}


bool color_eq(Color a, Color b)
{
	return (a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a);
}

#define fast_i(width, x, y) (x + y * width)
#define area(x0, y0, x1, y1) (x1 - x0 + 1) * (y1 - y0 + 1)

void draw_rect(Grid_t *grid, uint_t x0, uint_t y0, uint_t x1, uint_t y1, Color bg_col)
{
	uint_t tile_height = tile_height(grid);
	DrawRectangle(grid->offset_x + x0 * grid->tile_width, grid->offset_y + y0 * tile_height,
				  grid->tile_width * (x1 - x0 + 1), tile_height * (y1 - y0 + 1), bg_col);
}

void draw_line(Grid_t *grid, uint_t x0, uint_t x1, uint_t y, Color bg_col)
{
	uint_t tile_height = tile_height(grid);
	DrawRectangle(grid->offset_x + x0 * grid->tile_width, grid->offset_y + y * tile_height,
				  grid->tile_width * (x1 - x0 + 1), tile_height * (y + 1), bg_col);
}

uint_t render_background(Grid_t *grid)
{
	Color *bg_ptr = grid->bg_colors;
	uint_t width = grid_width(grid);
	uint_t height = grid_height(grid);
	uint_t x_max = width - 1;
	uint_t n_draw_calls = 0;

	Color c_bg;
	Color new_bg;
	uint_t c_bg_start_x;
	bool same_bg;
	
	for(uint_t _y = 0; _y < height; _y++)
	{
		c_bg = bg_ptr[fast_i(width, 0, _y)];
		c_bg_start_x = 0;
		for(uint_t _x = 1; true; _x++)
		{
			new_bg = bg_ptr[fast_i(width, _x, _y)];
			same_bg = color_eq(new_bg, c_bg);
			
			if(_x == x_max)
			{
				if(same_bg)
				{
					draw_line(grid, c_bg_start_x, x_max, _y, c_bg);
					n_draw_calls++;
					break;
				}
				else
				{
					draw_line(grid, c_bg_start_x, x_max - 1, _y, c_bg);
					draw_line(grid, x_max, x_max, _y, new_bg);
					n_draw_calls += 2;
					break;
				}
			}
			
			if(!color_eq(new_bg, c_bg))
			{
				draw_line(grid, c_bg_start_x, _x - 1, _y, c_bg);
				n_draw_calls++;
				c_bg_start_x = _x;
				c_bg = new_bg;
			}
			
		}
	}
	return n_draw_calls;
}

uint_t render_symbols(Grid_t *grid)
{
	Color *col_ptr = grid->symbol_colors;
	char *symbols = grid->symbols;
	Font **fonts = grid->fonts;
	uint_t tile_height = tile_height(grid);
	uint_t width = grid_width(grid);
	uint_t height = grid_height(grid);
	uint_t n_draw_calls = 0;

	uint_t index;
	Color c_col;
	Font c_font;
	for(uint_t _y = 0; _y < height; _y++)
	{
		for(uint_t _x = 0; _x < width; _x++)
		{
			index = fast_i(width, _x, _y);
			c_col = col_ptr[index];
			if(symbols[index] != '\0' && c_col.a != 0)
			{
				index = fast_i(width, _x, _y);	
				c_font = (fonts[index])? *fonts[index] : *grid->default_font;
				DrawTextCodepoint(c_font , symbols[index],
									(Vector2){grid->offset_x + _x * grid->tile_width, grid->offset_y + _y * tile_height},
									grid->tile_width * grid->font_size_multiplier, c_col);
				n_draw_calls++;
			}
		}
	}
	return n_draw_calls;
}

uint_t tl_render_grid(Grid_t *grid)
{
    return render_background(grid) + render_symbols(grid);
}

void tl_center_grid_on_screen(Grid_t *grid, uint_t scr_size_x, uint_t scr_size_y)
{
	uint_t actual_grid_scr_size_x = grid_width(grid) * grid->tile_width;
	uint_t actual_grid_scr_size_y = grid_height(grid) * tile_height(grid);

	grid->on_scr_size_x = actual_grid_scr_size_x;
	grid->on_scr_size_y = actual_grid_scr_size_y;

	grid->offset_x = (scr_size_x - actual_grid_scr_size_x)/2;
	grid->offset_y = (scr_size_y - actual_grid_scr_size_y)/2;
}

void tl_resize_grid(Grid_t *grid, int new_offset_x, int new_offset_y, int new_scr_size_x, int new_scr_size_y, uint_t new_tile_width)
{
	uint_t new_width = new_scr_size_x / new_tile_width;
	uint_t new_tile_height = new_tile_width * grid->tile_h_to_w_ratio;
	uint_t new_height = new_scr_size_y / new_tile_height;
	uint_t new_size = new_width * new_height;
	
	printf("\nGrid resizing started: \n  w[%u -> %u]\n  [%u x %u] -> [%u x %u]\n  size[%u/%u]\n  screen: [%d x %d] -> [%d x %d]", 
		grid->tile_width, new_tile_width, grid_width(grid), grid_height(grid), new_width, new_height, new_size, grid->max_tile_count,
		grid->on_scr_size_x, grid->on_scr_size_y, new_scr_size_x, new_scr_size_y);

	if(new_size > grid->max_tile_count)
	{
	/*
		grid_w * grid_h <= MAX
		grid_w = scr_x / tile_w
		grid_h = scr_y / tile_h, tile_h = tile_w * r_t, = scr_y / (tile_w * r_t)
		(scr_x / tile_w) * (scr_y / (tile_w * r_t)) <= MAX
		(scr_x * scr_y) / (tile_w² * r_t) <= MAX		| * tile_w²
		(scr_x * scr_y) / r_t <= MAX * tile_w²			| / MAX
		(scr_x * scr_y) / (r_t * MAX) <= tile_w²		| sqrt()
		sqrt((scr_x * scr_y) / (r_t * MAX)) <= tile_w
		floor(sqrt((scr_x * scr_y) / (r_t * MAX))) ~ tile_w
		
		
		
	*/

		float r_t = grid->tile_h_to_w_ratio;
		float max = (float)grid->max_tile_count;
		new_tile_width = ceil(sqrt(((float)new_scr_size_x * (float)new_scr_size_y) / (r_t * max)));
	}

	grid->offset_x = new_offset_x;
	grid->offset_y = new_offset_y;
	grid->on_scr_size_x = new_scr_size_x;
	grid->on_scr_size_y = new_scr_size_y;
	grid->tile_width = new_tile_width;
	
	printf("\n  Grid Resized:\n  new tile_width: %u\n  new size: %d x %d -> %d tiles (MAX[%u]) - coords: (0-%d, 0-%d)\n",
	    		new_tile_width, grid_width(grid), grid_height(grid), grid_size(grid), grid->max_tile_count,
	    		grid_width(grid) - 1, grid_height(grid) - 1);
}

// x0 < x1
// x1 <= top grid width
void tl_fit_subgrid(Grid_t *top_grid, Grid_t *sub_grid, uint_t x0, uint_t y0, uint_t x1, uint_t y1)
{
	int new_offset_x = x0 * top_grid->tile_width + top_grid->offset_x;
	int new_offset_y = y0 * tile_height(top_grid) + top_grid->offset_y;
	int new_scr_size_x = (x1-x0) * top_grid->tile_width;
	int new_scr_size_y = (y1-y0) * tile_height(top_grid);
	tl_resize_grid(sub_grid, new_offset_x, new_offset_y, new_scr_size_x, new_scr_size_y, sub_grid->tile_width);
}

void tl_draw_tile(Grid_t *grid, uint_t x, uint_t y, char symbol, Color char_col, Color bg_col, Font *font)
{
	uint_t index = pos_to_i(grid, x, y);

	grid->symbols[index] = symbol;
	grid->symbol_colors[index] = char_col;
	grid->bg_colors[index] = bg_col;
	grid->fonts[index] = font;
}

void tl_draw_rect(Grid_t *grid, uint_t x0, uint_t y0, uint_t width, uint_t height, char symbol, Color char_col, Color bg_col, Font *font)
{
    for (uint_t _x = x0; _x <= x0 + width; _x++)
    {
        for (uint_t _y = y0; _y <= y0 + height; _y++)
        {
            tl_draw_tile(grid, _x, _y, symbol, char_col, bg_col, font);
        }
    }
}

void tl_draw_line(Grid_t *grid, uint_t x0, uint_t y0, uint_t x1, uint_t y1, char symbol, Color char_col, Color bg_col, Font *font)
{
    // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
    int dx = abs(x1 - x0);			// Don't believe the linter's lies, the abs() are needed!
    int sx = (x0 < x1)? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = (y0 < y1)? 1 : -1;
    int error = dx + dy;
    
    while (true)
    {
        tl_draw_tile(grid, x0, y0, symbol, char_col, bg_col, font);

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

// wrap = 0 => no wrapping
uint_t tl_draw_text(Grid_t * grid, uint_t x, uint_t y, uint_t wrap, char *text, uint_t len, Color char_col, Color bg_col, Font *font)
{
    uint_t _x = x;
    uint_t _y = y;

    for (uint_t i = 0; i < len; i++)
    {
        if(text[i] != '\n')
            tl_draw_tile(grid, _x, _y, text[i], char_col, bg_col, font);

        _x++;

        // Wrap around to start of x, on new line (y) and continue
        if ((_x >= wrap && wrap != 0) || text[i] == '\n') {_x = x; _y++;}
    }

    return _y - y + 1; // Rows written
}

Pos_t tl_screen_to_grid_coords(Grid_t *grid, Pos_t xy)
{
	xy.x = uclamp(grid->offset_x, xy.x, rendered_grid_size_x(grid)); 
	xy.y = uclamp(grid->offset_y, xy.y, rendered_grid_size_y(grid)); 
    return pos((xy.x - grid->offset_x) / grid->tile_width, (xy.y - grid->offset_y) / tile_height(grid));
}

void tl_set_tile_bg(Grid_t *grid, uint_t x, uint_t y, Color bg_col)
{
	grid->bg_colors[pos_to_i(grid, x, y)] = bg_col;
}

void tl_set_tile_char_col(Grid_t *grid, uint_t x, uint_t y, Color char_col)
{
	grid->symbol_colors[pos_to_i(grid, x, y)] = char_col;
}

void tl_tile_invert_colors(Grid_t *grid, uint_t x, uint_t y)
{
	uint_t index = pos_to_i(grid, x, y);
    Color tmp = grid->bg_colors[index];
    grid->bg_colors[index] = grid->symbol_colors[index];
    grid->symbol_colors[index] = tmp;
}

void tl_grid_set_txt_padding(Grid_t *grid, float pp)
{
	grid->txt_padding_prc_h = pp;
}
