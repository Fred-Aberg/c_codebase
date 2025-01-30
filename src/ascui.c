#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "ascui.h"
#include "raytiles.h"



static uint ascui_draw_container(Grid_t *grid, Container_t *container, uint x0, uint y0, uint x1, uint y1, Container_orientation_e parent_orientation)
{
	if (!container->open)
		return 0;

	if (x1 - x0 <= 0 || y1 - y0 <= 0)
		return 0;

	uint pixels_drawn = 0; 	// Number of pixels required for this container and all it's subcontainers
							// Cannot exceed x1-x0 if VERTICAL, y1-y0 if HORIZONTAL
		

	float percentage;
	uint sub_x1;
	uint sub_y1;
	uint sub_x0;
	uint sub_y0;
	uint n_subcontainers;
	Container_t *c_subcontainer;
	Container_Type_e c_type = container->container_type;
	switch (c_type)
	{
		case CONTAINER:
			Container_data_t c_data = *ascui_get_container_data(container);
			n_subcontainers = c_data.n_subcontainers;

			
			for (uint i = 0; i < n_subcontainers; i++)
			{
				c_subcontainer = &c_data.subcontainers[i];
				
				if (c_subcontainer->size_type == PIXEL)
				{
					if (c_data.orientation == HORIZONTAL)
					{
						sub_x1 = x1;
						sub_y1 = y0 + pixels_drawn + c_subcontainer->size;
						if (sub_y1 > y1)
							break;
						pixels_drawn += ascui_draw_container(grid, c_subcontainer, x0, y0 + pixels_drawn, sub_x1, sub_y1, c_data.orientation);
					}
					else if (c_data.orientation == VERTICAL)
					{
						sub_x1 = x0 + pixels_drawn + c_subcontainer->size;
						sub_y1 = y1;
						if (sub_x1 > x1)
							break;
						pixels_drawn += ascui_draw_container(grid, c_subcontainer, x0 + pixels_drawn, y0, sub_x1, sub_y1, c_data.orientation);
					} 					
				}
				else if (c_subcontainer->size_type == PERCENTAGE)
				{
					percentage = (float)c_subcontainer->size / 100;
					if (c_data.orientation == HORIZONTAL)
					{
						sub_x1 = x1;
						sub_y1 = y0 + pixels_drawn + (y1-y0) * percentage;
						if (sub_y1 > y1)
							break;
						pixels_drawn += ascui_draw_container(grid, c_subcontainer, x0, y0 + pixels_drawn, sub_x1, sub_y1, c_data.orientation);
					}
					else if (c_data.orientation == VERTICAL)
					{
						sub_x1 = x0 + pixels_drawn + (x1-x0) * percentage;
						sub_y1 = y1;
						if (sub_x1 > x1)
							break;
						pixels_drawn += ascui_draw_container(grid, c_subcontainer, x0 + pixels_drawn, y0, sub_x1, sub_y1, c_data.orientation);
					} 					
				}
				else if (c_subcontainer->size_type == GROW)
				{
					if (c_data.orientation == HORIZONTAL)
					{
						sub_x1 = x1;
						sub_y1 = y1;
						pixels_drawn += ascui_draw_container(grid, c_subcontainer, x0, y0 + pixels_drawn, sub_x1, sub_y1, c_data.orientation);
					}
					else if (c_data.orientation == VERTICAL)
					{
						sub_x1 = x1;
						sub_y1 = y1;
						pixels_drawn += ascui_draw_container(grid, c_subcontainer, x0 + pixels_drawn, y0, sub_x1, sub_y1, c_data.orientation);
					} 					
				}
			}
			return pixels_drawn;
		case BOX:
			Box_data_t b_data = *ascui_get_box_data(container);
			n_subcontainers = b_data.n_subcontainers;
			sub_x0 = x0 + 1;
			sub_y0 = y0 + 1;
			

			for (uint i = 0; i < n_subcontainers; i++)
			{
				c_subcontainer = &c_data.subcontainers[i];
				
				if (c_subcontainer->size_type == PIXEL)
				{
					if (c_data.orientation == HORIZONTAL)
					{
						sub_x1 = x1 - 1;
						sub_y1 = sub_y0 + pixels_drawn + c_subcontainer->size;
						if (sub_y1 > y1 - 1)
							break;
						pixels_drawn += ascui_draw_container(grid, c_subcontainer, sub_x0, sub_y0 + pixels_drawn, sub_x1, sub_y1, c_data.orientation);
					}
					else if (c_data.orientation == VERTICAL)
					{
						sub_x1 = sub_x0 + pixels_drawn + c_subcontainer->size;
						sub_y1 = y1 - 1;
						if (sub_x1 > x1 - 1)
							break;
						pixels_drawn += ascui_draw_container(grid, c_subcontainer, sub_x0 + pixels_drawn, sub_y0, sub_x1, sub_y1, c_data.orientation);
					} 					
				}
				else if (c_subcontainer->size_type == PERCENTAGE)
				{
					float percentage = (float)c_subcontainer->size / 100;
					if (c_data.orientation == HORIZONTAL)
					{
						sub_x1 = x1 - 1;
						sub_y1 = sub_y0 + pixels_drawn + (y1-y0-2) * percentage;
						if (sub_y1 > y1 - 1)
							break;
						pixels_drawn += ascui_draw_container(grid, c_subcontainer, sub_x0, sub_y0 + pixels_drawn, sub_x1, sub_y1, c_data.orientation);
					}
					else if (c_data.orientation == VERTICAL)
					{
						sub_x1 = sub_x0 + pixels_drawn + (x1-x0-2) * percentage;
						sub_y1 = y1 - 1;
						if (sub_x1 > x1 - 1)
							break;
						pixels_drawn += ascui_draw_container(grid, c_subcontainer, sub_x0 + pixels_drawn, sub_y0, sub_x1, sub_y1, c_data.orientation);
					} 					
				}
				else if (c_subcontainer->size_type == GROW)
				{
					if (c_data.orientation == HORIZONTAL)
					{
						sub_x1 = x1 - 1;
						sub_y1 = y1 - 1;
						pixels_drawn += ascui_draw_container(grid, c_subcontainer, sub_x0, sub_y0 + pixels_drawn, sub_x1, sub_y1, c_data.orientation);
					}
					else if (c_data.orientation == VERTICAL)
					{
						sub_x1 = x1 - 1;
						sub_y1 = y1 - 1;
						pixels_drawn += ascui_draw_container(grid, c_subcontainer, sub_x0 + pixels_drawn, sub_y0, sub_x1, sub_y1, c_data.orientation);
					} 					
				}
			}

			if (parent_orientation == HORIZONTAL)
				x1 = x0 + pixels_drawn + 1; 
			else if (parent_orientation == VERTICAL)
				y1 = y0 + pixels_drawn + 1; 

			tl_draw_line(grid, x0 + 1, y0, x1 - 1, y0, b_data.style.border_h_symbol, b_data.style.char_col, b_data.style.border_col, NULL); // Top
			tl_draw_line(grid, x0 + 1, y1, x1 - 1, y1, b_data.style.border_h_symbol, b_data.style.char_col, b_data.style.border_col, NULL); // Bottom
			tl_draw_line(grid, x0, y0 + 1, x0, y1 - 1, b_data.style.border_v_symbol, b_data.style.char_col, b_data.style.border_col, NULL); // Left
			tl_draw_line(grid, x1, y0 + 1, x1, y1 - 1, b_data.style.border_v_symbol, b_data.style.char_col, b_data.style.border_col, NULL); // Right

			tl_draw_tile(grid, x0, y0, b_data.style.corner_symbol, b_data.style.char_col, b_data.style.border_col, NULL);
			tl_draw_tile(grid, x0, y1, b_data.style.corner_symbol, b_data.style.char_col, b_data.style.border_col, NULL);
			tl_draw_tile(grid, x1, y0, b_data.style.corner_symbol, b_data.style.char_col, b_data.style.border_col, NULL);
			tl_draw_tile(grid, x1, y1, b_data.style.corner_symbol, b_data.style.char_col, b_data.style.border_col, NULL);
			
			return pixels_drawn;
		case TEXT:
		case SUBGRID:
		default:
			return 0;
	}
}

