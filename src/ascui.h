#pragma once
#include "common.h"
#include "raytiles.h"
#include "raylib.h"
#include <stdbool.h>

#define NO_COLOR (Color){0, 0, 0, 0}
#define style(font, bg_col, border_col, char_col, border_h_symbol, border_v_symbol, corner_symbol) (container_style_t){font, bg_col, border_col, char_col, border_h_symbol, border_v_symbol, corner_symbol}


typedef struct 
{
	char font;
    color8b_t bg_col;
    color8b_t border_col;
    color8b_t char_col;
    uchar_t border_h_symbol;
    uchar_t border_v_symbol;
    uchar_t corner_symbol;
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
	INPUT_FIELD
}container_type_e;

typedef enum
{
	HORIZONTAL,
	VERTICAL,
}container_orientation_e;

typedef struct
{
	bool open;
	container_type_e container_type;
	uint_t scroll_offset;
	uchar_t size;
	size_type_e size_type;
	void *container_type_data;
}container_t;

typedef struct
{
	container_orientation_e orientation;
	uint_t n_subcontainers;
	container_t *subcontainers;
}container_data_t;

typedef struct
{
	container_orientation_e orientation;
	uint_t n_subcontainers;
	container_t *subcontainers;
	container_style_t style;
}box_data_t;

typedef struct
{
	container_style_t style;
	uint_t text_len;
	char *text;
}text_data_t;

typedef struct
{
	grid_t *subgrid;
}subgrid_data_t;

typedef enum
{
	STR,
	SIGNED_INT,
	UNSIGNED_INT,
	SIGNED_SHORT,
	UNSIGNED_SHORT,
	SIGNED_CHAR,
	UNSIGNED_CHAR,
}input_field_type_e;

typedef struct
{
	input_field_type_e type;
	char buf[64];
	void *var;
}input_field_data_t;

typedef struct
{
    bool right_button_pressed; 
    bool left_button_pressed; 
    bool middle_button_pressed; 
    uchar_t x;
    uchar_t y;
    container_t *selected_container;
    container_t *hovered_container;
    float scroll;
} cursor_t;

typedef void (*UI_side_effect_func)(void *domain, void *function_data, cursor_t *cursor);

typedef struct
{
	uint_t text_len;
	char *text;
	UI_side_effect_func side_effect_func;
	void *domain;
	void *function_data;
	container_style_t style;
}button_data_t;

#define TAG_MAX_LEN 64
typedef struct
{
	char tag[TAG_MAX_LEN];
	container_t *container;
}container_tag_t;

# define REALLOC_PERCENTAGE_INCREASE 1.5f
typedef struct
{
	container_tag_t *tags;
	uint_t capacity;
	uint_t count;
}container_tag_list_t;



void ascui_draw_ui(grid_t *grid, container_t *top_container, cursor_t *cursor);

container_t ascui_create_container(bool open, size_type_e s_type, uchar_t size,
									container_orientation_e orientation, uint_t n_subcontainers);
									
container_data_t *ascui_get_container_data(container_t container);

container_t ascui_create_box(bool open, size_type_e s_type, uchar_t size,
									container_orientation_e orientation, uint_t n_subcontainers, container_style_t style);

box_data_t *ascui_get_box_data(container_t container);

container_t ascui_create_text(bool open, size_type_e s_type, uchar_t size, uint_t text_len, char *text, container_style_t style);

text_data_t *ascui_get_text_data(container_t container);

container_t ascui_create_subgrid(bool open, size_type_e s_type, uchar_t size, grid_t *subgrid);

subgrid_data_t *ascui_get_subgrid_data(container_t container);

button_data_t *ascui_get_button_data(container_t container);

container_t ascui_create_button(bool open, size_type_e s_type, uchar_t size, 
								container_style_t style, UI_side_effect_func side_effect_func, uint_t text_len, char *text, void *domain, void *function_data);

void ascui_set_nth_subcontainer(container_t container, uint_t n, container_t subcontainer);

container_t *ascui_get_nth_subcontainer(container_t container, uint_t n);

void ascui_print_ui(container_t container);

void ascui_destroy(container_t container);

// Tag list //

container_tag_list_t ascui_tag_list_create(uint_t init_capacity);

void ascui_tag_list_add(container_tag_list_t *list, container_t *container, char *tag);

container_t *ascui_tag_list_get(container_tag_list_t list, char *tag);

void ascui_tag_list_destroy(container_tag_list_t list);

void ascui_tag_list_print(container_tag_list_t list);

// UI Constructor //

container_t *ascui_construct_ui_from_file(char *ui_file_path, container_tag_list_t *tag_list);
