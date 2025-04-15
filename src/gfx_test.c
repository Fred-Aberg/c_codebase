#include <raylib.h>
#include "raytiles.h"
#include "common.h"
#include "ascui.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "texts.c"

#define DEF_COLOR (Color){20, 40, 29, 255}

void dropdown_button(void *domain, void *function_data, cursor_t *cursor)
{
	if (!cursor->left_button_pressed)
		return;
	container_t *container = (container_t *)domain; 
	container->open = !container->open;
}

context_t *c_ctx;

void navigate_button(void *domain, void *function_data, cursor_t *cursor)
{
	if (!cursor->left_button_pressed)
		return;
	c_ctx = *(context_t **)domain; 
}

uint8_t global_int = 15;
char global_str[INPUT_BUF_MAX_LEN] = "Hello...";

context_t *main_ctx;
context_t *text_ctx;
context_t *input_ctx;



container_t *subgrid_container;

void main_ui()
{
	
	container_style_t s_0 = style(0, col8bt(0,0,0), col8bt(5,0,2), col8bt(7,7,3), '=', '|', '+');
	container_style_t s_1 = style(1, col8bt(2,0,1), col8bt(2,0,1), col8bt(7,7,3), '-', '|', 'O');
	main_ctx = ascui_context(0,
		ascui_container(true, PERCENTAGE, 100, VERTICAL, 1,
			ascui_box(true, HOVERABLE, TILES, 1, HORIZONTAL, s_0, 3, 
				ascui_button(true, HOVERABLE, TILES, 3, str("Text Examples >"), ALIGN_MIDDLE, ALIGN_MIDDLE, s_1, navigate_button, &text_ctx, NULL),
				ascui_button(true, HOVERABLE, TILES, 3, str("Input Examples >"), ALIGN_MIDDLE, ALIGN_MIDDLE, s_1, navigate_button, &input_ctx, NULL),
				ascui_divider(s_1)
			)
		)
	);
}

