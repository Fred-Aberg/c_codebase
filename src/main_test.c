#include <raylib.h>
#include "raytiles.h"
#include "common.h"
#include "ascui.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define DEF_COLOR (Color){20, 40, 29, 255}

void dropdown_button(void *domain, void *function_data, cursor_t *cursor)
{
	if (!cursor->left_button_pressed)
		return;
	container_t *container = (container_t *)domain; 
	container->open = !container->open;
}

uint8_t global_int = 15;
char global_str[INPUT_BUF_MAX_LEN] = "Hello...";

container_t *top_container;
container_t *subgrid_container;

void main_ui()
{
	container_style_t s_0 = style(0, col8bt(0,0,0), col8bt(5,0,2), col8bt(7,7,3), '=', '|', '+');
	container_style_t s_1 = style(1, col8bt(2,0,1), col8bt(2,0,1), col8bt(7,7,3), '-', '|', 'O');
	top_container = ascui_container(true, PERCENTAGE, 100, VERTICAL, 2,
		ascui_box(true, SELECTABLE, PERCENTAGE, 30, HORIZONTAL, s_0, 10, 
			ascui_text(true, STATIC, TILES, 2, strlen("\b000Tool\b \a700Box\a!"), "\b000Tool\b \a700Box\a!", s_1),
			ascui_text(true, STATIC, TILES, 2, strlen("\a700Test\a \b000Test\b \a003\b000Test\b\a"), "\a700Test\a \b000Test\b \a003\b000Test\b\a", s_1),
			ascui_text(true, SELECTABLE, TILES, 5, strlen("\b001Test\b Hello"), "\b001Test\b Hello", s_1),
			ascui_button(true, SELECTABLE, TILES, 5, strlen("S\a003ea\a"), "S\a003ea\a", s_1, NULL, NULL, NULL),
			ascui_button(true, HOVERABLE, TILES, 5, strlen("Grasslands"), "Grasslands", s_1, NULL, NULL, NULL),
			ascui_input(true, TILES, 3, s_1, S8_INT, -128, 127, &global_int),
			ascui_input(true, TILES, 3, s_1, STRING, 0, 64, &global_str),
			ascui_input_w_desc(true, TILES, 3, strlen("\n8bit: "), "\n8bit: ", s_1, S8_INT, -128, 127, &global_int),
			ascui_input_w_desc(true, TILES, 3, strlen("\nstr: "), "\nstr: ", s_1, STRING, 0, 64, &global_str),
			ascui_button(true, STATIC, TILES, 5, strlen("Mountains"), "Mountains", s_1, NULL, NULL, NULL)
		),
		(subgrid_container = ascui_subgrid(true, TILES, 1, NULL))
	);
}

