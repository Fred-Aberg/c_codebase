#pragma once
#include "common.h"
#include "raytiles.h"
#include "raylib.h"
#include <stdbool.h>
#include <stdarg.h>

#define style(font, bg_col, border_col, char_col, border_h_symbol, border_v_symbol, corner_symbol) (container_style_t){font, bg_col, border_col, char_col, border_h_symbol, border_v_symbol, corner_symbol}


typedef struct 
{
	uint8_t font;
    color8b_t bg_col;
    color8b_t border_col;
    color8b_t char_col;
    uint8_t border_h_symbol;
    uint8_t border_v_symbol;
    uint8_t corner_symbol;
}container_style_t;

typedef enum
{
	TILES,			// The container will receive exactly [TILES] tiles
	PERCENTAGE,		// The container will receive a % of the top container
}size_type_e;

typedef enum
{
	CONTAINER,
	BOX,
	TEXT,
	SUBGRID,
	BUTTON,
	DROPDOWN,
	INPUT,
	TOGGLE,
	DISPLAY,
	DIVIDER
}container_type_e;

typedef enum
{
	HORIZONTAL,
	VERTICAL,
}container_orientation_e;


#define STATIC 0		// !STATIC => HOVERABLE
#define HOVERABLE 1
#define SELECTABLE 2	// SELECTABLE => HOVERABLE
typedef struct
{
	bool open;
	uint8_t selectability;
	container_type_e container_type;
	uint16_t scroll_offset;
	uint8_t size;
	size_type_e size_type;
	void *container_type_data;
}container_t;

typedef struct
{
	container_orientation_e orientation;
	uint16_t n_subcontainers;
	container_t **subcontainers;
}container_data_t;

typedef struct
{
	container_orientation_e orientation;
	uint16_t n_subcontainers;
	container_t **subcontainers;
	container_style_t style;
}box_data_t;

#define ALIGN_TOP		0
#define ALIGN_MIDDLE	1
#define ALIGN_BOTTOM	2
#define ALIGN_LEFT		3
#define ALIGN_RIGHT		4

typedef struct
{
	container_style_t style;
	uint16_t text_len;
	char *text;
	uint8_t h_alignment;		// TOP, MIDDLE or BOTTOM
	uint8_t v_alignment;		// LEFT, RIGHT or MIDDLE
	uint8_t baked_available_width;
	ui8_list_t baked_line_widths;
}text_data_t;

typedef struct
{
	grid_t *subgrid;
}subgrid_data_t;

typedef enum
{
	U32_INT,	// 0
	U16_INT,	// 1
	U8_INT,		// 2
	S32_INT,	// 3
	S16_INT,	// 4
	S8_INT,		// 5
	STRING		// 6
}input_field_type_e;

#define INPUT_BUF_MAX_LEN 64
typedef struct
{
	input_field_type_e type;
	container_style_t style;
	int32_t min;				// min length of string or minimum value of digit
	int32_t max;
	uint8_t buf_i;
	char buf[INPUT_BUF_MAX_LEN];
	void *var;					// variable to be written to
}input_data_t;

typedef struct
{
    bool right_button_pressed; 
    bool left_button_pressed; 
    bool middle_button_pressed; 
    uint8_t x;
    uint8_t y;
    container_t *selected_container;
    container_t *hovered_container;
    float scroll;
} cursor_t;

typedef void (*UI_side_effect_func)(void *domain, void *function_data, cursor_t *cursor);
void ascui_dropdown_button_func(void *dropdown_cntr, void *button_text, cursor_t *cursor);

// Used for parameter substitution in buttons
#define SUBST_NEXT_CNTR (void *)1
#define SUBST_OWN_TEXT (void *)2
typedef struct
{
	bool param_substitution;
	uint16_t text_len;
	char *text;
	uint8_t baked_available_width;
	uint8_t h_alignment;		// TOP, MIDDLE or BOTTOM
	uint8_t v_alignment;		// LEFT, RIGHT or MIDDLE
	UI_side_effect_func side_effect_func;
	void *domain;
	void *function_data;
	container_style_t style;
	ui8_list_t baked_line_widths;
}button_data_t;

typedef struct
{
	container_style_t style_on;
	container_style_t style_off;
	bool *var;
}toggle_data_t;