void ascui_draw_ui(Grid_t *grid, Container_t *top_container)
{
	Pos_t grid_size = tl_grid_get_size(grid);
	Container_data_t c_data = *ascui_get_container_data(top_container);
	ascui_draw_container(grid, top_container, 0, 0, grid_size.x, grid_size.y, c_data.orientation);
}

static Container_t *create_container_stub(bool open, Size_Type_e s_type, uint size, bool scrollable, Container_Type_e c_type)
{
	Container_t	*container = calloc(1, sizeof(Container_t));
	container->open = open;
	container->size_type = s_type;
	container->size = size;
	container->scrollable = scrollable;
	container->container_type = c_type;

	return container;
}

Container_data_t *ascui_get_container_data(Container_t *container)
{
	return (Container_data_t *)container->container_type_data;
}

Container_t *ascui_create_container(bool open, Size_Type_e s_type, uint size, bool scrollable,
									Container_orientation_e orientation, uint n_subcontainers, Container_t *subcontainers)
{
	Container_t *container = create_container_stub(open, s_type, size, scrollable, CONTAINER);
	container->container_type_data = calloc(1, sizeof(Container_data_t));
	
	Container_data_t *c_data = ascui_get_container_data(container);

	c_data->orientation = orientation;
	c_data->n_subcontainers = n_subcontainers;
	c_data->subcontainers = subcontainers; // container owns subcontainers

	return container;
}

Box_data_t *ascui_get_box_data(Container_t *container)
{
	return (Box_data_t *)container->container_type_data;
}

Container_t *ascui_create_box(bool open, Size_Type_e s_type, uint size, bool scrollable, Container_orientation_e orientation, 
								uint n_subcontainers, Container_t *subcontainers, Container_Style_t style)
{
	Container_t *box = create_container_stub(open, s_type, size, scrollable, BOX);
	box->container_type_data = calloc(1, sizeof(Box_data_t));
	
	Box_data_t *b_data = ascui_get_box_data(box);

	b_data->style = style;
	b_data->orientation = orientation;
	b_data->n_subcontainers = n_subcontainers;
	b_data->subcontainers = subcontainers; // container owns subcontainers

	return box;
}

Text_data_t *ascui_get_text_data(Container_t *container)
{
	return (Text_data_t *)container->container_type_data;
}

Container_t *ascui_create_text(bool open, Size_Type_e s_type, uint size, bool scrollable, uint text_len, char *text)
{
	Container_t *text_container = create_container_stub(open, s_type, size, scrollable, TEXT);
	text_container->container_type_data = calloc(1, sizeof(Text_data_t));
	
	Text_data_t *t_data = ascui_get_text_data(text_container);

	t_data->text = text; // container owns text
	t_data->text_len = text_len;
	return text_container;
}

Grid_t *ascui_get_subgrid(Container_t *container)
{
	return (Grid_t *)container->container_type_data;
}

Container_t *ascui_create_subgrid(bool open, Size_Type_e s_type, uint size, Grid_t *subgrid)
{
	Container_t *subgrid_container = create_container_stub(open, s_type, size, false, SUBGRID);
	subgrid_container->container_type_data = subgrid;

	return subgrid_container;
}

