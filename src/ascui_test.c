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

   	if (screensize_x == 0)
   	{
   		// Defaults
   		screensize_x = 1000;
   		screensize_y = 500;
   		tile_width = 50;
   		tile_height = 50;
   	}
   	else
   	{
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
   	}
	


    InitWindow(screensize_x, screensize_y, "tl_test");
    Font square_font = LoadFontEx("Resources/Fonts/Ac437_TridentEarly_8x8.ttf", 32, 0, 252);
    SetTargetFPS(60);
    HideCursor();
    SetWindowMinSize(200, 200);

	// Main Grid
	Grid_t *main_grid = tl_init_grid(0, 0, screensize_x, screensize_y, tile_width, tile_height, DEF_COLOR, &square_font);

	// UI
	Container_style_t style1 = style(c(20,5,10), c(80, 20, 40), c(200, 50, 100), '=', '|', '+');
	Container_style_t style2 = style(c(10,50,10), c(40, 60, 40), c(20, 255, 20), '~', 'I', 'O');
	Container_t top_container = ascui_create_container(true, PERCENTAGE, 100, VERTICAL, 2);
	// Container_t top_box = ascui_create_box(true, PERCENTAGE, 100, false, VERTICAL, 0, style);

	
	ascui_set_nth_subcontainer(top_container, 0, ascui_create_box(true, PERCENTAGE, 50, VERTICAL, 2, style2));
	ascui_set_nth_subcontainer(top_container, 1, ascui_create_container(true, PERCENTAGE, 50, HORIZONTAL, 2));

	Container_t *first_box = ascui_get_nth_subcontainer(top_container, 0);
	ascui_set_nth_subcontainer(*first_box, 0, ascui_create_box(true, PERCENTAGE, 10, HORIZONTAL, 0, style1));
	char *txt = "I like to creep around my home and act like a goblin\n\nI don’t know why but I just enjoy doing this. Maybe it’s my way of dealing with stress or something but I just do it about once every week. Generally I’ll carry around a sack and creep around in a sort of crouch-walking position making goblin noises, then I’ll walk around my house and pick up various different “trinkets” and put them in my bag while saying stuff like “I’ll be having that” and laughing maniacally in my goblin voice (“trinkets” can include anything from shit I find on the ground to cutlery or other utensils). The other day I was talking with my neighbours and they mentioned hearing weird noises like what I wrote about and I was just internally screaming the entire conversation. I’m 99% sure they don’t know it’s me but god that 1% chance is seriously weighing on my mind.";
	
	ascui_set_nth_subcontainer(*first_box, 1, ascui_create_text(true, PERCENTAGE, 90, strlen(txt), txt, c(0,200,0), c(30,60,30)));
	
	Container_t *second_container = ascui_get_nth_subcontainer(top_container, 1);
	ascui_set_nth_subcontainer(*second_container, 0, ascui_create_container(true, PERCENTAGE, 25, VERTICAL, 2));
	ascui_set_nth_subcontainer(*second_container, 1, ascui_create_container(true, PERCENTAGE, 75, VERTICAL, 1));

	Container_t *left_container = ascui_get_nth_subcontainer(*second_container, 0);
	ascui_set_nth_subcontainer(*left_container, 0, ascui_create_box(true, PERCENTAGE, 50, HORIZONTAL, 0, style1));
	ascui_set_nth_subcontainer(*left_container, 1, ascui_create_box(true, PERCENTAGE, 50, HORIZONTAL, 0, style2));
	Container_t *right_container = ascui_get_nth_subcontainer(*second_container, 1);
	ascui_set_nth_subcontainer(*right_container, 0, ascui_create_subgrid(true, PERCENTAGE, 100, NULL, main_grid->default_col, main_grid->default_font));

	Container_t *subgrid_c = ascui_get_nth_subcontainer(*right_container, 0);
	Subgrid_data_t *subg_data = ascui_get_subgrid_data(*subgrid_c);
	

	Cursor_t cursor; 
	
    while (!WindowShouldClose()){
		Vector2 mouse_scr_pos = (Vector2){GetMouseX(), GetMouseY()};

        BeginDrawing();
        ClearBackground(BLACK);

		if(IsKeyDown(45))
		{
			uint new_tile_size = uclamp(8, main_grid->tile_width + 1, screensize_x/10);
			tl_resize_grid(&main_grid, main_grid->offset_x, main_grid->offset_y, 
							main_grid->on_scr_size_x, main_grid->on_scr_size_y, new_tile_size, new_tile_size);
		}
		else if(IsKeyDown(47))
		{
			uint new_tile_size = uclamp(8, main_grid->tile_width - 1, screensize_x/10);
			tl_resize_grid(&main_grid, main_grid->offset_x, main_grid->offset_y, 
							main_grid->on_scr_size_x, main_grid->on_scr_size_y, new_tile_size, new_tile_size);
		}
		

		if(IsWindowResized())
		{
			screensize_x = GetScreenWidth();
			screensize_y = GetScreenHeight();
			
			tl_resize_grid(&main_grid, 0, 0, screensize_x, screensize_y, main_grid->tile_width, main_grid->tile_height);
		}
		
		// Main grid
		Pos_t mouse_grid_pos = tl_screen_to_grid_coords(main_grid, pos(mouse_scr_pos.x, mouse_scr_pos.y));
		cursor.x = mouse_grid_pos.x;
		cursor.y = mouse_grid_pos.y;
		cursor.right_button_pressed = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
		cursor.left_button_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
		cursor.scroll = GetMouseWheelMove();
		
		ascui_draw_ui(main_grid, &top_container, &cursor);

		printf("\n%p", cursor.selected_container);
	
		tl_draw_tile(main_grid, mouse_grid_pos.x, mouse_grid_pos.y, 'A', WHITE, DEF_COLOR, &square_font);
		
		tl_render_grid(main_grid);


		Pos_t subgrid_size = tl_grid_get_size(subg_data->subgrid);
		tl_draw_rect(subg_data->subgrid, 0,0, subgrid_size.x - 1, subgrid_size.y - 1, '-', c(200, 20, 20), c(100,10,10), NULL);
		tl_render_grid(subg_data->subgrid);

		char buf[50];
		sprintf(buf, "(%u, %u)", mouse_grid_pos.x, mouse_grid_pos.y);
		DrawText(buf, 0, 0, 24, c(200, 0, 200));

		// int charp = GetCharPressed();
		// int keyp = GetKeyPressed();
// 
		// if (charp != 0 || keyp != 0)
			// printf("\n(c[%d], k[%d])", charp, keyp);
		
        DrawFPS(0,64);
        EndDrawing();
    }
    UnloadFont(square_font);
    CloseWindow();
	tl_deinit_grid(main_grid);
    return 0;
}
