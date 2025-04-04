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
	DISPLAY,
	INPUT
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

typedef struct
{
	container_style_t style;
	uint16_t text_len;
	char *text;
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

typedef struct
{
	uint16_t text_len;
	char *text;
	UI_side_effect_func side_effect_func;
	void *domain;
	void *function_data;
	container_style_t style;
}button_data_t;

///// UI CONSTRUCTION

container_t *ascui_container(bool open, size_type_e s_type, uint8_t size, container_orientation_e orientation, uint16_t n_subcontainers, ...);

container_t *ascui_box(bool open, uint8_t selectability, size_type_e s_type, uint8_t size, container_orientation_e orientation, container_style_t style, uint16_t n_subcontainers, ...);

container_t *ascui_text(bool open, uint8_t selectability, size_type_e s_type, uint8_t size, uint16_t text_len, char *text, container_style_t style);

container_t *ascui_subgrid(bool open, size_type_e s_type, uint8_t size, grid_t *subgrid);

container_t *ascui_button(bool open, uint8_t selectability, size_type_e s_type, uint8_t size, uint16_t text_len, char *text, container_style_t style, 
						  UI_side_effect_func side_effect_func, void *domain, void *function_data);
						  
container_t *ascui_input(bool open, size_type_e s_type, uint8_t size, container_style_t style, 
						  input_field_type_e input_type, int32_t min, int32_t max, void *var);
						  
container_t *ascui_input_w_desc(bool open, size_type_e s_type, uint8_t size, uint16_t text_len, char *text, container_style_t style, 
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

void ascui_set_nth_subcontainer(container_t *container, uint16_t n, container_t *subcontainer);

container_t *ascui_get_nth_subcontainer(container_t *container, uint16_t n);

///// MISC.

void ascui_print_ui(container_t *container);

void ascui_destroy(container_t *container);
