#include <raylib.h>
#include "raytiles.h"
#include "math.h"
#include "stdio.h"
#include "assert.h"
#include "string.h"

#define MIN(a, b) (a < b)? a : b
#define grid_width(grid) ((grid->on_scr_size_x) / grid->tile_width)
#define grid_height(grid) ((grid->on_scr_size_y) / grid->tile_height)
#define grid_size(grid) (grid_width(grid) * grid_height(grid))

Tile_t *get_tile(Grid_t *grid, uint x, uint y)
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
    uint y = i / grid_width(grid); // C autimatically floors the result
    uint x = i - (y * grid_width(grid));
	return (Pos_t){.x = x, .y = y};
}

Grid_t *tl_init_grid(int offset_x, int offset_y, int on_scr_size_x, int on_scr_size_y, 
					uint tile_width, uint tile_height, Color def_col, Font *def_font)
{
    Grid_t *grid = calloc(1, sizeof(Grid_t));

    grid->offset_x = offset_x;
    grid->offset_y = offset_y;
    grid->on_scr_size_x = on_scr_size_x;
    grid->on_scr_size_y = on_scr_size_y;
	grid->tile_width = tile_width;
	grid->tile_height = tile_height;
    grid->default_col = def_col;
    grid->default_font = def_font;

    grid->tiles = calloc(grid_size(grid), sizeof(Tile_t));

    // fprintf(stderr, "tl_grid initialized - size: %d x %d -> %d tiles - coords: (0-%d, 0-%d)\n",
    		// grid_width(grid), grid_height(grid), grid_size(grid),
    		// grid_width(grid) - 1, grid_height(grid) - 1);
    
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

    for (uint i = 0; i < grid_size(grid); i++)
    {
            tile = grid->tiles[i];
            pos = i_to_pos(grid, i);

			DrawRectangle(grid->offset_x + pos.x * grid->tile_width, grid->offset_y + pos.y * grid->tile_height,
						 grid->tile_width, grid->tile_height, tile.bg_col);
            // if (tile.bg_col.a != 0) DrawRectangle(x * X_TL_SIZE, y * Y_TL_SIZE, X_TL_SIZE, Y_TL_SIZE, tile.bg_col);
            // else                    DrawRectangle(x * X_TL_SIZE, y * Y_TL_SIZE, X_TL_SIZE, Y_TL_SIZE, GRID->default_col);

            if (tile.symbol != 0 && tile.char_col.a != 0)
            {
            	int txt_padding = grid->tile_width * 0.15f; // 15% padding
                tile_font = (tile.font != NULL)? tile.font : grid->default_font;
                DrawTextCodepoint(*tile_font, tile.symbol, (Vector2){txt_padding + grid->offset_x + pos.x * grid->tile_width, 
                					grid->offset_y + pos.y * grid->tile_height}, grid->tile_width, tile.char_col);
            }
    }
}

void tl_resize_grid(Grid_t **grid, int new_offset_x, int new_offset_y, int new_scr_size_x, int new_scr_size_y, uint new_tile_width, uint new_tile_height)
{
	Grid_t *old_grid = *grid;
	Grid_t *new_grid = tl_init_grid(new_offset_x, new_offset_y, new_scr_size_x, new_scr_size_y,
									new_tile_width, new_tile_height, old_grid->default_col, old_grid->default_font);
	tl_deinit_grid(*grid);
	*grid = new_grid;
}

// x0 < x1
// x1 <= top grid width
void tl_fit_subgrid(Grid_t *top_grid, Grid_t **sub_grid, uint x0, uint y0, uint x1, uint y1)
{
	int new_offset_x = x0 * top_grid->tile_width + top_grid->offset_x;
	int new_offset_y = y0 * top_grid->tile_height + top_grid->offset_y;
	int new_scr_size_x = (x1-x0) * top_grid->tile_width;
	int new_scr_size_y = (y1-y0) * top_grid->tile_height;
	tl_resize_grid(sub_grid, new_offset_x, new_offset_y, new_scr_size_x, new_scr_size_y, (*sub_grid)->tile_width, (*sub_grid)->tile_height);
}

void tl_draw_tile(Grid_t *grid, uint x, uint y, char symbol, Color char_col, Color bg_col, Font *font)
{
    Tile_t *tile = get_tile(grid, x, y);
    tile->bg_col = bg_col;
    tile->char_col = char_col;
    tile->symbol = symbol;
    tile->font = font;
}

void tl_draw_rect(Grid_t *grid, uint x0, uint y0, uint width, uint height, char symbol, Color char_col, Color bg_col, Font *font)
{
    for (uint _x = x0; _x <= x0 + width; _x++)
    {
        for (uint _y = y0; _y <= y0 + height; _y++)
        {
            tl_draw_tile(grid, _x, _y, symbol, char_col, bg_col, font);
        }
    }
}

void tl_draw_line(Grid_t *grid, uint x0, uint y0, uint x1, uint y1, char symbol, Color char_col, Color bg_col, Font *font)
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
uint tl_draw_text(Grid_t * grid, uint x, uint y, uint wrap, char *text, uint len, Color char_col, Color bg_col, Font *font)
{
    uint _x = x;
    uint _y = y;

    for (uint i = 0; i < len; i++)
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
    return pos((xy.x - grid->offset_x) / grid->tile_width, (xy.y - grid->offset_y) / grid->tile_height);
}

void tl_set_tile_bg(Grid_t *grid, uint x, uint y, Color bg_col)
{
    Tile_t *tile = get_tile(grid, x, y);
    tile->bg_col = bg_col;
}
