#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "common.h"
#include "ascui.h"
#include "raytiles.h"
#include "texts.h"

#define get_orientation(container) *(container_orientation_e *)(container->container_type_data)


// static void draw_cursor_selection(grid_t *grid, cursor_t *cursor, container_t *container, uchar_t x0, uchar_t y0, uchar_t x1, uchar_t y1, uint_t max_scroll)
// {
	// if(cursor->hovered_container != container && cursor->selected_container != container) return;
	// if (container->container_type == BOX || container->container_type == CONTAINER)
	// {
		// for (uint_t x = x0; x <= x1; x++)
		// {
			// tl_tile_invert_color8b_ts(grid, x, y0);
			// tl_tile_invert_color8b_ts(grid, x, y1);
		// }
		// for (uint_t y = y0 + 1; y < y1; y++)
		// {
			// tl_tile_invert_color8b_ts(grid, x0, y);
			// tl_tile_invert_color8b_ts(grid, x1, y);
		// }
	// }
	// else
	// {
		// for (uint_t x = x0; x <= x1; x++)
		// {
			// for (uint_t y = y0; y <= y1; y++)
			// {
				// tl_tile_invert_color8b_ts(grid, x, y);
			// }
		// }
	// }
// 
	// if(container->container_type == BOX || container->container_type == CONTAINER)
	// {
		// if(get_orientation(container) == HORIZONTAL)
			// tl_set_tile_bg(grid, x0, y0 + (y1-y0) * ((float)container->scroll_offset / (float)max_scroll), BLACK);
		// else
			// tl_set_tile_bg(grid, x0 + (x1-x0) * ((float)container->scroll_offset / (float)max_scroll), y0, BLACK);
	// }
	// if(cursor->scroll != 0 && max_scroll != 0)
	// {
		// tl_set_tile_bg(grid, x0, y0 + (y1-y0) * ((float)container->scroll_offset / (float)max_scroll), BLACK);
	// }
// }

static bool check_cursor_hover(cursor_t *cursor, container_t *container, uchar_t x0, int y0, uchar_t x1, int y1)
{
	if(container->container_type == CONTAINER)
		return false;
	
	if(container->container_type == BOX)
	{
		bool x_match = (cursor->x == x0) || (cursor->x == x1);
		bool y_match = (cursor->y == y0) || (cursor->y == y1);
		if(x_match || y_match)
		{
			cursor->hovered_container = container;
			return true;
		}
		return false;
	}

	// All other types (childless)
	if(x0 <= cursor->x && x1 >= cursor->x && y0 <= (int)cursor->y && y1 >= (int)cursor->y)
	{
		cursor->hovered_container = container;
		return true;
	}
	return false;
}

static void draw_box(grid_t *grid, container_style_t style, uchar_t x0, uchar_t y0, uchar_t x1, uchar_t y1, bool invert_cols)
{
	if (invert_cols)
	{
		color8b_t tmp = style.char_col;
		style.char_col = style.border_col;
		style.border_col = tmp;
	}

	tl_draw_line_smbl_w_bg(grid, x0 + 1, y0, x1 - 1, y0, style.border_h_symbol, style.char_col, style.border_col, style.font); // Top
	tl_draw_line_smbl_w_bg(grid, x0 + 1, y1, x1 - 1, y1, style.border_h_symbol, style.char_col, style.border_col, style.font); // Bottom
	tl_draw_line_smbl_w_bg(grid, x0, y0 + 1, x0, y1 - 1, style.border_v_symbol, style.char_col, style.border_col, style.font); // Left
	tl_draw_line_smbl_w_bg(grid, x1, y0 + 1, x1, y1 - 1, style.border_v_symbol, style.char_col, style.border_col, style.font); // Right

	tl_plot_smbl_w_bg(grid, x0, y0, style.corner_symbol, style.char_col, style.border_col, style.font);
	tl_plot_smbl_w_bg(grid, x0, y1, style.corner_symbol, style.char_col, style.border_col, style.font);
	tl_plot_smbl_w_bg(grid, x1, y0, style.corner_symbol, style.char_col, style.border_col, style.font);
	tl_plot_smbl_w_bg(grid, x1, y1, style.corner_symbol, style.char_col, style.border_col, style.font);
}


static uint_t ascui_draw_container(grid_t *grid, container_t *container, uint_t x0, uint_t y0, uint_t x1, uint_t y1,
								 container_orientation_e parent_orientation, cursor_t *cursor);

/*
static uint_t ascui_draw_container_percentage_split(grid_t *grid, container_t *container, uint_t x0, uint_t y0, uint_t x1, uint_t y1, container_orientation_e parent_orientation, cursor_t *cursor)
{
	if (!container->open)
		return 0;

	if (x1 <= x0 || y1 <= y0)
		return 0;
		
	uint_t subcontainers_size = 0;
	float percentage;
	uint_t i = 0;
	uint_t tiles_drawn = 0;


	uint_t n_subcontainers;
	container_t *subcontainers;
	container_orientation_e orientation;
	container_style_t style;
	
	if (container->container_type == CONTAINER)
	{
		container_data_t c_data = *ascui_get_container_data(*container);
		n_subcontainers = c_data.n_subcontainers;
		subcontainers = c_data.subcontainers;
		orientation = c_data.orientation;
		
	}
	else if (container->container_type == BOX)
	{
		box_data_t b_data = *ascui_get_box_data(*container);
		n_subcontainers = b_data.n_subcontainers;
		subcontainers = b_data.subcontainers;
		orientation = b_data.orientation;
		style = b_data.style;
		x0++;
		y0++;
		x1--;
		y1--;
	}
	else
		assert(false);

	uint_t container_size = (orientation == VERTICAL)? x1-x0 + 1 : y1-y0 + 1;
	// printf("\n----\npre: %u", container_size);
		
	uint_t allotted_tiles[n_subcontainers];
	for (i = 0; i < n_subcontainers; i++)
	{
		percentage = (float)subcontainers[i].size / 100;
		allotted_tiles[i] = container_size * percentage;
		subcontainers_size += container_size * percentage;
	}
	
	container_size -= subcontainers_size;
	// printf("\npost: %u", container_size);
	i = 0;
	while (container_size > 0)
	{
		allotted_tiles[i]++;
		container_size--;
		i++;
		i = i % n_subcontainers;
	}
	// printf("\npost-allottment: %u", container_size);
	
	for (i = 0; i < n_subcontainers; i++)
	{
		// printf("\n[%u] -> %u", i, allotted_tiles[i]);
		if (orientation == HORIZONTAL)
			tiles_drawn += ascui_draw_container(grid, &subcontainers[i], x0, y0 + tiles_drawn, x1 , y0 + tiles_drawn + allotted_tiles[i] - 1, orientation, cursor);
		else	// VERTICAL
			tiles_drawn += ascui_draw_container(grid, &subcontainers[i], x0 + tiles_drawn, y0, x0 + tiles_drawn + allotted_tiles[i] - 1 , y1, orientation, cursor);		
	}

	if(container->container_type == BOX)
	{
		x0--;
		y0--;
		x1++;
		y1++;
		draw_box(grid, style, x0, y0, x1, y1);
	}
	draw_cursor_selection(grid, cursor, container, x0, y0, x1, y1, 0);
	return (parent_orientation == VERTICAL)? x1-x0 + 1 : y1-y0 + 1;
}*/

