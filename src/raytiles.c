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

Tile_t *get_tile(Grid_t *grid, uint_t x, uint_t y)
{
	if(x >= grid_width(grid) && y >= grid_height(grid))
	{
    fprintf(stderr, "RAYTILES: get tile overflow (x,y)=(%u, %u) [w,h]=[%u, %u] i=%d\n", 
    		x, y, grid_width(grid), grid_height(grid), x + y * grid_width(grid));
    assert(false); // kys	
	}
    return &grid->tiles[x + y * grid_width(grid)];
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
	grid->max_tile_count = max_tile_count;
    grid->default_col = def_col;
    grid->default_font = def_font;

	// Which approach will result in fewer bugs?
	// assert(grid_size(grid) <= max_tile_count);
	grid->max_tile_count = umax(grid->max_tile_count, grid_size(grid));
	
    grid->tiles = calloc(grid->max_tile_count, sizeof(Tile_t));

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
    free(grid->tiles);
    free(grid);
}

void tl_render_grid(Grid_t *grid)
{
    Pos_t pos;
    Tile_t tile;
    Font *tile_font;

    uint_t tile_height = tile_height(grid);

    for (uint_t i = 0; i < grid_size(grid); i++)
    {
            tile = grid->tiles[i];
            pos = i_to_pos(grid, i);

            if (tile.bg_col.a != 0) 
					DrawRectangle(grid->offset_x + pos.x * grid->tile_width, grid->offset_y + pos.y * tile_height,
								 grid->tile_width, tile_height, tile.bg_col);
								 
            if (tile.symbol != 0 && tile.char_col.a != 0)
            {
            	int txt_padding = grid->tile_width * 0.15f; // 15% padding
                tile_font = (tile.font != NULL)? tile.font : grid->default_font;
                DrawTextCodepoint(*tile_font, tile.symbol, (Vector2){txt_padding + grid->offset_x + pos.x * grid->tile_width, 
                					grid->offset_y + pos.y * tile_height}, grid->tile_width, tile.char_col);
            }
    }
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
    Tile_t *tile = get_tile(grid, x, y);
    tile->bg_col = bg_col;
    tile->char_col = char_col;
    tile->symbol = symbol;
    tile->font = font;
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
    Tile_t *tile = get_tile(grid, x, y);
    tile->bg_col = bg_col;
}

void tl_set_tile_char_col(Grid_t *grid, uint_t x, uint_t y, Color char_col)
{
    Tile_t *tile = get_tile(grid, x, y);
    tile->char_col = char_col;
}

void tl_tile_invert_colors(Grid_t *grid, uint_t x, uint_t y)
{
	Tile_t *tile = get_tile(grid, x, y);
    Color tmp = tile->bg_col;
    tile->bg_col = tile->char_col;
    tile->char_col = tmp;
}
