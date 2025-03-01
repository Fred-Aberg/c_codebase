#include <raylib.h>
#include "raytiles.h"
#include "common.h"
#include "ascui.h"
#include <string.h>
#include <stdio.h>

#define DEF_COLOR (Color){20, 40, 29, 255}

void dropdown_button(void *domain, void *function_data, Cursor_t *cursor)
{
	if (!cursor->left_button_pressed)
		return;
	Container_t *container = (Container_t *)domain; 
	container->open = !container->open;
}

int main(){
   	int screensize_x = 0;
	int screensize_y = 0;
	uint_t tile_width = 0;
	char input[50];
 
   	printf("Screen width:\n");
   	fgets(input, sizeof(input), stdin);
   	screensize_x = atoi(input);

   	if (screensize_x == 0)
   	{
   		// Defaults
   		screensize_x = 1500;
   		screensize_y = 1000;
   		tile_width = 25;
   	}
   	else
   	{
	   	printf("Screen height:\n");
	   	fgets(input, sizeof(input), stdin);
	   	screensize_y = atoi(input);

	   	printf("Tile width:\n");
	   	fgets(input, sizeof(input), stdin);
	   	tile_width = atoi(input);

		if (screensize_x == 0 || screensize_y == 0 || tile_width == 0)
		{
			perror("Invalid inputs");
			return 0;
		}	
   	}
	


    InitWindow(screensize_x, screensize_y, "ascui_test");
    Font square_font = LoadFontEx("Resources/Fonts/Ac437_TridentEarly_8x8.ttf", 32, 0, 252);
    SetTargetFPS(60);
    HideCursor();
    SetWindowMinSize(200, 200);

	// Main Grid
	Grid_t *main_grid = tl_init_grid(0, 0, screensize_x, screensize_y, tile_width, 1.0f, CALCULATE_MAX_TILES, DEF_COLOR, &square_font);

	// UI
	Container_tag_list_t tag_list = ascui_tag_list_create(10);
	Container_t *top_container = ascui_construct_ui_from_file("src/test.ui", &tag_list);

	Cursor_t cursor; 

	ascui_tag_list_print(tag_list);
	ascui_print_ui(*top_container);
	
    while (!WindowShouldClose()){
		Pos_t mouse_scr_pos = pos(GetMouseX(), GetMouseY());

        BeginDrawing();
        ClearBackground(BLACK);

		if(IsKeyDown(45))
		{
			uint_t new_tile_size = main_grid->tile_width + 1;
			tl_resize_grid(main_grid, 0, 0, screensize_x, screensize_y, new_tile_size);
			tl_center_grid_on_screen(main_grid, screensize_x, screensize_y);
		}
		else if(IsKeyDown(47))
		{
			uint_t new_tile_size = main_grid->tile_width - 1;
			tl_resize_grid(main_grid, 0, 0, screensize_x, screensize_y, new_tile_size);
			tl_center_grid_on_screen(main_grid, screensize_x, screensize_y);
		}
		

		if(IsWindowResized())
		{
			screensize_x = GetScreenWidth();
			screensize_y = GetScreenHeight();
			
			tl_resize_grid(main_grid, 0, 0, screensize_x, screensize_y, main_grid->tile_width);
			tl_center_grid_on_screen(main_grid, screensize_x, screensize_y);
		}
		
		// Main grid
		Pos_t mouse_grid_pos = tl_screen_to_grid_coords(main_grid, mouse_scr_pos);
		cursor.x = mouse_grid_pos.x;
		cursor.y = mouse_grid_pos.y;
		cursor.right_button_pressed = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
		cursor.left_button_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
		cursor.scroll = GetMouseWheelMove();


		Pos_t main_grid_size = tl_grid_get_size(main_grid);
		tl_draw_rect(main_grid, 0,0, main_grid_size.x - 1, main_grid_size.y - 1, '.', c(20,20,20), c(0,0,0), NULL);

		ascui_draw_ui(main_grid, top_container, &cursor);
		if(cursor.selected_container != NULL)
		{
			if (cursor.selected_container->container_type == BUTTON)
			{
				Button_data_t *bt_data = ascui_get_button_data(*cursor.selected_container);
				bt_data->side_effect_func(bt_data->domain, bt_data->function_data, &cursor);
			}
			else if (!(cursor.scroll > 0 && cursor.selected_container->scroll_offset == 0))
				cursor.selected_container->scroll_offset -= cursor.scroll;
				
		}

		// printf("\n%p - SO: %u", cursor.selected_container, cursor.selected_container->scroll_offset);
	
		tl_draw_tile(main_grid, mouse_grid_pos.x, mouse_grid_pos.y, 'A', WHITE, DEF_COLOR, &square_font);
		
		tl_render_grid(main_grid);


		// Pos_t subgrid_size = tl_grid_get_size(subg_data->subgrid);
		// tl_draw_rect(subg_data->subgrid, 0,0, subgrid_size.x - 1, subgrid_size.y - 1, '-', c(200, 20, 20), c(100,10,10), NULL);
		// tl_render_grid(subg_data->subgrid);

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
    // ascui_destroy(top_container);
	tl_deinit_grid(main_grid);
    return 0;
}