static uint_t ascui_draw_container(grid_t *grid, container_t *container, uint_t x0, uint_t y0, uint_t x1, uint_t y1,
								 container_orientation_e parent_orientation, cursor_t *cursor)
{
	if (!container->open)
		return 0;

	if (x1 <= x0 || y1 <= y0)
		return 0;

	check_cursor_hover(cursor, container, x0, y0 , x1, y1);

	bool hovered = false;
	bool selected = cursor->selected_container == container;
	float percentage;

	uchar_t sub_x1;
	uchar_t sub_y1;

	uint_t n_subcontainers;
	container_t *c_subcontainer;
	container_type_e c_type = container->container_type;
	// bool percentage_split = true;

	uint_t tiles_drawn = 0;
	uint_t max_scroll = 0;
	 	
	if (c_type == CONTAINER || c_type == BOX)
	{
		container_t *subcontainers;
		container_orientation_e orientation;
		container_style_t style;

		

		if (c_type == CONTAINER)
		{
			container_data_t c_data = *ascui_get_container_data(*container);
			n_subcontainers = c_data.n_subcontainers;
			subcontainers = c_data.subcontainers;
			orientation = c_data.orientation;
			
		}
		else if (c_type == BOX)
		{
			box_data_t b_data = *ascui_get_box_data(*container);
			n_subcontainers = b_data.n_subcontainers;
			subcontainers = b_data.subcontainers;
			orientation = b_data.orientation;
			style = b_data.style;
		}

		max_scroll = max((int)n_subcontainers - 1, 0);
		container->scroll_offset = umin(container->scroll_offset, max_scroll); // Clamp scroll

		/*
		for (uint_t i = 0; i < n_subcontainers; i++)
			if (subcontainers[i].size_type != PERCENTAGE)
				percentage_split = false;

		if (percentage_split && n_subcontainers != 0)
		{
			tiles_drawn = ascui_draw_container_percentage_split(grid, container, x0, y0, x1, y1, parent_orientation, cursor);
			draw_cursor_selection(grid, cursor, container, x0, y0, x1, y1, 0);
			return tiles_drawn;
		}
		*/

		// Account for box edges when delegating space for subcontainers
		if (c_type == BOX) {x0++; y0++; x1--; y1--;}

		for (uint_t i = container->scroll_offset; i < n_subcontainers; i++)
		{
			c_subcontainer = &subcontainers[i];
			// Edge case: Last subcontainer
			if (i == n_subcontainers - 1)
			{
				if (orientation == HORIZONTAL)
				{
					tiles_drawn += ascui_draw_container(grid, c_subcontainer, x0, y0 + tiles_drawn, x1, y1, orientation, cursor);

					// if (c_type == BOX)
					// {
						// x0--; y0--; x1++; y1++;
						// draw_box(grid, style, x0, y0, x1, y1);
					// }
					// draw_cursor_selection(grid, cursor, container, x0, y0, x1, y1, max_scroll);
					// 
					// return y1-y0 + 1;
				}
				else if (orientation == VERTICAL)
				{
					tiles_drawn += ascui_draw_container(grid, c_subcontainer, x0 + tiles_drawn, y0, x1, y1, orientation, cursor);

					// if (c_type == BOX)
					// {
						// x0--; y0--; x1++; y1++;
						// draw_box(grid, style, x0, y0, x1, y1);
					// }
					// draw_cursor_selection(grid, cursor, container, x0, y0, x1, y1, max_scroll);
					// 
					// return x1-x0 + 1;
				}

				continue;
			}

				
			if (c_subcontainer->size_type == TILES)
			{
				if (orientation == HORIZONTAL)
				{
					sub_x1 = x1;
					sub_y1 = y0 + tiles_drawn + c_subcontainer->size - 1;
					if (sub_y1 > y1)
						break;
					tiles_drawn += ascui_draw_container(grid, c_subcontainer, x0, y0 + tiles_drawn, sub_x1, sub_y1, orientation, cursor);
				}
				else if (orientation == VERTICAL)
				{
					sub_x1 = x0 + tiles_drawn + c_subcontainer->size - 1;
					sub_y1 = y1;
					if (sub_x1 > x1)
						break;
					tiles_drawn += ascui_draw_container(grid, c_subcontainer, x0 + tiles_drawn, y0, sub_x1, sub_y1, orientation, cursor);
				} 					
			}
			else if (c_subcontainer->size_type == PERCENTAGE)
			{
				percentage = (float)c_subcontainer->size / 100;
				if (orientation == HORIZONTAL)
				{
					sub_x1 = x1;
					sub_y1 = y0 + tiles_drawn + (y1-y0) * percentage - 1; // (y1-y0) * percentage is a length, -1 to make a coord
					if (sub_y1 > y1)
						break;
					tiles_drawn += ascui_draw_container(grid, c_subcontainer, x0, y0 + tiles_drawn, sub_x1, sub_y1, orientation, cursor);
				}
				else if (orientation == VERTICAL)
				{
					sub_x1 = x0 + tiles_drawn + (x1-x0) * percentage - 1;
					sub_y1 = y1;
					if (sub_x1 > x1)
						break;
					tiles_drawn += ascui_draw_container(grid, c_subcontainer, x0 + tiles_drawn, y0, sub_x1, sub_y1, orientation, cursor);
				} 					
			}
		}
		
		if(c_type == BOX)
		{
			x0--;
			y0--;
			x1++;
			y1++;
			
			draw_box(grid, style, x0, y0, x1, y1, check_cursor_hover(cursor, container, x0, y0, x1, y1));
		}
		

		if (parent_orientation == HORIZONTAL)
			tiles_drawn = y1-y0 + 1;
		else if (parent_orientation == VERTICAL)
			tiles_drawn = x1-x0 + 1;

		return tiles_drawn;
	}
	else if(c_type == TEXT)
	{
		hovered = check_cursor_hover(cursor, container, x0, y0, x1, y1);
		
		text_data_t t_data = *ascui_get_text_data(*container);

		color8b_t bg_col;
		color8b_t smbl_col;
		if(hovered || selected)
		{
			bg_col = t_data.style.char_col;
			smbl_col = t_data.style.bg_col;
		}
		else
		{
			bg_col = t_data.style.bg_col;
			smbl_col = t_data.style.char_col;
		}
		
		uint_t _x = 0;
		int _y = - container->scroll_offset; // = 0 at no scroll
		uint_t total_text_len = 0;

		tl_draw_rect_bg(grid, x0, y0, x1, y1, bg_col);

		for (uint_t i = 0; i < t_data.text_len; i++)
		{
			if (x0 + _x > x1)
				{total_text_len++;  _y++; _x = 0; }
			
			if (t_data.text[i] == '\n')
			{	
				_y++;
				total_text_len++;
				_x = 0;
				continue; 
			}
			if(_y >= 0 && !(y0 + max(0, _y) > y1)) // do not render if tiles have been "scrolled" out of view - or if the text overflows
				tl_plot_smbl(grid, x0 + _x, y0 + _y, t_data.text[i], smbl_col, t_data.style.font);
			_x++;
		}
			
		uint_t max_scroll = max((int)total_text_len - (int)(y1-y0), 0);
		container->scroll_offset = umin(container->scroll_offset, max_scroll);

		if(hovered && cursor->scroll != 0)
			tl_plot_bg(grid, x0, y0 + (y1-y0) * ((float)container->scroll_offset / (float)max_scroll), BLACK8B);
			

		return (parent_orientation == VERTICAL)? x1-x0 + 1 : y1-y0 + 1;
	}
	else if(c_type == SUBGRID)
	{
		subgrid_data_t *subg_data = ascui_get_subgrid_data(*container);

		if(!subg_data->subgrid)
			ERROR("\n UI drawing error: Subgrid not initialized!")
		
		if(cursor->hovered_container == container)
			subg_data->subgrid->tile_p_w -= cursor->scroll;		// "Suggest" new tile width
		tl_fit_subgrid(grid, subg_data->subgrid, x0, y0, x1, y1);	// new tile width sanitized by tl_fit_subgrid
		// draw_cursor_selection(grid, cursor, container, x0, y0, x1, y1, 0);
	}
	else if(c_type == BUTTON)
	{
		hovered = check_cursor_hover(cursor, container, x0, y0, x1, y1);
		button_data_t bt_data = *ascui_get_button_data(*container);

		color8b_t bg_col;
		color8b_t smbl_col;
		if(hovered || selected)
		{
			bg_col = bt_data.style.char_col;
			smbl_col = bt_data.style.bg_col;
		}
		else
		{
			bg_col = bt_data.style.bg_col;
			smbl_col = bt_data.style.char_col;
		}


		uchar_t horizontal_space = x1 - x0 + 1;
		uchar_t vertical_space = y1 - y0 + 1;
		uchar_t vertical_midpoint = vertical_space / 2;
		int  horizontal_start = ((int)horizontal_space - (int)bt_data.text_len) / 2;

		tl_draw_rect_bg(grid, x0, y0, x1, y1, bg_col);

		if (horizontal_start >= 0)
		{
			for (uint_t i = 0; i < bt_data.text_len; i++)
			{
				tl_plot_smbl(grid, x0 + horizontal_start + i, y0 + vertical_midpoint, bt_data.text[i], smbl_col, bt_data.style.font);
			}
		}
		else
		{
			uchar_t _x = 0;
			uchar_t _y = 0;
			
			for (uint_t i = 0; i < bt_data.text_len; i++)
			{
			
				if (x0 + _x > x1)
					{_y++; _x = 0; }
					
				if(y0 + _y < y1)
					break;
				
				if (bt_data.text[i] == '\n')
				{	
					_y++;
					_x = 0;
					continue; 
				}
				
				tl_plot_smbl(grid, x0 + _x, y0 + _y, bt_data.text[i], smbl_col, bt_data.style.font);
				_x++;
			}
		}
		return (parent_orientation == VERTICAL)? x1-x0 + 1 : y1-y0 + 1;
	}
	else return 0;
}