typedef struct
{
	container_style_t style;
	uint16_t text_len;
	char *last_text;			// used to check whether text has changed
	char **text;				// NOT owned by display
	uint8_t h_alignment;		// TOP, MIDDLE or BOTTOM
	uint8_t v_alignment;		// LEFT, RIGHT or MIDDLE
	uint8_t baked_available_width;
	ui8_list_t baked_line_widths;
}display_data_t;

typedef struct
{
	container_style_t style;
}divider_data_t;

///// UI CONSTRUCTION

container_t *ascui_container(bool open, size_type_e s_type, uint8_t size, container_orientation_e orientation, uint16_t n_subcontainers, ...);

container_t *ascui_box(bool open, uint8_t selectability, size_type_e s_type, uint8_t size, container_orientation_e orientation, 
						container_style_t style, uint16_t n_subcontainers, ...);

container_t *ascui_text(bool open, uint8_t selectability, size_type_e s_type, uint8_t size, char *text, uint8_t h_align, uint8_t v_align, container_style_t style);

container_t *ascui_subgrid(bool open, size_type_e s_type, uint8_t size, grid_t *subgrid);

container_t *ascui_button(bool open, uint8_t selectability, size_type_e s_type, uint8_t size, char *text, uint8_t h_align, uint8_t v_align, container_style_t style, 
						  UI_side_effect_func side_effect_func, void *domain, void *function_data);

container_t *ascui_button_subst(bool open, uint8_t selectability, size_type_e s_type, uint8_t size, char *text, uint8_t h_align, uint8_t v_align, container_style_t style, 
						  UI_side_effect_func side_effect_func, void *domain, void *function_data);

container_t *ascui_input(bool open, size_type_e s_type, uint8_t size, container_style_t style, 
						  input_field_type_e input_type, int32_t min, int32_t max, void *var);
						  
container_t *ascui_toggle(bool *var, container_style_t style_on, container_style_t style_off);

container_t *ascui_display(bool open, uint8_t selectability, size_type_e s_type, uint8_t size, char **text, uint8_t h_align, uint8_t v_align, container_style_t style);

container_t *ascui_divider(container_style_t style);

#define ascui_dropdown_button(s_type, size, menu_text, h_align, v_align, style) \
	ascui_button_subst(true, HOVERABLE, s_type, size, menu_text, h_align, v_align, style, ascui_dropdown_button_func, SUBST_NEXT_CNTR, SUBST_OWN_TEXT)


// Composites
container_t *ascui_input_w_desc(bool open, size_type_e txt_s_type, uint8_t txt_size, char *text, uint8_t h_align, uint8_t v_align,
								size_type_e input_s_type, uint8_t input_size, container_style_t style, 
						  		input_field_type_e input_type, int32_t min, int32_t max, void *var);

///// TOP LEVEL FUNCTIONS

// Inits all fields of the cursor (except for hovered and selected containers)
void ascui_update_cursor(grid_t *grid, cursor_t *cursor);

// Draws given UI, sets hovered and selected containers
void ascui_draw_ui(grid_t *grid, container_t *top_container, cursor_t *cursor);

// Draws given UI, sets hovered and selected containers and calls appropriate button functions etc.
void ascui_navigate_ui(grid_t *grid, container_t *top_container, cursor_t *cursor, double *ascui_drawing_time, Sound *click_sound, Sound *scroll_sound);

// Adapts main grid to screen
void ascui_adapt_grid_to_screen(grid_t *grid, int screensize_x, int zoom_out_key);

// Composite of all top lvl functions in order
void ascui_run_ui(grid_t *grid, container_t *top_container, double *ascui_drawing_time, Sound *click_sound, Sound *scroll_sound,
						int zoom_in_key, int zoom_out_key, cursor_t *cursor);

///// CONTAINER DATA FETCHING

container_data_t *ascui_get_container_data(container_t *container);

box_data_t *ascui_get_box_data(container_t *container);

text_data_t *ascui_get_text_data(container_t *container);

subgrid_data_t *ascui_get_subgrid_data(container_t *container);

button_data_t *ascui_get_button_data(container_t *container);

toggle_data_t *ascui_get_toggle_data(container_t *container);

display_data_t *ascui_get_display_data(container_t *container);

divider_data_t *ascui_get_divider_data(container_t *container);

void ascui_set_nth_subcontainer(container_t *container, uint16_t n, container_t *subcontainer);

container_t *ascui_get_nth_subcontainer(container_t *container, uint16_t n);

///// MISC.

void ascui_print_ui(container_t *container);

void ascui_destroy(container_t *container);
