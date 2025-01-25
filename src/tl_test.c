#include <raylib.h>
#include "raytiles.h"
#include <string.h>
#include <stdio.h>

#define DEF_COLOR (Color){20, 40, 29, 255}

// main SCREENSIZE_X SCREENSIZE_Y



int main(){
   	int screensize_x = 0;
	int screensize_y = 0;
	int tile_width = 0;
	int tile_height = 0;
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

	Grid_t *main_grid = tl_init_grid(0, 0, screensize_x, screensize_y, tile_width, tile_height, DEF_COLOR, &square_font);
	
    while (!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(BLACK);

		if(IsWindowResized())
			tl_resize_grid(main_grid, GetScreenWidth(), GetScreenHeight());

		Pos_t grid_size = tl_grid_get_size(main_grid);
		tl_draw_rect(main_grid, 0, 0, grid_size.x - 1, grid_size.y - 1, '.', (Color){70, 30, 50, 255}, (Color){50, 10, 30, 255}, NULL);
		tl_draw_line(main_grid, 0, 0, grid_size.x - 1, grid_size.y - 1, 'X', (Color){0, 200, 180, 255}, (Color){70, 30, 50, 255}, NULL);
		tl_draw_line(main_grid, 0, grid_size.y - 1, grid_size.x - 1, 0, 'X', (Color){0, 200, 180, 255}, (Color){70, 30, 50, 255}, NULL);

		Pos_t mouse_scr_pos = (Pos_t){GetMouseX(), GetMouseY()};
		Pos_t mouse_grid_pos = tl_screen_to_grid_coords(main_grid, mouse_scr_pos);

		tl_draw_line(main_grid, grid_size.x / 2, grid_size.y / 2, mouse_grid_pos.x, mouse_grid_pos.y, 'X', (Color){0, 200, 180, 255}, (Color){70, 30, 50, 255}, NULL);

		tl_draw_tile(main_grid, mouse_grid_pos.x, mouse_grid_pos.y, 'A', WHITE, DEF_COLOR, &square_font);
		
		tl_render_grid(main_grid);

		char buf[50];
		sprintf(buf, "(%u, %u)", mouse_grid_pos.x, mouse_grid_pos.y);
		DrawText(buf, 0, 0, 24, WHITE);
        
        DrawFPS(0,48);
        EndDrawing();
    }
    UnloadFont(square_font);
    CloseWindow();
	tl_deinit_grid(main_grid);
    return 0;
}