void ascui_draw_ui(grid_t *grid, container_t *top_container, cursor_t *cursor)
{
	cursor->hovered_container = NULL; // Reset hovered container

	Pos_t grid_size = tl_grid_get_dimensions(grid);
	container_data_t c_data = *ascui_get_container_data(*top_container);
	ascui_draw_container(grid, top_container, 0, 0, grid_size.x - 1, grid_size.y - 1, c_data.orientation, cursor);
}

static container_t create_container_stub(bool open, size_type_e s_type, uchar_t size, container_type_e c_type)
{
	container_t	container;
	container.open = open;
	container.scroll_offset = 0;
	container.size_type = s_type;
	container.size = size;
	container.container_type = c_type;

	return container;
}

container_data_t *ascui_get_container_data(container_t container)
{
	return (container_data_t *)container.container_type_data;
}

container_t ascui_create_container(bool open, size_type_e s_type, uchar_t size,
									container_orientation_e orientation, uint_t n_subcontainers)
{
	container_t container = create_container_stub(open, s_type, size, CONTAINER);
	container.container_type_data = calloc(1, sizeof(container_data_t));
	
	container_data_t *c_data = ascui_get_container_data(container);

	c_data->orientation = orientation;
	c_data->n_subcontainers = n_subcontainers;
	if (n_subcontainers != 0)
		c_data->subcontainers = calloc(n_subcontainers, sizeof(container_t)); // container owns subcontainers

	return container;
}