int main(){

	printf("\nSTRUCT-SIZES [BYTES]:\n\t BG_INS=%lu\n\t SMBL_INS=%lu\n\t GEN_INS=%lu\n", sizeof(bg_instruction_t), sizeof(smbl_instruction_t), sizeof(instruction_t));

	double tl_rendering_time;
	double total_tl_rendering_time = 0;
	double ascui_drawing_time;
	double total_ascui_drawing_time = 0;
	double frame_time;
	double total_frame_time = 0;
	bool show_diagnostics = false;
	long n_smbl_draw_calls = 0;
	long n_bg_draw_calls = 0;
	long tick = 0;


   	int16_t screensize_x = 0;
	int16_t screensize_y = 0;
	uint16_t tile_width = 0;
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

    InitWindow(screensize_x, screensize_y, "ascui_constr_test");
    Font unscii = LoadFontEx("Resources/Fonts/unscii-8-alt.ttf", 32, 0, 256);
    Font unscii_fantasy = LoadFontEx("Resources/Fonts/unscii-8-fantasy.ttf", 32, 0, 256);

	Font fonts[2] = {unscii, unscii_fantasy};
    
    SetTargetFPS(60);
    HideCursor();
    SetWindowMinSize(200, 200);

	// Main Grid
	grid_t *main_grid = tl_init_grid(0, 0, screensize_x, screensize_y, tile_width, 1.0f, fonts, 200);
	grid_t *subgrid = tl_init_grid(0, 0, 100, 100, tile_width, 1.0f, fonts, 200);
	// UI
	main_ui();
	ascui_get_subgrid_data(subgrid_container)->subgrid = subgrid;
	cursor_t cursor; 
	ascui_print_ui(top_container);
	pos16_t mouse_subgrid_pos;


	
    while (!WindowShouldClose()){
    	tick++;
		frame_time = -GetTime();

        BeginDrawing();
        ClearBackground(BLACK);
        
		ascui_run_ui(main_grid, top_container, &ascui_drawing_time, NULL, NULL, 45, 47, &cursor);

		// printf("\nhov=[%p] sel[%p]", cursor.hovered_container, cursor.selected_container);

		pos16_t subgrid_size = tl_grid_get_dimensions(subgrid);
		tl_draw_rect_smbl_w_bg(subgrid, 0, 0, subgrid_size.x - 1, subgrid_size.y - 1, '.', col8bt(5, 1, 2), col8bt(3, 0, 1), 1);

		if(cursor.hovered_container == subgrid_container)
		{
			mouse_subgrid_pos = tl_screen_to_grid_coords(subgrid, pos16(GetMouseX(), GetMouseY()));
			tl_plot_smbl_w_bg(subgrid, mouse_subgrid_pos.x, mouse_subgrid_pos.y, 'A', WHITE8B, BLACK8B, 1);
		}
		else
			tl_plot_smbl_w_bg(main_grid, cursor.x, cursor.y, 'A', WHITE8B, BLACK8B, 0);

		tl_rendering_time = -GetTime();
		pos16_t main_grid_dcalls = tl_render_grid(main_grid);
		pos16_t sub_grid_dcalls = tl_render_grid(subgrid);
		tl_rendering_time += GetTime();

		frame_time += GetTime();

		n_bg_draw_calls = main_grid_dcalls.x + sub_grid_dcalls.x;
		n_smbl_draw_calls = main_grid_dcalls.y + sub_grid_dcalls.y;

		total_frame_time += frame_time;
		total_ascui_drawing_time += ascui_drawing_time;
		total_tl_rendering_time += tl_rendering_time;

		char buf[100];
		if(IsKeyPressed(KEY_TAB))
			show_diagnostics = !show_diagnostics;
		
		if (show_diagnostics)
		{
			DrawRectangle(0,0, 350, 550, c(40, 10, 40));
			sprintf(buf, "\n\tN draw calls: %li\n\n\t = [bg:%li + smbl:%li]", n_bg_draw_calls + n_smbl_draw_calls, n_bg_draw_calls, n_smbl_draw_calls);
			DrawText(buf, 0, 0, 24, c(255, 0, 255));
			sprintf(buf, "\n\tframe time [ms]:\n\n\t\t%f\n\n\t\tavg: %f", frame_time * 1000.0f, (total_frame_time * 1000.0f) / (double)tick);
			DrawText(buf, 0, 75, 24, c(255, 0, 255));
			sprintf(buf, "\n\tascui drawing time [ms]:\n\n\t\t%f\n\n\t\tavg: %f", ascui_drawing_time * 1000.0f, (total_ascui_drawing_time * 1000.0f) / (double)tick);
			DrawText(buf, 0, 200, 24, c(255, 0, 255));
			sprintf(buf, "\n\ttl drawing time [ms]:\n\n\t\t%f\n\n\t\tavg: %f", tl_rendering_time * 1000.0f, (total_tl_rendering_time * 1000.0f) / (double)tick);
			DrawText(buf, 0, 350, 24, c(255, 0, 255));
		}
		else
		{
			sprintf(buf, "(%u, %u)", cursor.x, cursor.y);
			DrawText(buf, 0, 0, 24, c(200, 0, 200));
			sprintf(buf, "(%u, %u)", mouse_subgrid_pos.x, mouse_subgrid_pos.y);
			DrawText(buf, 0, 48, 24, c(200, 0, 200));
	        DrawFPS(0,64);
		}

        EndDrawing();
    }
    UnloadFont(unscii);
    UnloadFont(unscii_fantasy);
    CloseWindow();
    ascui_destroy(top_container);
	tl_deinit_grid(main_grid);
    return 0;
}