var_binding_t *text_index_binding;
var_binding_t *text_ptr_binding;
void text_example_ui()
{
	create_example_strings();

	container_style_t s_1 = style(1, col8bt(2,0,1), col8bt(2,0,1), col8bt(7,7,3), '-', '|', 'O');
	container_style_t s_title = style(1, col8bt(4,0,2), col8bt(2,0,1), col8bt(0,0,0), '-', '|', 'O');
	container_style_t s_desc = style(0, col8bt(2,0,1), col8bt(2,0,1), col8bt(7,7,3), '-', '|', 'O');
	container_style_t s_text = style(0, col8bt(4,0,2), col8bt(2,0,1), col8bt(7,7,3), '-', '|', 'O');

	text_ctx = ascui_context(2, NULL); // Create context
	
	// Add variable bindings before containers because of param. execution sequencing not necessarily being from left to right
	text_index_binding = ascui_add_context_var_binding(text_ctx, 0);
	text_ptr_binding = ascui_add_context_var_binding(text_ctx, (intptr_t)example_strings[0]);
	
	text_ctx->top_container = ascui_box(true, HOVERABLE, PERCENTAGE, 100, HORIZONTAL, s_1, 14,
		ascui_container(true, TILES, 3, VERTICAL, 4,
			ascui_button(true, HOVERABLE, TILES, 7, str("< BACK"), ALIGN_MIDDLE, ALIGN_MIDDLE, s_1, navigate_button, &main_ctx, NULL),
			ascui_divider(s_desc),
			ascui_text(true, STATIC, TILES, 15, str("TEXT EXAMPLES"), ALIGN_MIDDLE, ALIGN_MIDDLE, s_title),
			ascui_input_w_desc(true, TILES, 8, str(" TXT: "), ALIGN_RIGHT, ALIGN_MIDDLE, TILES, 1, s_1, U16_INT, 0, N_STRINGS - 1, text_index_binding)
		),
		
		ascui_divider(s_1),
		ascui_container(true, TILES, 7, VERTICAL, 2,
			ascui_text(true, STATIC, TILES, 17, str("h:LEFT-v:TOP"), ALIGN_MIDDLE, ALIGN_MIDDLE, s_desc),
			ascui_display(true, HOVERABLE, TILES, 1, STRING, str("%s"), text_ptr_binding, ALIGN_LEFT, ALIGN_TOP, s_text)
		),
		ascui_container(true, TILES, 7, VERTICAL, 2,
			ascui_text(true, STATIC, TILES, 17, str("h:MIDDLE-v:TOP"), ALIGN_MIDDLE, ALIGN_MIDDLE, s_text),
			ascui_display(true, HOVERABLE, TILES, 1, STRING, str("%s"), text_ptr_binding, ALIGN_MIDDLE, ALIGN_TOP, s_desc)
		),
		ascui_container(true, TILES, 7, VERTICAL, 2,
			ascui_text(true, STATIC, TILES, 17, str("h:RIGHT-v:TOP"), ALIGN_MIDDLE, ALIGN_MIDDLE, s_desc),
			ascui_display(true, HOVERABLE, TILES, 1, STRING, str("%s"), text_ptr_binding, ALIGN_RIGHT, ALIGN_TOP, s_text)
		),
		
		ascui_divider(s_1),
		ascui_container(true, TILES, 7, VERTICAL, 2,
			ascui_text(true, STATIC, TILES, 17, str("h:LEFT-v:MIDDLE"), ALIGN_MIDDLE, ALIGN_MIDDLE, s_desc),
			ascui_display(true, HOVERABLE, TILES, 1, STRING, str("%s"), text_ptr_binding, ALIGN_LEFT, ALIGN_MIDDLE, s_text)
		),
		ascui_container(true, TILES, 7, VERTICAL, 2,
			ascui_text(true, STATIC, TILES, 17, str("h:MIDDLE-v:MIDDLE"), ALIGN_MIDDLE, ALIGN_MIDDLE, s_text),
			ascui_display(true, HOVERABLE, TILES, 1, STRING, str("%s"), text_ptr_binding, ALIGN_MIDDLE, ALIGN_MIDDLE, s_desc)
		),
		ascui_container(true, TILES, 7, VERTICAL, 2,
			ascui_text(true, STATIC, TILES, 17, str("h:RIGHT-v:MIDDLE"), ALIGN_MIDDLE, ALIGN_MIDDLE, s_desc),
			ascui_display(true, HOVERABLE, TILES, 1, STRING, str("%s"), text_ptr_binding, ALIGN_RIGHT, ALIGN_MIDDLE, s_text)
		),

		ascui_divider(s_1),
		ascui_container(true, TILES, 7, VERTICAL, 2,
			ascui_text(true, STATIC, TILES, 17, str("h:LEFT-v:BOTTOM"), ALIGN_MIDDLE, ALIGN_MIDDLE, s_desc),
			ascui_display(true, HOVERABLE, TILES, 1, STRING, str("%s"), text_ptr_binding, ALIGN_LEFT, ALIGN_BOTTOM, s_text)
		),
		ascui_container(true, TILES, 7, VERTICAL, 2,
			ascui_text(true, STATIC, TILES, 17, str("h:MIDDLE-v:BOTTOM"), ALIGN_MIDDLE, ALIGN_MIDDLE, s_text),
			ascui_display(true, HOVERABLE, TILES, 1, STRING, str("%s"), text_ptr_binding, ALIGN_MIDDLE, ALIGN_BOTTOM, s_desc)
		),
		ascui_container(true, TILES, 7, VERTICAL, 2,
			ascui_text(true, STATIC, TILES, 17, str("h:RIGHT-v:BOTTOM"), ALIGN_MIDDLE, ALIGN_MIDDLE, s_desc),
			ascui_display(true, HOVERABLE, TILES, 1, STRING, str("%s"), text_ptr_binding, ALIGN_RIGHT, ALIGN_BOTTOM, s_text)
		),
		ascui_divider(s_1)
	);

}

bool example_bool = false;
str_t *example_str;