box_data_t *ascui_get_box_data(container_t container)
{
	return (box_data_t *)container.container_type_data;
}

container_t ascui_create_box(bool open, size_type_e s_type, uchar_t size, container_orientation_e orientation, 
								uint_t n_subcontainers, container_style_t style)
{
	container_t box = create_container_stub(open, s_type, size, BOX);
	box.container_type_data = calloc(1, sizeof(box_data_t));
	
	box_data_t *b_data = ascui_get_box_data(box);

	b_data->style = style;
	b_data->orientation = orientation;
	b_data->n_subcontainers = n_subcontainers;
	if (n_subcontainers != 0)
			b_data->subcontainers = calloc(n_subcontainers, sizeof(container_t)); // container owns subcontainers

	return box;
}

text_data_t *ascui_get_text_data(container_t container)
{
	return (text_data_t *)container.container_type_data;
}

container_t ascui_create_text(bool open, size_type_e s_type, uchar_t size, uint_t text_len, char *text, container_style_t style)
{
	container_t text_container = create_container_stub(open, s_type, size, TEXT);
	text_container.container_type_data = calloc(1, sizeof(text_data_t));
	
	text_data_t *t_data = ascui_get_text_data(text_container);

	t_data->text = calloc(text_len, sizeof(char));
	strcpy(t_data->text, text); // container owns text
	
	t_data->text_len = text_len;
	t_data->style = style;
	return text_container;
}

subgrid_data_t *ascui_get_subgrid_data(container_t container)
{
	return (subgrid_data_t *)container.container_type_data;
}

container_t ascui_create_subgrid(bool open, size_type_e s_type, uchar_t size, grid_t *subgrid)
{
	container_t subgrid_container = create_container_stub(open, s_type, size, SUBGRID);
	subgrid_container.container_type_data = calloc(1, sizeof(subgrid_data_t));

	subgrid_data_t *subg_data = ascui_get_subgrid_data(subgrid_container);

	subg_data->subgrid = subgrid;

	return subgrid_container;
}

button_data_t *ascui_get_button_data(container_t container)
{
	return (button_data_t *)container.container_type_data;
}

container_t ascui_create_button(bool open, size_type_e s_type, uchar_t size, 
								container_style_t style, UI_side_effect_func side_effect_func, uint_t text_len, char *text, void *domain, void *function_data)
{
	container_t button = create_container_stub(open, s_type, size, BUTTON);
	button.container_type_data = calloc(1, sizeof(button_data_t));
	
	button_data_t *bt_data = ascui_get_button_data(button);

	bt_data->text = calloc(text_len, sizeof(char));
	strcpy(bt_data->text, text); // container owns text
	bt_data->text_len = text_len;
	
	bt_data->style = style;
	bt_data->side_effect_func = side_effect_func;
	bt_data->domain = domain;
	bt_data->function_data = function_data;
	return button;
}


container_t *ascui_get_nth_subcontainer(container_t container, uint_t n)
{
	 container_type_e type = container.container_type;

	container_t *subcontainers;
	uint_t n_subcontainers;
	if(type == CONTAINER)
	{
		subcontainers = ascui_get_container_data(container)->subcontainers;
		n_subcontainers = ascui_get_container_data(container)->n_subcontainers;
	}
	else if(type == BOX)
	{
		subcontainers = ascui_get_box_data(container)->subcontainers;
		n_subcontainers = ascui_get_box_data(container)->n_subcontainers;
	}
	else
		assert(false);
		
	assert(n < n_subcontainers);

	return &(subcontainers[n]);
}

void ascui_set_nth_subcontainer(container_t container, uint_t n, container_t subcontainer)
{
	container_t *subcontainer_address = ascui_get_nth_subcontainer(container, n);
	*subcontainer_address = subcontainer;
}


static void _print_ui(container_t container, uint_t indentation, bool last_child)
{
	int line_len = 0;
	putc('\n', stdout);
	for (uint_t i = 0; i < indentation; i++)
	{
		if (i == indentation - 1)
			if(last_child)
				printf("  └──");
			else
				printf("  ├──");
		else
			printf("  │  ");
		line_len += 5;
	}
	switch (container.container_type)
	{
		case CONTAINER:
			printf("[CONTAINER]");
			line_len += strlen("[CONTAINER]");
			break;
		case BOX:
			printf("[BOX]");
			line_len += strlen("[BOX]");
			break;
		case TEXT:
			printf("[TEXT]");
			line_len += strlen("[TEXT]");
			break;
		case SUBGRID:
			printf("[SUBGRID]");
			line_len += strlen("[SUBGRID]");
			break;
		case BUTTON:
			printf("[BUTTON]");
			line_len += strlen("[BUTTON]");
			break;
		default:
			puts("[?]");
			line_len += strlen("[?]");
			break;
	}

	uint_t n_subcontainers;
	container_t *subcontainers;
	
	if (container.container_type == CONTAINER)
	{
		container_data_t c_data = *ascui_get_container_data(container);
		n_subcontainers = c_data.n_subcontainers;
		subcontainers = c_data.subcontainers;
	}
	else if (container.container_type == BOX)
	{
		box_data_t b_data = *ascui_get_box_data(container);
		n_subcontainers = b_data.n_subcontainers;
		subcontainers = b_data.subcontainers;
	}
	else return;
	
	for (uint_t i = 0; i < n_subcontainers; i++)
		_print_ui(subcontainers[i], indentation + 1, (i == n_subcontainers - 1));
}

void ascui_print_ui(container_t container)
{
	_print_ui(container, 0, false);
	putc('\n', stdout);
}

