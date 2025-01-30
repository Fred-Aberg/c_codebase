#pragma once
#include "common.h"
#include "raytiles.h"
#include "raylib.h"
#include <stdbool.h>

#define c(r, g, b) (Color){r, g, b, 255}
#define NO_COLOR (Color){0, 0, 0, 0}

typedef struct 
{
    Color bg_col;
    Color border_col;
    Color char_col;
    char border_h_symbol;
    char border_v_symbol;
    char corner_symbol;
}Container_Style_t;

typedef enum
{
	PIXEL,			// The container will receive exactly [PIXEL] pixels
	PERCENTAGE,		// The container will receive a % of the top container
	GROW			// The container will receive as much of the top container as it needs (max 100%)
}Size_Type_e;

typedef enum
{
	CONTAINER,
	BOX,
	TEXT,
	SUBGRID,
	BUTTON
}Container_Type_e;

typedef enum
{
	HORIZONTAL,
	VERTICAL,
}Container_orientation_e;

typedef struct
{
	bool open;
	Size_Type_e size_type;
	uint size;
	bool scrollable;
	Container_Type_e container_type;
	void *container_type_data;
}Container_t;

typedef struct
{
	Container_orientation_e orientation;
	uint n_subcontainers;
	Container_t *subcontainers;
}Container_data_t;

typedef struct
{
	Container_orientation_e orientation;
	uint n_subcontainers;
	Container_t *subcontainers;
	Container_Style_t style;
}Box_data_t;

typedef struct
{
	uint text_len;
	char *text;
}Text_data_t;

// container_type_data = Grid_t *subgrid;
// typedef struct
// {
	// Grid_t *subgrid;
// }Subgrid_data_t;

typedef int (*UI_side_effect_func)(void *domain, void *type_data, KeyboardKey input);

typedef struct
{
	uint text_len;
	char *text;
	UI_side_effect_func *side_effect_func;
	Container_Style_t style;
}Button_data_t;

typedef struct
{
    Pos_t pos;
} Cursor_t;

void ascui_draw_ui(Grid_t *grid, Container_t *top_container);

Container_t *ascui_create_container(bool open, Size_Type_e s_type, uint size, bool scrollable,
									Container_orientation_e orientation, uint n_subcontainers, Container_t *subcontainers);
									
Container_data_t *ascui_get_container_data(Container_t *container);

Container_t *ascui_create_box(bool open, Size_Type_e s_type, uint size, bool scrollable, Container_orientation_e orientation, 
								uint n_subcontainers, Container_t *subcontainers, Container_Style_t style);

Box_data_t *ascui_get_box_data(Container_t *container);

Container_t *ascui_create_text(bool open, Size_Type_e s_type, uint size, bool scrollable, uint text_len, char *text);

Text_data_t *ascui_get_text_data(Container_t *container);

Container_t *ascui_create_subgrid(bool open, Size_Type_e s_type, uint size, Grid_t *subgrid);

Grid_t *ascui_get_subgrid(Container_t *container);