void input_example_ui()
{
	
	container_style_t s_1 = style(1, col8bt(2,0,1), col8bt(2,0,1), col8bt(7,7,3), '-', '|', 'O');
	container_style_t s_title = style(1, col8bt(4,0,2), col8bt(2,0,1), col8bt(0,0,0), '-', '|', 'O');
	container_style_t s_desc = style(0, col8bt(2,0,1), col8bt(2,0,1), col8bt(7,7,3), '-', '|', 'O');
	container_style_t s_text = style(0, col8bt(4,0,2), col8bt(2,0,1), col8bt(7,7,3), '-', '|', 'O');
	container_style_t s_on = style(0, col8bt(2,0,1), col8bt(4,0,2), col8bt(7,7,3), '-', '|', '+');
	container_style_t s_off = style(0, col8bt(0,0,0), col8bt(2,0,1), col8bt(7,7,3), ' ', ' ', ' ');
	container_style_t s_slider = style(0, col8bt(2,0,1), col8bt(4,0,2), col8bt(7,7,3), ' ', ' ', ' ');

	input_ctx = ascui_context(4, NULL);
	var_binding_t *str_binding = ascui_add_context_var_binding(input_ctx, (intptr_t)str_empty(INPUT_BUF_MAX_LEN));
	var_binding_t *uint_binding = ascui_add_context_var_binding(input_ctx, 1);
	var_binding_t *int_binding = ascui_add_context_var_binding(input_ctx, -1);
	var_binding_t *float_binding = ascui_add_context_var_binding_float(input_ctx, 0.5f);

	input_ctx->top_container = 
	ascui_box(true, HOVERABLE, PERCENTAGE, 100, HORIZONTAL, s_1, 14,
		ascui_container(true, TILES, 3, VERTICAL, 3,
			ascui_button(true, HOVERABLE, TILES, 7, str("< BACK"), ALIGN_MIDDLE, ALIGN_MIDDLE, s_1, navigate_button, &main_ctx, NULL),
			ascui_divider(s_desc),
			ascui_text(true, STATIC, TILES, 1, str("INPUT EXAMPLES"), ALIGN_MIDDLE, ALIGN_MIDDLE, s_title)
		),
		ascui_divider(s_desc),
		ascui_dropdown_button(TILES, 3, str("^ Dropdown test"), ALIGN_LEFT, ALIGN_MIDDLE, s_1),
		ascui_box(false, HOVERABLE, TILES, 10, HORIZONTAL, s_1, 3,
			ascui_text(true, STATIC, TILES, 3, str("item 1"), ALIGN_MIDDLE, ALIGN_MIDDLE, s_title),
			ascui_text(true, STATIC, TILES, 3, str("item 2"), ALIGN_MIDDLE, ALIGN_MIDDLE, s_desc),
			ascui_text(true, STATIC, TILES, 3, str("item 3"), ALIGN_MIDDLE, ALIGN_MIDDLE, s_text)
		),
		ascui_container(true, TILES, 3, VERTICAL, 2,
			ascui_toggle(&example_bool, s_on, s_off),
			ascui_text(true, STATIC, TILES, 1, str("Toggle button"), ALIGN_LEFT, ALIGN_MIDDLE, s_desc)
		),
		ascui_input_w_desc(true, TILES, 15, str("uint [0-69]: "), ALIGN_RIGHT, ALIGN_MIDDLE, TILES, 3, s_1, U16_INT, 0, 69, uint_binding),
		ascui_slider(true, TILES, 3, s_slider, U16_INT, 0, 69, uint_binding),
		ascui_input_w_desc(true, TILES, 15, str("int [-69-69]: "), ALIGN_RIGHT, ALIGN_MIDDLE, TILES, 3, s_1, S16_INT, -69, 69, int_binding),
		ascui_slider(true, TILES, 3, s_slider, S16_INT, -69, 69, int_binding),
		ascui_input_w_desc(true, TILES, 15, str("str [10]: "), ALIGN_RIGHT, ALIGN_MIDDLE, TILES, 3, s_1, STRING, 0, 10, str_binding),
		ascui_input_w_desc(true, TILES, 15, str("float [0-1]: "), ALIGN_RIGHT, ALIGN_MIDDLE, TILES, 3, s_1, FLOAT, 0, 1, float_binding),
		ascui_slider(true, TILES, 3, s_slider, FLOAT, 0, 1, float_binding),
		ascui_display(true, HOVERABLE, TILES, 1, FLOAT, str("%f"), float_binding, ALIGN_MIDDLE, ALIGN_MIDDLE, s_text),
		ascui_divider(s_desc)
	);
}