void ascui_destroy(container_t container)
{
	container_type_e type = container.container_type;

	container_t *subcontainers;
	uint_t n_subcontainers = 0;
	if(type == CONTAINER)
	{
		subcontainers = ascui_get_container_data(container)->subcontainers;
		n_subcontainers = ascui_get_container_data(container)->n_subcontainers;
	}
	else if(type == BOX)
	{
		subcontainers = ascui_get_box_data(container)->subcontainers;
		n_subcontainers = ascui_get_box_data(container)->n_subcontainers;
	}

	if(n_subcontainers != 0)
	{
		for (uint_t i = 0; i < n_subcontainers; i++)
		{
			ascui_destroy(subcontainers[i]);
		}
	}

	free(container.container_type_data);
}	

///	UI CONSTRUCTOR ///

// Tag List //
// The tag list is meant to be filled with tag-container pairs during program setup for easy fetching of specific containers
// Potential improvements: Hashing?

container_tag_list_t ascui_tag_list_create(uint_t init_capacity)
{
	container_tag_list_t list;
	list.capacity = init_capacity;
	list.count = 0;
	list.tags = calloc(init_capacity, sizeof(container_tag_t));
	
	return list;
}

void ascui_tag_list_add(container_tag_list_t *list, container_t *container, char *tag)
{
	assert(strlen(tag) <= TAG_MAX_LEN);

	if (list->capacity <= list->count)
	{
		list->capacity *= REALLOC_PERCENTAGE_INCREASE; 
		list->tags = (container_tag_t *)realloc(list->tags, (list->capacity * sizeof(container_tag_t)));
	}

	int comparison;
	
	if(list->count == 0)
	{
		list->tags[0].container = container;
		strcpy(list->tags[0].tag, tag);
		list->count++;
		return;
	}
		

	for (uint_t i = 0; i < (uint_t)list->count; i++)
	{
		comparison = strcmp(tag, list->tags[i].tag);
		if(comparison > 0)
		{
			// Move all tags from and including i one step forward
			memmove(&list->tags[i + 1], &list->tags[i], (list->count - i) * sizeof(container_tag_t));
			
			list->tags[i].container = container;
			strcpy(list->tags[i].tag, tag);
			break;
		}
		else if (comparison == 0)
		{
			ERRORF("ERROR: tag:'%s' already exists in list", tag)
		}
	}
	list->count++;
}
container_t *ascui_tag_list_get(container_tag_list_t list, char *tag)
{
	// binary search
	uint_t search_area_min = 0;
	uint_t search_area_max = list.count;
	uint_t i;
	while (search_area_min <= search_area_max)
	{
		i = (search_area_max + search_area_min) / 2;
		
		int comp = strcmp(tag, list.tags[i].tag);
		if (comp == 0)
			return list.tags[i].container;
		if (comp < 0) // i too large
		{
			search_area_max = i - 1;
		}
		if (comp > 0) // i too small
		{
			search_area_min = i + 1;
		}
	}

	return NULL; // tag not present
}

void ascui_tag_list_destroy(container_tag_list_t list)
{
	free(list.tags);
}

void ascui_tag_list_print(container_tag_list_t list)
{
	printf("\nTag list [capacity=%u, count=%u]", list.capacity, list.count);
	for (uint_t i = 0; i < list.capacity; i++)
		if(list.tags[i].container != NULL)
			printf("\n\t%u: '%s'=[%p]", i, list.tags[i].tag, list.tags[i].container);
}


//// PARSER FUNCTIONS /////
#define FLAG_VERTICAL 'V'
#define FLAG_HORIZONTAL 'H'
#define FLAG_CLOSED 'X'



#define PARAM_PERCENTAGE '%'
#define PARAM_TILES 'T'
#define PARAM_GROW 'G'
#define PARAM_STYLE '$'
#define PARAM_TEXT 't'
#define PARAM_TAG '#'

#define DECL_STYLE '$'
#define DECL_CONTAINER 'C'

#define ASSIGNMENT '='
#define REFERENCE ':'

#define COMMENT '/'


#define DELIMITATORS " [],\n"
#define MAX_LINE_LEN 256
#define MAX_STYLES 64
int N_LINES = 0; // mutable global variable

// https://stackoverflow.com/questions/12733105/c-function-that-counts-lines-in-file
int count_lines(FILE* file)
{
	#define BUF_SIZE 65536
    char buf[BUF_SIZE];
    int counter = 0;
    for(;;)
    {
        size_t res = fread(buf, 1, BUF_SIZE, file);
        if (ferror(file))
            return -1;

        int i;
        for(i = 0; i < (int)res; i++)
            if (buf[i] == '\n')
                counter++;

        if (feof(file))
            break;
    }

    return counter;
}

uint_t count_indentations(char ui_file[N_LINES][MAX_LINE_LEN], uint_t i)
{
	uint_t j = 0;
	while (ui_file[i][j] == '\t')
		j++;
	return j;
}

#define COUNT_PARENT -2
uint_t count_subcontainers(char ui_file[N_LINES][MAX_LINE_LEN], uint_t i, int parent_indentation)
{
	if(parent_indentation == COUNT_PARENT)
		parent_indentation = count_indentations(ui_file, i); // Count parent
	uint_t n_subcontainers = 0;
	uint_t c_indentation;
	for (uint_t c_line = i + 1; c_line < (uint_t)N_LINES; c_line++)
	{
		c_indentation = count_indentations(ui_file, c_line);
		if (ui_file[c_line][c_indentation] == DECL_CONTAINER) // Container, not a comment etc.
		{
			if ((int)c_indentation == parent_indentation + 1) // Subcontainer -> ++;
				n_subcontainers++;
			if ((int)c_indentation > parent_indentation + 1) // Subsubcontainer -> skip
				continue;
			if ((int)c_indentation <= parent_indentation) // Sibling container or higher -> no more subcontainers
				break;
		}	
	}

	return n_subcontainers;
}

// assumes strtok already initialized
color8b_t parse_color8b()
{
	char *tok = strtok(NULL, DELIMITATORS);
	uint_t r = atoi(tok);
	tok = strtok(NULL, DELIMITATORS);
	uint_t g = atoi(tok);
	tok = strtok(NULL, DELIMITATORS);
	uint_t b = atoi(tok);
	return col8bt(umin(r, 7),umin(g, 7),umin(b, 3));
}

