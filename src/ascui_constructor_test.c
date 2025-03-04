#include <raylib.h>
#include "raytiles.h"
#include "common.h"
#include "ascui.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define DEF_COLOR (Color){20, 40, 29, 255}

void dropdown_button(void *domain, void *function_data, Cursor_t *cursor)
{
	if (!cursor->left_button_pressed)
		return;
	Container_t *container = (Container_t *)domain; 
	container->open = !container->open;
}

int main(){
	double tl_rendering_time;
	double total_tl_rendering_time = 0;
	double ascui_drawing_time;
	double total_ascui_drawing_time = 0;
	double frame_time;
	double total_frame_time = 0;
	bool show_diagnostics = false;
	long n_draw_calls = 0;
	long tick = 0;


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
	Grid_t *subgrid;
	// UI
	Container_tag_list_t tag_list = ascui_tag_list_create(10);
	Container_t *top_container = ascui_construct_ui_from_file("src/test.ui", &tag_list);

	Container_t *subgrid_container = ascui_tag_list_get(tag_list, "map_view");
	subgrid = ascui_get_subgrid_data(*subgrid_container)->subgrid;

	subgrid->default_font = &square_font;
	
	Cursor_t cursor; 

	ascui_tag_list_print(tag_list);
	ascui_print_ui(*top_container);
	
    while (!WindowShouldClose()){
    	tick++;
		frame_time = -GetTime();
    
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

		ascui_drawing_time = -GetTime();
		ascui_draw_ui(main_grid, top_container, &cursor);
		ascui_drawing_time += GetTime();

		if(cursor.hovered_container != NULL)
		{
			if (cursor.hovered_container->container_type == BUTTON)
			{
				Button_data_t *bt_data = ascui_get_button_data(*cursor.hovered_container);
				if(bt_data->side_effect_func)
					bt_data->side_effect_func(bt_data->domain, bt_data->function_data, &cursor);
			}
			else if (!(cursor.scroll > 0 && cursor.hovered_container->scroll_offset == 0))
				cursor.hovered_container->scroll_offset -= cursor.scroll;

			// Select
			if(cursor.hovered_container->container_type == BUTTON && cursor.left_button_pressed)
				cursor.selected_container = cursor.hovered_container;
			// Deselect
			if(cursor.hovered_container == cursor.selected_container && cursor.right_button_pressed)
				cursor.selected_container = NULL;
		}
		printf("\nhov=[%p] sel[%p]", cursor.hovered_container, cursor.selected_container);

		Pos_t subgrid_size = tl_grid_get_size(subgrid);
		tl_draw_rect(subgrid, 0,0, subgrid_size.x - 1, subgrid_size.y - 1, '-', c(200, 20, 20), c(100,10,10), NULL);

		if(cursor.hovered_container == subgrid_container)
		{
			mouse_grid_pos = tl_screen_to_grid_coords(subgrid, mouse_scr_pos);
			tl_draw_tile(subgrid, mouse_grid_pos.x, mouse_grid_pos.y, 'A', WHITE, DEF_COLOR, &square_font);
		}
		else
			tl_draw_tile(main_grid, mouse_grid_pos.x, mouse_grid_pos.y, 'A', WHITE, DEF_COLOR, &square_font);

		tl_rendering_time = -GetTime();
		n_draw_calls = tl_render_grid(main_grid) + tl_render_grid(subgrid);
		tl_rendering_time += GetTime();

		frame_time += GetTime();

		total_frame_time += frame_time;
		total_ascui_drawing_time += ascui_drawing_time;
		total_tl_rendering_time += tl_rendering_time;

		char buf[100];
		if(IsKeyPressed(KEY_TAB))
			show_diagnostics = !show_diagnostics;
		
		if (show_diagnostics)
		{
			DrawRectangle(0,0, 350, 550, c(40, 10, 40));
			sprintf(buf, "\n\tN draw calls: %li", n_draw_calls);
			DrawText(buf, 0, 0, 24, c(255, 0, 255));
			sprintf(buf, "\n\tframe time [ms]:\n\n\t\t%f\n\n\t\tavg: %f", frame_time * 1000.0f, (total_frame_time * 1000.0f) / (double)tick);
			DrawText(buf, 0, 150, 24, c(255, 0, 255));
			sprintf(buf, "\n\tascui drawing time [ms]:\n\n\t\t%f\n\n\t\tavg: %f", ascui_drawing_time * 1000.0f, (total_ascui_drawing_time * 1000.0f) / (double)tick);
			DrawText(buf, 0, 300, 24, c(255, 0, 255));
			sprintf(buf, "\n\ttl drawing time [ms]:\n\n\t\t%f\n\n\t\tavg: %f", tl_rendering_time * 1000.0f, (total_tl_rendering_time * 1000.0f) / (double)tick);
			DrawText(buf, 0, 450, 24, c(255, 0, 255));
		}
		else
		{
			sprintf(buf, "(%u, %u)", mouse_grid_pos.x, mouse_grid_pos.y);
			DrawText(buf, 0, 0, 24, c(200, 0, 200));
	        DrawFPS(0,64);
		}

        EndDrawing();
    }
    UnloadFont(square_font);
    CloseWindow();
    // ascui_destroy(top_container);
	tl_deinit_grid(main_grid);
    return 0;
}