int main(){
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


   	int16_t screensize_x = 1500;
	int16_t screensize_y = 1000;
	uint16_t tile_width = 25;

    InitWindow(screensize_x, screensize_y, "c_codebase Test Suite");
    Font unscii = LoadFontEx("Resources/Fonts/unscii-8-alt.ttf", 32, 0, 256);
    Font unscii_fantasy = LoadFontEx("Resources/Fonts/unscii-8-fantasy.ttf", 32, 0, 256);

	Font fonts[2] = {unscii, unscii_fantasy};
    
    SetTargetFPS(60);
    HideCursor();
    SetWindowMinSize(200, 200);

	// Main Grid
	grid_t *main_grid = tl_init_grid(0, 0, screensize_x, screensize_y, tile_width, 1.0f, fonts, 200);
	// grid_t *subgrid = tl_init_grid(0, 0, 100, 100, tile_width, 1.0f, fonts, 200);
	// UI
	main_ui();
	text_example_ui();
	input_example_ui();
	// ascui_get_subgrid_data(subgrid_container)->s    %ubgrid = subgrid;
	cursor_t cursor; 
	ascui_print_ui(main_ctx->top_container);
	ascui_print_ui(text_ctx->top_container);
	ascui_print_ui(input_ctx->top_container);
	// pos16_t mouse_subgrid_pos;

	c_ctx = main_ctx;
	
    while (!WindowShouldClose())
    {
		set_bound_var(text_ptr_binding, example_strings[text_index_binding->var]);
    	
    	tick++;
		frame_time = -GetTime();

        BeginDrawing();
        ClearBackground(BLACK);
        
		ascui_run_ui(main_grid, c_ctx, &ascui_drawing_time, NULL, NULL, 45, 47, &cursor);

		// printf("\nhov=[%p] sel[%p]", cursor.hovered_container, cursor.selected_container);

		// pos16_t subgrid_size = tl_grid_get_dimensions(subgrid);
		// tl_draw_rect_smbl_w_bg(subgrid, 0, 0, subgrid_size.x - 1, subgrid_size.y - 1, '.', col8bt(5, 1, 2), col8bt(3, 0, 1), 1);

		// if(cursor.hovered_container == subgrid_container)
		// {
			// mouse_subgrid_pos = tl_screen_to_grid_coords(subgrid, pos16(GetMouseX(), GetMouseY()));
			// tl_plot_smbl_w_bg(subgrid, mouse_subgrid_pos.x, mouse_subgrid_pos.y, 'A', WHITE8B, BLACK8B, 1);
		// }
		// else
		tl_plot_smbl_w_bg(main_grid, cursor.x, cursor.y, 'A', WHITE8B, BLACK8B, 0);

		tl_rendering_time = -GetTime();
		pos16_t main_grid_dcalls = tl_render_grid(main_grid);
		// pos16_t sub_grid_dcalls = tl_render_grid(subgrid);
		tl_rendering_time += GetTime();

		frame_time += GetTime();

		n_bg_draw_calls = main_grid_dcalls.x;
		n_smbl_draw_calls = main_grid_dcalls.y;

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
			// sprintf(buf, "(%u, %u)", mouse_subgrid_pos.x, mouse_subgrid_pos.y);
			// DrawText(buf, 0, 48, 24, c(200, 0, 200));
	        DrawFPS(0,64);
		}

        EndDrawing();
    }
    free_example_strings();
    UnloadFont(unscii);
    UnloadFont(unscii_fantasy);
    CloseWindow();
    ascui_destroy(main_ctx->top_container);
	tl_deinit_grid(main_grid);
    return 0;
}