void parse_style(char line[MAX_LINE_LEN], container_style_t *style_list)
{
	char font;
	color8b_t bg_col;
    color8b_t border_col;
    color8b_t char_col;
    char border_h_symbol;
    char border_v_symbol;
    char corner_symbol;
    int index;
	char *tok = strtok(line, DELIMITATORS);
	index = atoi(&tok[1]);

	tok = strtok(NULL, DELIMITATORS);
	font = atoi(tok);

	bg_col = parse_color8b();
	
	border_col = parse_color8b();

	char_col = parse_color8b();

	tok = strtok(NULL , DELIMITATORS);
	border_h_symbol = *tok;
	tok = strtok(NULL , DELIMITATORS);
	border_v_symbol = *tok;
	tok = strtok(NULL , DELIMITATORS);
	corner_symbol = *tok;

	style_list[index] = style(font, bg_col, border_col, char_col, border_h_symbol, border_v_symbol, corner_symbol);
}

char tab_followed_char(char line[MAX_LINE_LEN])
{
	for (uint_t i = 0; i < MAX_LINE_LEN; i++)
	{
		if(line[i] != '\t')
			return line[i];
	}
	return 0;
}

uint_t insert_child_container_into_parent(container_t child, container_t *parent)
{
	container_t *subcontainers;
	uint_t n_subcontainers;
	if (parent->container_type == CONTAINER)
	{
		subcontainers = ascui_get_container_data(*parent)->subcontainers;
		n_subcontainers = ascui_get_container_data(*parent)->n_subcontainers;
	}
	else if (parent->container_type == BOX)
	{
		subcontainers = ascui_get_box_data(*parent)->subcontainers;
		n_subcontainers = ascui_get_box_data(*parent)->n_subcontainers;
	}
	else
		ERROR("\nChild insertion error: bad type, parent is not fit to have children.")

	for (uint_t i = 0; i < n_subcontainers; i++)
	{
		if(subcontainers[i].container_type_data == NULL) // container_type_data being NULL should imply an empty slot.
		{
			subcontainers[i] = child;
			return i;
		}
	}
	
	ERROR("\nChild insertion error: cannot fit child into parent.")	
}

void parse_container_tree(char ui_file[N_LINES][MAX_LINE_LEN], uint_t i, container_t *c_parent, 
						container_tag_list_t *tag_list, container_style_t style_list[MAX_STYLES]);

void parse_subcontainers(char ui_file[N_LINES][MAX_LINE_LEN], uint_t i, int parent_indentation, container_t *c_parent,
						container_tag_list_t *tag_list, container_style_t style_list[MAX_STYLES])
{
	uint_t c_indentation;
	for (uint_t c_line = i + 1; c_line < (uint_t)N_LINES; c_line++)
	{
		c_indentation = count_indentations(ui_file, c_line);
		if (ui_file[c_line][c_indentation] == DECL_CONTAINER) // Container, not a comment etc.
		{
			if (c_indentation == (uint_t)parent_indentation + 1) // Subcontainer -> parse;
				parse_container_tree(ui_file, c_line, c_parent, tag_list, style_list);
			if (c_indentation > (uint_t)parent_indentation + 1) // Subsubcontainer -> skip
				continue;
			if (c_indentation <= (uint_t)parent_indentation) // Sibling container or higher -> no more subcontainers
				break;
		}
	}
}

void parse_container(char ui_file[N_LINES][MAX_LINE_LEN], uint_t i, container_t *c_parent, 
						container_tag_list_t *tag_list, container_style_t style_list[MAX_STYLES])
{
	char tag[TAG_MAX_LEN];
	tag[0] = 0;
	bool open = true;
	size_type_e s_type = TILES;
	uint_t size = 0;
	container_orientation_e orientation = VERTICAL;
	uint_t c_indentation = count_indentations(ui_file, i);
	uint_t n_subcontainers = 0;
	n_subcontainers = count_subcontainers(ui_file, i, c_indentation);

	char *tok;
	
	while ((tok = strtok(NULL , DELIMITATORS)))
	{
		if (*tok == COMMENT) // Comment reached
			break;
	
		switch (*tok)
		{
			case PARAM_TILES:
				s_type = TILES;
				size = atoi(&tok[2]);
				break;
			case PARAM_PERCENTAGE:
				s_type = PERCENTAGE;
				size = atoi(&tok[2]);
				break;
			case FLAG_VERTICAL:
				orientation = VERTICAL;
				break;
			case FLAG_HORIZONTAL:
				orientation = HORIZONTAL;
				break;
			case PARAM_STYLE:
				ERRORF("\nContainer-parsing error: container does not take $:style - at line %u.", i)
			case PARAM_TAG:
				strcpy(tag, &tok[1]);
				break;
			case FLAG_CLOSED:
				open = false;
				break;
			case PARAM_TEXT:
				ERRORF("\nContainer-parsing error: container does not take t:txt - at line %u.", i)
			default:
				ERRORF("\nContainer-parsing error: unknown argument at line %u.", i)
		}
	}
	

	container_t new_container = ascui_create_container(open, s_type, size, orientation, n_subcontainers);
	uint_t index = insert_child_container_into_parent(new_container, c_parent);
	container_t *new_container_ptr = ascui_get_nth_subcontainer(*c_parent, index);
	
	if (tag[0] != 0)
		ascui_tag_list_add(tag_list, new_container_ptr, tag);

	if(n_subcontainers != 0)
		parse_subcontainers(ui_file, i, c_indentation, new_container_ptr, tag_list, style_list);
}

