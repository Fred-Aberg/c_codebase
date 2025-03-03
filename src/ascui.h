#pragma once
#include "common.h"
#include "raytiles.h"
#include "raylib.h"
#include <stdbool.h>

#define NO_COLOR (Color){0, 0, 0, 0}
#define style(bg_col, border_col, char_col, border_h_symbol, border_v_symbol, corner_symbol) (Container_style_t){bg_col, border_col, char_col, border_h_symbol, border_v_symbol, corner_symbol}


typedef struct 
{
    Color bg_col;
    Color border_col;
    Color char_col;
    char border_h_symbol;
    char border_v_symbol;
    char corner_symbol;
}Container_style_t;

typedef enum
{
	TILES,			// The container will receive exactly [TILES] tiles
	PERCENTAGE,		// The container will receive a % of the top container
	GROW			// The container will receive as much of the top container as it needs (max 100%)
}Size_Type_e;

typedef enum
{
	CONTAINER,
	BOX,
	TEXT,
	SUBGRID,
	BUTTON,
	DISPLAY,
	INPUT_FIELD
}Container_Type_e;

typedef enum
{
	HORIZONTAL,
	VERTICAL,
}Container_orientation_e;

typedef struct
{
	bool open;
	Container_Type_e container_type;
	uint_t scroll_offset;
	uint_t size;
	Size_Type_e size_type;
	void *container_type_data;
}Container_t;

typedef struct
{
	Container_orientation_e orientation;
	uint_t n_subcontainers;
	Container_t *subcontainers;
}Container_data_t;

typedef struct
{
	Container_orientation_e orientation;
	uint_t n_subcontainers;
	Container_t *subcontainers;
	Container_style_t style;
}Box_data_t;

typedef struct
{
	Color bg_col;
	Color text_col;
	uint_t text_len;
	char *text;
}Text_data_t;

typedef struct
{
	Grid_t *subgrid;
}Subgrid_data_t;

typedef enum
{
	STR,
	SIGNED_INT,
	UNSIGNED_INT,
	SIGNED_SHORT,
	UNSIGNED_SHORT,
	SIGNED_CHAR,
	UNSIGNED_CHAR,
}Input_field_type_e;

typedef struct
{
	Input_field_type_e type;
	char buf[64];
	void *var;
}Input_field_data_t;

typedef struct
{
    bool right_button_pressed; 
    bool left_button_pressed; 
    bool middle_button_pressed; 
    uint_t x;
    uint_t y;
    Container_t *selected_container;
    Container_t *hovered_container;
    float scroll;
} Cursor_t;

typedef void (*UI_side_effect_func)(void *domain, void *function_data, Cursor_t *cursor);

typedef struct
{
	uint_t text_len;
	char *text;
	UI_side_effect_func side_effect_func;
	void *domain;
	void *function_data;
	Container_style_t style;
}Button_data_t;

#define TAG_MAX_LEN 64
typedef struct
{
	char tag[TAG_MAX_LEN];
	Container_t *container;
}Container_tag_t;

# define REALLOC_PERCENTAGE_INCREASE 1.5f
typedef struct
{
	Container_tag_t *tags;
	uint_t capacity;
	uint_t count;
}Container_tag_list_t;



void ascui_draw_ui(Grid_t *grid, Container_t *top_container, Cursor_t *cursor);

Container_t ascui_create_container(bool open, Size_Type_e s_type, uint_t size,
									Container_orientation_e orientation, uint_t n_subcontainers);
									
Container_data_t *ascui_get_container_data(Container_t container);

Container_t ascui_create_box(bool open, Size_Type_e s_type, uint_t size,
									Container_orientation_e orientation, uint_t n_subcontainers, Container_style_t style);

Box_data_t *ascui_get_box_data(Container_t container);

Container_t ascui_create_text(bool open, Size_Type_e s_type, uint_t size, uint_t text_len, char *text, Color text_col, Color bg_color);

Text_data_t *ascui_get_text_data(Container_t container);

Container_t ascui_create_subgrid(bool open, Size_Type_e s_type, uint_t size, Grid_t *subgrid, Color default_color, Font *default_font);

Subgrid_data_t *ascui_get_subgrid_data(Container_t container);

Button_data_t *ascui_get_button_data(Container_t container);

Container_t ascui_create_button(bool open, Size_Type_e s_type, uint_t size, 
								Container_style_t style, UI_side_effect_func side_effect_func, uint_t text_len, char *text, void *domain, void *function_data);

void ascui_set_nth_subcontainer(Container_t container, uint_t n, Container_t subcontainer);

Container_t *ascui_get_nth_subcontainer(Container_t container, uint_t n);

void ascui_print_ui(Container_t container);

void ascui_destroy(Container_t container);

// Tag list //

Container_tag_list_t ascui_tag_list_create(uint_t init_capacity);

void ascui_tag_list_add(Container_tag_list_t *list, Container_t *container, char *tag);

Container_t *ascui_tag_list_get(Container_tag_list_t list, char *tag);

void ascui_tag_list_destroy(Container_tag_list_t list);

void ascui_tag_list_print(Container_tag_list_t list);

// UI Constructor //

Container_t *ascui_construct_ui_from_file(char *ui_file_path, Container_tag_list_t *tag_list);
