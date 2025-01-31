#include <raylib.h>
#include "raytiles.h"
#include "common.h"
#include "ascui.h"
#include <string.h>
#include <stdio.h>

#define DEF_COLOR (Color){20, 40, 29, 255}

#define MIN(a, b) (a < b)? a : b
#define MAX(a, b) (a < b)? b : a

int main(){
   	int screensize_x = 0;
	int screensize_y = 0;
	uint tile_width = 0;
	uint tile_height = 0;
	char input[50];
 
   	printf("Screen width:\n");
   	fgets(input, sizeof(input), stdin);
   	screensize_x = atoi(input);
	
   	printf("Screen height:\n");
   	fgets(input, sizeof(input), stdin);
   	screensize_y = atoi(input);

   	printf("Tile width:\n");
   	fgets(input, sizeof(input), stdin);
   	tile_width = atoi(input);

	printf("Tile height:\n");
   	fgets(input, sizeof(input), stdin);
	tile_height = atoi(input);

	if (screensize_x == 0 || screensize_y == 0 || tile_height == 0 || tile_width == 0)
	{
		perror("Invalid inputs");
		return 0;
	}


    InitWindow(screensize_x, screensize_y, "tl_test");
    Font square_font = LoadFontEx("Resources/Fonts/Ac437_TridentEarly_8x8.ttf", 32, 0, 252);
    SetTargetFPS(60);
    HideCursor();
    SetWindowMinSize(200, 200);

	// Main Grid
	Grid_t *main_grid = tl_init_grid(0, 0, screensize_x, screensize_y, tile_width, tile_height, DEF_COLOR, &square_font);

	// Subgrid
	Pos_t main_grid_size = tl_grid_get_size(main_grid);
	uint sub_tile_size = tile_width/2;
	
	Grid_t *sub_grid = tl_init_grid(0, 0, screensize_x, screensize_y, sub_tile_size, sub_tile_size, DEF_COLOR, &square_font);
	tl_fit_subgrid(main_grid, &sub_grid, main_grid_size.x / 2, main_grid_size.y / 3, main_grid_size.x - 2, main_grid_size.y - 2);
	Grid_t **active_grid = NULL;
	
    while (!WindowShouldClose()){
    	main_grid_size = tl_grid_get_size(main_grid);
		Vector2 mouse_scr_pos = (Vector2){GetMouseX(), GetMouseY()};

		if (mouse_scr_pos.x > sub_grid->offset_x && mouse_scr_pos.x < sub_grid->offset_x + sub_grid->on_scr_size_x
			 && mouse_scr_pos.y > sub_grid->offset_y && sub_grid->offset_y + sub_grid->on_scr_size_y)
			active_grid = &sub_grid;
		else
			active_grid = &main_grid;

        BeginDrawing();
        ClearBackground(BLACK);

		float scroll = GetMouseWheelMove();
		if (scroll != 0)
		{
			uint new_tile_size = uclamp(8, (*active_grid)->tile_width + scroll, screensize_x/10);
			tl_resize_grid(active_grid, (*active_grid)->offset_x, (*active_grid)->offset_y, 
							(*active_grid)->on_scr_size_x, (*active_grid)->on_scr_size_y, new_tile_size, new_tile_size);
			if (*active_grid == main_grid)
			{
				main_grid_size = tl_grid_get_size(main_grid);
				tl_fit_subgrid(main_grid, &sub_grid, main_grid_size.x / 2, main_grid_size.y / 3, main_grid_size.x - 2, main_grid_size.y - 2);
			}
		}

		if(IsWindowResized())
		{
			screensize_x = GetScreenWidth();
			screensize_y = GetScreenHeight();
			
			tl_resize_grid(&main_grid, 0, 0, screensize_x, screensize_y, tile_width, tile_height);

			main_grid_size = tl_grid_get_size(main_grid);
			tl_fit_subgrid(main_grid, &sub_grid, main_grid_size.x / 2, main_grid_size.y / 3, main_grid_size.x - 2, main_grid_size.y - 2);
		}
		
		// Main grid
		Pos_t grid_size = tl_grid_get_size(main_grid);
		tl_draw_rect(main_grid, 0, 0, grid_size.x - 1, grid_size.y - 1, '.', (Color){70, 30, 50, 255}, (Color){50, 10, 30, 255}, NULL);
		tl_draw_line(main_grid, 0, 0, grid_size.x - 1, grid_size.y - 1, 'X', (Color){0, 200, 180, 255}, (Color){70, 30, 50, 255}, NULL);
		tl_draw_line(main_grid, 0, grid_size.y - 1, grid_size.x - 1, 0, 'X', (Color){0, 200, 180, 255}, (Color){70, 30, 50, 255}, NULL);

		// Sub grid
		grid_size = tl_grid_get_size(sub_grid);
		tl_draw_rect(sub_grid, 0, 0, grid_size.x - 1, grid_size.y - 1, '*', (Color){70, 60, 70, 255}, (Color){50, 30, 50, 255}, NULL);

		// Active grid
		Pos_t mouse_grid_pos = tl_screen_to_grid_coords(*active_grid, pos(mouse_scr_pos.x, mouse_scr_pos.y));
		tl_draw_tile(*active_grid, mouse_grid_pos.x, mouse_grid_pos.y, 'A', WHITE, DEF_COLOR, &square_font);
		
		tl_render_grid(main_grid);
		tl_render_grid(sub_grid);

		char buf[50];
		sprintf(buf, "(%u, %u)", mouse_grid_pos.x, mouse_grid_pos.y);
		DrawText(buf, 0, 0, 24, WHITE);
        
		sprintf(buf, "%f", scroll);
		DrawText(buf, 0, 32, 24, WHITE);

		
        DrawFPS(0,64);
        EndDrawing();
    }
    UnloadFont(square_font);
    CloseWindow();
	tl_deinit_grid(main_grid);
    return 0;
}