void parse_box(char ui_file[N_LINES][MAX_LINE_LEN], uint_t i, container_t *c_parent, 
						container_tag_list_t *tag_list, container_style_t style_list[MAX_STYLES])
{
	char tag[TAG_MAX_LEN];
	tag[0] = 0;
	bool open = true;
	size_type_e s_type = TILES;
	uint_t size = 0;
	container_orientation_e orientation = VERTICAL;
	uint_t c_indentation = count_indentations(ui_file, i);
	uint_t n_subcontainers = 0;
	n_subcontainers = count_subcontainers(ui_file, i, c_indentation);
	container_style_t style = style_list[0]; // 0 as default

	char *tok;
	
	while ((tok = strtok(NULL , DELIMITATORS)))
	{
		if (*tok == '\t')
			continue;
			
		if (*tok == COMMENT) // Comment reached
			break;
	
		switch (*tok)
		{
			case PARAM_TILES:
				s_type = TILES;
				size = atoi(&tok[2]);
				break;
			case PARAM_PERCENTAGE:
				s_type = PERCENTAGE;
				size = atoi(&tok[2]);
				break;
			case FLAG_VERTICAL:
				orientation = VERTICAL;
				break;
			case FLAG_HORIZONTAL:
				orientation = HORIZONTAL;
				break;
			case PARAM_STYLE:
				style = style_list[atoi(&tok[2])]; // defaults to $:0 on error
				break;
			case PARAM_TAG:
				strcpy(tag, &tok[1]);
				break;
			case FLAG_CLOSED:
				open = false;
				break;
			case PARAM_TEXT:
				ERRORF("\nContainer-parsing error: box does not take t:txt - at line %u.", i)
			default:
				ERRORF("\nContainer-parsing error: unknown argument at line %u.", i)
		}
	}
	

	container_t new_container = ascui_create_box(open, s_type, size, orientation, n_subcontainers, style);
	uint_t index = insert_child_container_into_parent(new_container, c_parent);
	container_t *new_container_ptr = ascui_get_nth_subcontainer(*c_parent, index);
	
	if (tag[0] != 0)
		ascui_tag_list_add(tag_list, new_container_ptr, tag);

	if(n_subcontainers != 0)
		parse_subcontainers(ui_file, i, c_indentation, new_container_ptr, tag_list, style_list);
}

void replace_underscores(char *str)
{
	uint_t i = 0;
	while(str[i] != 0)
	{
		if(str[i] == '_')
			str[i] = ' ';
		i++;
	}
}

void parse_text(char ui_file[N_LINES][MAX_LINE_LEN], uint_t i, container_t *c_parent, 
						container_tag_list_t *tag_list, container_style_t style_list[MAX_STYLES])
{
	char tag[TAG_MAX_LEN];
	tag[0] = 0;
	char txt[MAX_LINE_LEN];
	txt[0] = 0;

	int txt_index = -1;
	
	bool open = true;
	size_type_e s_type = TILES;
	uint_t size = 0;
	container_style_t style = style_list[0]; // 0 as default

	char *tok;
	
	while ((tok = strtok(NULL , DELIMITATORS)))
	{
		if (*tok == COMMENT) // Comment reached
			break;
	
		switch (*tok)
		{
			case PARAM_TILES:
				s_type = TILES;
				size = atoi(&tok[2]);
				break;
			case PARAM_PERCENTAGE:
				s_type = PERCENTAGE;
				size = atoi(&tok[2]);
				break;
			case FLAG_VERTICAL:
				WARNINGF("\nContainer-parsing warning: text does not need orientation - at line %u.", i)
				break;
			case FLAG_HORIZONTAL:
				WARNINGF("\nContainer-parsing warning: text does not need orientation - at line %u.", i)
				break;
			case PARAM_STYLE:
				style = style_list[atoi(&tok[2])]; // defaults to $:0 on error
				break;
			case PARAM_TAG:
				strcpy(tag, &tok[1]);
				break;
			case FLAG_CLOSED:
				open = false;
				break;
			case PARAM_TEXT:
				if(tok[1] == ASSIGNMENT) // take text as parameter
				{
					strcpy(txt, &tok[2]);
					replace_underscores(txt);
				}
				else if (tok[1] == REFERENCE) // treat parameter as index - read text from texts-list
					txt_index = atoi(&tok[2]);
				break;
			default:
				ERRORF("\nContainer-parsing error: unknown argument at line %u.", i)
		}
	}
	

	container_t new_container;

	if(txt_index == -1) // Text from parameter
	 	new_container = ascui_create_text(open, s_type, size, strlen(txt), txt, style);
	else 				// Text from texts[txt_index]
	 	new_container = ascui_create_text(open, s_type, size, strlen(texts[txt_index]), texts[txt_index], style);

	uint_t index = insert_child_container_into_parent(new_container, c_parent);
	container_t *new_container_ptr = ascui_get_nth_subcontainer(*c_parent, index);
	
	if (tag[0] != 0)
		ascui_tag_list_add(tag_list, new_container_ptr, tag);
}


void parse_button(char ui_file[N_LINES][MAX_LINE_LEN], uint_t i, container_t *c_parent, 
						container_tag_list_t *tag_list, container_style_t style_list[MAX_STYLES])
{
	char tag[TAG_MAX_LEN];
	tag[0] = 0;
	char txt[MAX_LINE_LEN];
	txt[0] = 0;

	int txt_index = -1;
	
	bool open = true;
	size_type_e s_type = TILES;
	uint_t size = 0;
	container_style_t style = style_list[0]; // 0 as default

	char *tok;
	
	while ((tok = strtok(NULL , DELIMITATORS)))
	{
		if (*tok == COMMENT) // Comment reached
			break;
	
		switch (*tok)
		{
			case PARAM_TILES:
				s_type = TILES;
				size = atoi(&tok[2]);
				break;
			case PARAM_PERCENTAGE:
				s_type = PERCENTAGE;
				size = atoi(&tok[2]);
				break;
			case FLAG_VERTICAL:
				WARNINGF("\nContainer-parsing warning: button does not need orientation - at line %u.", i)
				break;
			case FLAG_HORIZONTAL:
				WARNINGF("\nContainer-parsing warning: button does not need orientation - at line %u.", i)
				break;
			case PARAM_STYLE:
				style = style_list[atoi(&tok[2])]; // defaults to $:0 on error
				break;
			case PARAM_TAG:
				strcpy(tag, &tok[1]);
				break;
			case FLAG_CLOSED:
				open = false;
				break;
			case PARAM_TEXT:
				if(tok[1] == ASSIGNMENT) // take text as parameter
				{
					strcpy(txt, &tok[2]);
					replace_underscores(txt);
				}
				else if (tok[1] == REFERENCE) // treat parameter as index - read text from texts-list
					txt_index = atoi(&tok[2]);
				break;
			default:
				ERRORF("\nContainer-parsing error: unknown argument at line %u.", i)
		}
	}
	

	container_t new_container;

	if(txt_index == -1) // Text from parameter
	 	new_container = ascui_create_button(open, s_type, size, style, NULL, strlen(txt), txt, NULL, NULL);
	else 				// Text from texts[txt_index]
	 	new_container = ascui_create_button(open, s_type, size, style, NULL, strlen(texts[txt_index]), texts[txt_index], NULL, NULL);

	uint_t index = insert_child_container_into_parent(new_container, c_parent);
	container_t *new_container_ptr = ascui_get_nth_subcontainer(*c_parent, index);
	
	if (tag[0] != 0)
		ascui_tag_list_add(tag_list, new_container_ptr, tag);
}

void parse_subgrid(char ui_file[N_LINES][MAX_LINE_LEN], uint_t i, container_t *c_parent, 
						container_tag_list_t *tag_list, container_style_t style_list[MAX_STYLES])
{
	char tag[TAG_MAX_LEN];
	tag[0] = 0;
	
	bool open = true;
	size_type_e s_type = TILES;
	uint_t size = 0;
	char *tok;
	
	while ((tok = strtok(NULL , DELIMITATORS)))
	{
		if (*tok == COMMENT) // Comment reached
			break;
	
		switch (*tok)
		{
			case PARAM_TILES:
				s_type = TILES;
				size = atoi(&tok[2]);
				break;
			case PARAM_PERCENTAGE:
				s_type = PERCENTAGE;
				size = atoi(&tok[2]);
				break;			
			case FLAG_VERTICAL:
				WARNINGF("\nContainer-parsing warning: subgrid does not need orientation - at line %u.", i)
				break;
			case FLAG_HORIZONTAL:
				WARNINGF("\nContainer-parsing warning: subgrid does not need orientation - at line %u.", i)
				break;
			case PARAM_STYLE:
				WARNINGF("\nContainer-parsing warning: subgrid does not need style - at line %u.", i)
				break;
			case PARAM_TAG:
				strcpy(tag, &tok[1]);
				break;
			case FLAG_CLOSED:
				open = false;
				break;
			case PARAM_TEXT:
				WARNINGF("\nContainer-parsing warning: subgrid does not need text - at line %u.", i)
				break;
			default:
				ERRORF("\nContainer-parsing error: unknown argument at line %u.", i)
		}
	}
	
	container_t new_container = ascui_create_subgrid(open, s_type, size, NULL);

	uint_t index = insert_child_container_into_parent(new_container, c_parent);
	container_t *new_container_ptr = ascui_get_nth_subcontainer(*c_parent, index);
	
	if (tag[0] != 0)
		ascui_tag_list_add(tag_list, new_container_ptr, tag);
	else
		WARNINGF("\nContainer-parsing warning: subgrid does not have a tag at line %u. (which is recommended)", i)
		
}
void parse_container_tree(char ui_file[N_LINES][MAX_LINE_LEN], uint_t i, container_t *c_parent, 
						container_tag_list_t *tag_list, container_style_t style_list[MAX_STYLES])
{
	uint_t c_indentation = count_indentations(ui_file, i);
	// 									V	start tokenization after tabs
	char *tok = strtok(&ui_file[i][c_indentation], DELIMITATORS); 

	char *type_str = &tok[2];
	
	if (strcmp(type_str, "box") == 0)
		parse_box(ui_file, i, c_parent, tag_list, style_list);
	else if (strcmp(type_str, "container") == 0)
		parse_container(ui_file, i, c_parent, tag_list, style_list);
	else if (strcmp(type_str, "button") == 0)
		parse_button(ui_file, i, c_parent, tag_list, style_list);
	else if (strcmp(type_str, "text") == 0)
		parse_text(ui_file, i, c_parent, tag_list, style_list);
	else if (strcmp(type_str, "subgrid") == 0) 
		parse_subgrid(ui_file, i, c_parent, tag_list, style_list);
	else ERRORF("\nContainer-parsing error: unknown container type at line %u.", i)
	
	
}


container_t *ascui_construct_ui_from_file(char *ui_file_path, container_tag_list_t *tag_list)
{	
	FILE *file = fopen(ui_file_path, "r");
	if (file == NULL) {
	    perror("Error opening file");
		fclose(file);
		return NULL;
	}
	
	N_LINES = count_lines(file); //count_lines mangles the file ptr, reopen file. Is there a less stupid way?
	if(N_LINES == -1)
	{
	    perror("Error counting lines in file");
		fclose(file);
		return NULL;
	}
	
	fclose(file);
	file = fopen(ui_file_path, "r");


	// Read file into an array of strings
	char ui_file[N_LINES][MAX_LINE_LEN];
	uint_t line_num = 0;
	while (fgets(ui_file[line_num], MAX_LINE_LEN, file)) 
	{
		line_num++;
 	}
	fclose(file);

	container_style_t style_list[MAX_STYLES];
	container_style_t *style_list_ptr = &(style_list[0]);

	uint_t i;
	// Parse styles
	for (i = 0; i < (uint_t)N_LINES; i++)
	{
		if(ui_file[i][0] == DECL_STYLE) // Style declaration
			parse_style(ui_file[i], style_list_ptr);
		
		if(ui_file[i][0] == FLAG_HORIZONTAL || ui_file[i][0] == FLAG_VERTICAL) // Containers reached
			break;
	}

	assert(ui_file[i][0] == FLAG_HORIZONTAL || ui_file[i][0] == FLAG_VERTICAL);

	container_orientation_e top_c_orientation = (ui_file[i][0] == FLAG_HORIZONTAL)? HORIZONTAL : VERTICAL;

	container_t *top_container = calloc(1, sizeof(container_t));
	*top_container = ascui_create_container(true, PERCENTAGE, 100, top_c_orientation, 
											count_subcontainers(ui_file, 0, -1));


	// Parse every top-level subtree 
	for (uint_t i = 0; i < (uint_t)N_LINES; i++)
		if(ui_file[i][0] == DECL_CONTAINER) // top-level Container
			parse_container_tree(ui_file, i, top_container, tag_list, style_list);
			
	return top_container;
}
