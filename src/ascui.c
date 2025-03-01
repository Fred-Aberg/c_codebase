#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "common.h"
#include "ascui.h"
#include "raytiles.h"
#include "texts.h"

#define ERROR(str) {fprintf(stdout, str); putc('\n', stdout); fflush(stdout); assert(false);}
#define ERRORF(str, ...) {fprintf(stdout, str, __VA_ARGS__); putc('\n', stdout); fflush(stdout); assert(false);}
#define WARNINGF(str, ...) {fprintf(stdout, str, __VA_ARGS__); putc('\n', stdout);}
#define get_orientation(container) *(Container_orientation_e *)(container->container_type_data)


static void draw_cursor_selection(Grid_t *grid, Cursor_t *cursor, Container_t *container, uint x0, uint y0, uint x1, uint y1, uint max_scroll)
{
	if(cursor->selected_container != container) return;
	if (container->container_type == BOX || container->container_type == CONTAINER)
	{
		for (uint x = x0; x <= x1; x++)
		{
			tl_set_tile_bg(grid, x, y0, WHITE);
			tl_set_tile_bg(grid, x, y1, WHITE);
		}
		for (uint y = y0 + 1; y < y1; y++)
		{
			tl_set_tile_bg(grid, x0, y, WHITE);
			tl_set_tile_bg(grid, x1, y, WHITE);
		}
	}
	else
	{
		for (uint x = x0; x <= x1; x++)
		{
			for (uint y = y0; y <= y1; y++)
			{
				tl_set_tile_bg(grid, x, y, WHITE);
			}
		}
	}

	if(container->container_type == BOX || container->container_type == CONTAINER)
	{
		tl_set_tile_bg(grid, x0, y0 + (y1-y0) * ((float)container->scroll_offset / (float)max_scroll), BLACK);
	}
	if(cursor->scroll != 0 && max_scroll != 0)
	{
		if(get_orientation(container) == HORIZONTAL)
			tl_set_tile_bg(grid, x0, y0 + (y1-y0) * ((float)container->scroll_offset / (float)max_scroll), BLACK);
		else
			tl_set_tile_bg(grid, x0 + (x1-x0) * ((float)container->scroll_offset / (float)max_scroll), y0, BLACK);
	}
}

static void check_cursor_selection(Cursor_t *cursor, Container_t *container, uint x0, int y0, uint x1, int y1)
{		
	if(x0 <= cursor->x && x1 >= cursor->x && y0 <= (int)cursor->y && y1 >= (int)cursor->y)
		cursor->selected_container = container;
}

static void draw_box(Grid_t *grid, Container_style_t style, uint x0, uint y0, uint x1, uint y1)
{
	tl_draw_line(grid, x0 + 1, y0, x1 - 1, y0, style.border_h_symbol, style.char_col, style.border_col, NULL); // Top
	tl_draw_line(grid, x0 + 1, y1, x1 - 1, y1, style.border_h_symbol, style.char_col, style.border_col, NULL); // Bottom
	tl_draw_line(grid, x0, y0 + 1, x0, y1 - 1, style.border_v_symbol, style.char_col, style.border_col, NULL); // Left
	tl_draw_line(grid, x1, y0 + 1, x1, y1 - 1, style.border_v_symbol, style.char_col, style.border_col, NULL); // Right

	tl_draw_tile(grid, x0, y0, style.corner_symbol, style.char_col, style.border_col, NULL);
	tl_draw_tile(grid, x0, y1, style.corner_symbol, style.char_col, style.border_col, NULL);
	tl_draw_tile(grid, x1, y0, style.corner_symbol, style.char_col, style.border_col, NULL);
	tl_draw_tile(grid, x1, y1, style.corner_symbol, style.char_col, style.border_col, NULL);
}


static uint ascui_draw_container(Grid_t *grid, Container_t *container, uint x0, uint y0, uint x1, uint y1,
								 Container_orientation_e parent_orientation, Cursor_t *cursor);

/*
static uint ascui_draw_container_percentage_split(Grid_t *grid, Container_t *container, uint x0, uint y0, uint x1, uint y1, Container_orientation_e parent_orientation, Cursor_t *cursor)
{
	if (!container->open)
		return 0;

	if (x1 <= x0 || y1 <= y0)
		return 0;
		
	uint subcontainers_size = 0;
	float percentage;
	uint i = 0;
	uint tiles_drawn = 0;


	uint n_subcontainers;
	Container_t *subcontainers;
	Container_orientation_e orientation;
	Container_style_t style;
	
	if (container->container_type == CONTAINER)
	{
		Container_data_t c_data = *ascui_get_container_data(*container);
		n_subcontainers = c_data.n_subcontainers;
		subcontainers = c_data.subcontainers;
		orientation = c_data.orientation;
		
	}
	else if (container->container_type == BOX)
	{
		Box_data_t b_data = *ascui_get_box_data(*container);
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

	uint container_size = (orientation == VERTICAL)? x1-x0 + 1 : y1-y0 + 1;
	// printf("\n----\npre: %u", container_size);
		
	uint allotted_tiles[n_subcontainers];
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

static uint ascui_draw_container(Grid_t *grid, Container_t *container, uint x0, uint y0, uint x1, uint y1,
								 Container_orientation_e parent_orientation, Cursor_t *cursor)
{
	if (!container->open)
		return 0;

	if (x1 <= x0 || y1 <= y0)
		return 0;

	check_cursor_selection(cursor, container, x0, y0 , x1, y1);		

	float percentage;
	uint sub_x0;
	uint sub_y0;
	uint sub_x1;
	uint sub_y1;

	uint n_subcontainers;
	Container_t *c_subcontainer;
	Container_Type_e c_type = container->container_type;
	// bool percentage_split = true;

	uint tiles_drawn = 0;
	uint max_scroll = 0;
	 	
	if (c_type == CONTAINER || c_type == BOX)
	{
		Container_t *subcontainers;
		Container_orientation_e orientation;
		Container_style_t style;

		

		if (c_type == CONTAINER)
		{
			Container_data_t c_data = *ascui_get_container_data(*container);
			n_subcontainers = c_data.n_subcontainers;
			subcontainers = c_data.subcontainers;
			orientation = c_data.orientation;
			
		}
		else if (c_type == BOX)
		{
			Box_data_t b_data = *ascui_get_box_data(*container);
			n_subcontainers = b_data.n_subcontainers;
			subcontainers = b_data.subcontainers;
			orientation = b_data.orientation;
			style = b_data.style;
		}

		max_scroll = max((int)n_subcontainers - 1, 0);
		container->scroll_offset = umin(container->scroll_offset, max_scroll); // Clamp scroll

		/*
		for (uint i = 0; i < n_subcontainers; i++)
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

		for (uint i = container->scroll_offset; i < n_subcontainers; i++)
		{
			c_subcontainer = &subcontainers[i];
			// Edge case: Last subcontainer
			if (i == n_subcontainers - 1 && container->size_type != GROW)
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
			else if (c_subcontainer->size_type == GROW)
			{
				if (orientation == HORIZONTAL)
				{
					sub_x1 = x1;
					sub_y1 = y1;
					tiles_drawn += ascui_draw_container(grid, c_subcontainer, x0, y0 + tiles_drawn, sub_x1, sub_y1, orientation, cursor);
				}
				else if (orientation == VERTICAL)
				{
					sub_x1 = x1;
					sub_y1 = y1;
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
			
			if (container->size_type == GROW)
			{
				// GROW => tiles_drawn is the deciding factor in how many tiles is to be drawn
				if (parent_orientation == HORIZONTAL)
					y1 = y0 + tiles_drawn + 1; 
				else if (parent_orientation == VERTICAL)
					x1 = x0 + tiles_drawn + 1; 
			}
			draw_box(grid, style, x0, y0, x1, y1);
		}
		
		if (container->size_type != GROW)
		{
			// !GROW => Force the size to be x1-x0 or y1-y0
			if (parent_orientation == HORIZONTAL)
				tiles_drawn = y1-y0 + 1;
			else if (parent_orientation == VERTICAL)
				tiles_drawn = x1-x0 + 1;
		}
		
		draw_cursor_selection(grid, cursor, container, x0, y0, x1, y1, max_scroll);
		return tiles_drawn;
	}
	else if(c_type == TEXT)
	{
		Text_data_t t_data = *ascui_get_text_data(*container);
		
		uint _x = 0;
		container->scroll_offset = container->scroll_offset;
		int _y = - container->scroll_offset; // = 0 at no scroll
		uint total_text_len = 0;
		
		uint gap;

		for (uint i = 0; i < t_data.text_len; i++)
		{
			if (x0 + _x > x1)
				{total_text_len++;  _y++; _x = 0; }
			
			if (t_data.text[i] == '\n')
			{
				gap = x1-x0-_x+1;
				while (gap != 0)
				{
					if(_y >= 0 && !(y0 + max(0, _y) > y1))
						tl_draw_tile(grid, x0 + _x, y0 + _y, 0, t_data.text_col, t_data.bg_col, NULL);
					gap--;
					_x++;
				}	
				_y++;
				total_text_len++;
				_x = 0;
				continue; 
			}
			if(_y >= 0 && !(y0 + max(0, _y) > y1)) // do not render if tiles have been "scrolled" out of view - or if the text overflows
				tl_draw_tile(grid, x0 + _x, y0 + _y, t_data.text[i], t_data.text_col, t_data.bg_col, NULL);
			_x++;
		}

		// Gap after text
		gap = x1-x0-_x + 1;
		while (gap != 0)
		{
			if(_y >= 0 && !(y0 + max(0, _y) > y1)) // do not render if tiles have been "scrolled" out of view - or if the text overflows
				tl_draw_tile(grid, x0 + _x, y0 + _y, 0, t_data.text_col, t_data.bg_col, NULL);
			gap--;
			_x++;
		}

		if (container->size_type == GROW)
		{
			
			uint max_scroll = max((int)total_text_len - _y, 0);
			container->scroll_offset = umin(container->scroll_offset, max_scroll);
	
			draw_cursor_selection(grid, cursor, container, x0, y0, x1, total_text_len, max_scroll);
			return (parent_orientation == VERTICAL)? x1-x0 + 1 : total_text_len;
		}
		else // Gap below text
			tl_draw_rect(grid, x0, max(_y + 1, y0), x1-x0, y1-max(_y + 1, y0), 0, t_data.text_col, t_data.bg_col, NULL);
			
		uint max_scroll = max((int)total_text_len - (int)(y1-y0), 0);
		container->scroll_offset = umin(container->scroll_offset, max_scroll);

		draw_cursor_selection(grid, cursor, container, x0, y0, x1, y1, max_scroll);
		return (parent_orientation == VERTICAL)? x1-x0 + 1 : y1-y0 + 1;
	}
	else if(c_type == SUBGRID)
	{
		Subgrid_data_t *subg_data = ascui_get_subgrid_data(*container);
		tl_fit_subgrid(grid, subg_data->subgrid, x0, y0, x1, y1);
		draw_cursor_selection(grid, cursor, container, x0, y0, x1, y1, 0);
	}
	else if(c_type == BUTTON)
	{
		Button_data_t *bt_data = ascui_get_button_data(*container);

		uint horizontal_space = x1 - x0 + 1;
		uint vertical_space = y1 - y0 + 1;
		uint vertical_midpoint = vertical_space / 2;
		int  horizontal_start = ((int)horizontal_space - (int)bt_data->text_len) / 2;

		tl_draw_rect(grid, x0, y0, horizontal_space - 1, vertical_space - 1, 0, bt_data->style.char_col, bt_data->style.bg_col, NULL);

		if (horizontal_start >= 0)
		{
			for (uint i = 0; i < bt_data->text_len; i++)
			{
				tl_draw_tile(grid, x0 + horizontal_start + i, y0 + vertical_midpoint, bt_data->text[i], bt_data->style.char_col, bt_data->style.bg_col, NULL);
			}
		}
		else
		{
			uint _x = 0;
			uint _y = 0;
			uint gap;
			
			for (uint i = 0; i < bt_data->text_len; i++)
			{
				if (_x > horizontal_space - 1)
					{_y++; _x = 0; }
					
				if (_y > vertical_space - 1)
					break;
				
				if (bt_data->text[i] == '\n')
				{
					gap = x1-x0-_x+1;
					while (gap != 0)
					{
						tl_draw_tile(grid, x0 + _x, y0 + _y, 0, bt_data->style.char_col, bt_data->style.bg_col, NULL);
						gap--;
						_x++;
					}	
					_y++;
					_x = 0;
					continue; 
				}
			tl_draw_tile(grid, x0 + _x, y0 + _y, bt_data->text[i], bt_data->style.char_col, bt_data->style.bg_col, NULL);
			_x++;
			}
		}

		
		draw_cursor_selection(grid, cursor, container, x0, y0, x1, y1, max_scroll);
		return (parent_orientation == VERTICAL)? x1-x0 + 1 : y1-y0 + 1;
	}
	else return 0;
}

void ascui_draw_ui(Grid_t *grid, Container_t *top_container, Cursor_t *cursor)
{
	cursor->selected_container = NULL; // Reset selected container

	Pos_t grid_size = tl_grid_get_size(grid);
	Container_data_t c_data = *ascui_get_container_data(*top_container);
	ascui_draw_container(grid, top_container, 0, 0, grid_size.x - 1, grid_size.y - 1, c_data.orientation, cursor);
}

static Container_t create_container_stub(bool open, Size_Type_e s_type, uint size, Container_Type_e c_type)
{
	Container_t	container;
	container.open = open;
	container.scroll_offset = 0;
	container.size_type = s_type;
	container.size = size;
	container.container_type = c_type;

	return container;
}

Container_data_t *ascui_get_container_data(Container_t container)
{
	return (Container_data_t *)container.container_type_data;
}

Container_t ascui_create_container(bool open, Size_Type_e s_type, uint size,
									Container_orientation_e orientation, uint n_subcontainers)
{
	Container_t container = create_container_stub(open, s_type, size, CONTAINER);
	container.container_type_data = calloc(1, sizeof(Container_data_t));
	
	Container_data_t *c_data = ascui_get_container_data(container);

	c_data->orientation = orientation;
	c_data->n_subcontainers = n_subcontainers;
	if (n_subcontainers != 0)
		c_data->subcontainers = calloc(n_subcontainers, sizeof(Container_t)); // container owns subcontainers

	return container;
}

Box_data_t *ascui_get_box_data(Container_t container)
{
	return (Box_data_t *)container.container_type_data;
}

Container_t ascui_create_box(bool open, Size_Type_e s_type, uint size, Container_orientation_e orientation, 
								uint n_subcontainers, Container_style_t style)
{
	Container_t box = create_container_stub(open, s_type, size, BOX);
	box.container_type_data = calloc(1, sizeof(Box_data_t));
	
	Box_data_t *b_data = ascui_get_box_data(box);

	b_data->style = style;
	b_data->orientation = orientation;
	b_data->n_subcontainers = n_subcontainers;
	if (n_subcontainers != 0)
			b_data->subcontainers = calloc(n_subcontainers, sizeof(Container_t)); // container owns subcontainers

	return box;
}

Text_data_t *ascui_get_text_data(Container_t container)
{
	return (Text_data_t *)container.container_type_data;
}

Container_t ascui_create_text(bool open, Size_Type_e s_type, uint size, uint text_len, char *text, Color text_col, Color bg_col)
{
	Container_t text_container = create_container_stub(open, s_type, size, TEXT);
	text_container.container_type_data = calloc(1, sizeof(Text_data_t));
	
	Text_data_t *t_data = ascui_get_text_data(text_container);

	t_data->text = calloc(text_len, sizeof(char));
	strcpy(t_data->text, text); // container owns text
	
	t_data->text_len = text_len;
	t_data->text_col = text_col;
	t_data->bg_col = bg_col;
	return text_container;
}

Subgrid_data_t *ascui_get_subgrid_data(Container_t container)
{
	return (Subgrid_data_t *)container.container_type_data;
}

Container_t ascui_create_subgrid(bool open, Size_Type_e s_type, uint size, Grid_t *subgrid, Color default_color, Font *default_font)
{
	Container_t subgrid_container = create_container_stub(open, s_type, size, SUBGRID);
	subgrid_container.container_type_data = calloc(1, sizeof(Subgrid_data_t));

	Subgrid_data_t *subg_data = ascui_get_subgrid_data(subgrid_container);

	if (subgrid != NULL)
		subg_data->subgrid = subgrid;
	else
		subg_data->subgrid = tl_init_grid(0,0, 10, 10, 4, 1.0f, 1000, default_color, default_font);

	return subgrid_container;
}

Button_data_t *ascui_get_button_data(Container_t container)
{
	return (Button_data_t *)container.container_type_data;
}

Container_t ascui_create_button(bool open, Size_Type_e s_type, uint size, 
								Container_style_t style, UI_side_effect_func side_effect_func, uint text_len, char *text, void *domain, void *function_data)
{
	Container_t button = create_container_stub(open, s_type, size, BUTTON);
	button.container_type_data = calloc(1, sizeof(Button_data_t));
	
	Button_data_t *bt_data = ascui_get_button_data(button);

	bt_data->text = calloc(text_len, sizeof(char));
	strcpy(bt_data->text, text); // container owns text
	bt_data->text_len = text_len;
	
	bt_data->style = style;
	bt_data->side_effect_func = side_effect_func;
	bt_data->domain = domain;
	bt_data->function_data = function_data;
	return button;
}


Container_t *ascui_get_nth_subcontainer(Container_t container, uint n)
{
	 Container_Type_e type = container.container_type;

	Container_t *subcontainers;
	uint n_subcontainers;
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

void ascui_set_nth_subcontainer(Container_t container, uint n, Container_t subcontainer)
{
	Container_t *subcontainer_address = ascui_get_nth_subcontainer(container, n);
	*subcontainer_address = subcontainer;
}


static void _print_ui(Container_t container, uint indentation, bool last_child)
{
	int line_len = 0;
	putc('\n', stdout);
	for (uint i = 0; i < indentation; i++)
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

	uint n_subcontainers;
	Container_t *subcontainers;
	
	if (container.container_type == CONTAINER)
	{
		Container_data_t c_data = *ascui_get_container_data(container);
		n_subcontainers = c_data.n_subcontainers;
		subcontainers = c_data.subcontainers;
	}
	else if (container.container_type == BOX)
	{
		Box_data_t b_data = *ascui_get_box_data(container);
		n_subcontainers = b_data.n_subcontainers;
		subcontainers = b_data.subcontainers;
	}
	else return;
	
	for (uint i = 0; i < n_subcontainers; i++)
		_print_ui(subcontainers[i], indentation + 1, (i == n_subcontainers - 1));
}

void ascui_print_ui(Container_t container)
{
	_print_ui(container, 0, false);
	putc('\n', stdout);
}

void ascui_destroy(Container_t container)
{
	Container_Type_e type = container.container_type;

	Container_t *subcontainers;
	uint n_subcontainers = 0;
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
		for (uint i = 0; i < n_subcontainers; i++)
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

Container_tag_list_t ascui_tag_list_create(uint init_capacity)
{
	Container_tag_list_t list;
	list.capacity = init_capacity;
	list.count = 0;
	list.tags = calloc(init_capacity, sizeof(Container_tag_t));
	
	return list;
}

void ascui_tag_list_add(Container_tag_list_t *list, Container_t *container, char *tag)
{
	assert(strlen(tag) <= TAG_MAX_LEN);

	if (list->capacity < list->count)
	{
		list->capacity *= REALLOC_PERCENTAGE_INCREASE; 
		list->tags = (Container_tag_t *)realloc(list->tags, (list->capacity * sizeof(Container_tag_t)));
	}

	int comparison;
	
	if(list->count == 0)
	{
		list->tags[0].container = container;
		strcpy(list->tags[0].tag, tag);
		list->count++;
		return;
	}
		

	for (uint i = 0; i < (uint)list->count; i++)
	{
		comparison = strcmp(tag, list->tags[i].tag);
		if(comparison > 0)
		{
			// Move all tags from and including i one step forward
			memmove(&list->tags[i + 1], &list->tags[i], (list->count - i) * sizeof(Container_tag_t));
			
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
Container_t *ascui_tag_list_get(Container_tag_list_t list, char *tag)
{
	// binary search
	uint search_area_min = 0;
	uint search_area_max = list.count;
	uint i;
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

void ascui_tag_list_destroy(Container_tag_list_t list)
{
	free(list.tags);
}

void ascui_tag_list_print(Container_tag_list_t list)
{
	printf("\nTag list [capacity=%u, count=%u]", list.capacity, list.count);
	for (uint i = 0; i < list.capacity; i++)
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

uint count_indentations(char ui_file[N_LINES][MAX_LINE_LEN], uint i)
{
	uint j = 0;
	while (ui_file[i][j] == '\t')
		j++;
	return j;
}

#define COUNT_PARENT -2
uint count_subcontainers(char ui_file[N_LINES][MAX_LINE_LEN], uint i, int parent_indentation)
{
	if(parent_indentation == COUNT_PARENT)
		parent_indentation = count_indentations(ui_file, i); // Count parent
	uint n_subcontainers = 0;
	uint c_indentation;
	for (uint c_line = i + 1; c_line < (uint)N_LINES; c_line++)
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
Color parse_color()
{
	uint r = atoi(strtok(NULL, DELIMITATORS));
	uint g = atoi(strtok(NULL, DELIMITATORS));
	uint b = atoi(strtok(NULL, DELIMITATORS));
	return c(r,g,b);
}

void parse_style(char line[MAX_LINE_LEN], Container_style_t *style_list)
{
	Color bg_col;
    Color border_col;
    Color char_col;
    char border_h_symbol;
    char border_v_symbol;
    char corner_symbol;
    int index;
	char *tok = strtok(line, DELIMITATORS);
	index = atoi(&tok[1]);

	bg_col = parse_color();
	
	border_col = parse_color();

	char_col = parse_color();

	tok = strtok(NULL , DELIMITATORS);
	border_h_symbol = *tok;
	tok = strtok(NULL , DELIMITATORS);
	border_v_symbol = *tok;
	tok = strtok(NULL , DELIMITATORS);
	corner_symbol = *tok;

	style_list[index] = style(bg_col, border_col, char_col, border_h_symbol, border_v_symbol, corner_symbol);
}

char tab_followed_char(char line[MAX_LINE_LEN])
{
	for (uint i = 0; i < MAX_LINE_LEN; i++)
	{
		if(line[i] != '\t')
			return line[i];
	}
	return 0;
}

uint insert_child_container_into_parent(Container_t child, Container_t *parent)
{
	Container_t *subcontainers;
	uint n_subcontainers;
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

	for (uint i = 0; i < n_subcontainers; i++)
	{
		if(subcontainers[i].container_type_data == NULL) // container_type_data being NULL should imply an empty slot.
		{
			subcontainers[i] = child;
			return i;
		}
	}
	
	ERROR("\nChild insertion error: cannot fit child into parent.")	
}

void parse_container_tree(char ui_file[N_LINES][MAX_LINE_LEN], uint i, Container_t *c_parent, 
						Container_tag_list_t *tag_list, Container_style_t style_list[MAX_STYLES]);

void parse_subcontainers(char ui_file[N_LINES][MAX_LINE_LEN], uint i, int parent_indentation, Container_t *c_parent,
						Container_tag_list_t *tag_list, Container_style_t style_list[MAX_STYLES])
{
	uint c_indentation;
	for (uint c_line = i + 1; c_line < (uint)N_LINES; c_line++)
	{
		c_indentation = count_indentations(ui_file, c_line);
		if (ui_file[c_line][c_indentation] == DECL_CONTAINER) // Container, not a comment etc.
		{
			if (c_indentation == (uint)parent_indentation + 1) // Subcontainer -> parse;
				parse_container_tree(ui_file, c_line, c_parent, tag_list, style_list);
			if (c_indentation > (uint)parent_indentation + 1) // Subsubcontainer -> skip
				continue;
			if (c_indentation <= (uint)parent_indentation) // Sibling container or higher -> no more subcontainers
				break;
		}
	}
}

void parse_container(char ui_file[N_LINES][MAX_LINE_LEN], uint i, Container_t *c_parent, 
						Container_tag_list_t *tag_list, Container_style_t style_list[MAX_STYLES])
{
	char tag[TAG_MAX_LEN];
	tag[0] = 0;
	bool open = true;
	Size_Type_e s_type = TILES;
	uint size = 0;
	Container_orientation_e orientation = VERTICAL;
	uint c_indentation = count_indentations(ui_file, i);
	uint n_subcontainers = 0;
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
			case PARAM_GROW:
				s_type = GROW;
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
	

	Container_t new_container = ascui_create_container(open, s_type, size, orientation, n_subcontainers);
	uint index = insert_child_container_into_parent(new_container, c_parent);
	Container_t *new_container_ptr = ascui_get_nth_subcontainer(*c_parent, index);
	
	if (tag[0] != 0)
		ascui_tag_list_add(tag_list, new_container_ptr, tag);

	if(n_subcontainers != 0)
		parse_subcontainers(ui_file, i, c_indentation, new_container_ptr, tag_list, style_list);
}

void parse_box(char ui_file[N_LINES][MAX_LINE_LEN], uint i, Container_t *c_parent, 
						Container_tag_list_t *tag_list, Container_style_t style_list[MAX_STYLES])
{
	char tag[TAG_MAX_LEN];
	tag[0] = 0;
	bool open = true;
	Size_Type_e s_type = TILES;
	uint size = 0;
	Container_orientation_e orientation = VERTICAL;
	uint c_indentation = count_indentations(ui_file, i);
	uint n_subcontainers = 0;
	n_subcontainers = count_subcontainers(ui_file, i, c_indentation);
	Container_style_t style = style_list[0]; // 0 as default

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
			case PARAM_GROW:
				s_type = GROW;
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
	

	Container_t new_container = ascui_create_box(open, s_type, size, orientation, n_subcontainers, style);
	uint index = insert_child_container_into_parent(new_container, c_parent);
	Container_t *new_container_ptr = ascui_get_nth_subcontainer(*c_parent, index);
	
	if (tag[0] != 0)
		ascui_tag_list_add(tag_list, new_container_ptr, tag);

	if(n_subcontainers != 0)
		parse_subcontainers(ui_file, i, c_indentation, new_container_ptr, tag_list, style_list);
}

void replace_underscores(char *str)
{
	uint i = 0;
	while(str[i] != 0)
	{
		if(str[i] == '_')
			str[i] = ' ';
		i++;
	}
}

void parse_text(char ui_file[N_LINES][MAX_LINE_LEN], uint i, Container_t *c_parent, 
						Container_tag_list_t *tag_list, Container_style_t style_list[MAX_STYLES])
{
	char tag[TAG_MAX_LEN];
	tag[0] = 0;
	char txt[MAX_LINE_LEN];
	txt[0] = 0;

	int txt_index = -1;
	
	bool open = true;
	Size_Type_e s_type = TILES;
	uint size = 0;
	Container_style_t style = style_list[0]; // 0 as default

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
			case PARAM_GROW:
				s_type = GROW;
				break;
			case FLAG_VERTICAL:
				WARNINGF("\nContainer-parsing error: text does not need orientation - at line %u.", i)
				break;
			case FLAG_HORIZONTAL:
				WARNINGF("\nContainer-parsing error: text does not need orientation - at line %u.", i)
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
	

	Container_t new_container;

	if(txt_index == -1) // Text from parameter
	 	new_container = ascui_create_text(open, s_type, size, strlen(txt), txt, style.char_col, style.bg_col);
	else 				// Text from texts[txt_index]
	 	new_container = ascui_create_text(open, s_type, size, strlen(texts[txt_index]), texts[txt_index], style.char_col, style.bg_col);

	uint index = insert_child_container_into_parent(new_container, c_parent);
	Container_t *new_container_ptr = ascui_get_nth_subcontainer(*c_parent, index);
	
	if (tag[0] != 0)
		ascui_tag_list_add(tag_list, new_container_ptr, tag);
}
void parse_container_tree(char ui_file[N_LINES][MAX_LINE_LEN], uint i, Container_t *c_parent, 
						Container_tag_list_t *tag_list, Container_style_t style_list[MAX_STYLES])
{
	uint c_indentation = count_indentations(ui_file, i);
	// 									V	start tokenization after tabs
	char *tok = strtok(&ui_file[i][c_indentation], DELIMITATORS); 

	char *type_str = &tok[2];
	
	if (strcmp(type_str, "box") == 0)
		parse_box(ui_file, i, c_parent, tag_list, style_list);
	else if (strcmp(type_str, "container") == 0)
		parse_container(ui_file, i, c_parent, tag_list, style_list);
	else if (strcmp(type_str, "button") == 0)
		ERROR("\nContainer-parsing error: button parsing not implemented")
	else if (strcmp(type_str, "text") == 0)
		parse_text(ui_file, i, c_parent, tag_list, style_list);
	else if (strcmp(type_str, "subgrid") == 0) 
		ERROR("\nContainer-parsing error: subgrid parsing not implemented")
	else ERRORF("\nContainer-parsing error: unknown container type at line %u.", i)
	
	
}


Container_t *ascui_construct_ui_from_file(char *ui_file_path, Container_tag_list_t *tag_list)
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
	uint line_num = 0;
	while (fgets(ui_file[line_num], MAX_LINE_LEN, file)) 
	{
		line_num++;
 	}
	fclose(file);

	Container_style_t style_list[MAX_STYLES];
	Container_style_t *style_list_ptr = &(style_list[0]);

	uint i;
	// Parse styles
	for (i = 0; i < (uint)N_LINES; i++)
	{
		if(ui_file[i][0] == DECL_STYLE) // Style declaration
			parse_style(ui_file[i], style_list_ptr);
		
		if(ui_file[i][0] == FLAG_HORIZONTAL || ui_file[i][0] == FLAG_VERTICAL) // Containers reached
			break;
	}

	assert(ui_file[i][0] == FLAG_HORIZONTAL || ui_file[i][0] == FLAG_VERTICAL);

	Container_orientation_e top_c_orientation = (ui_file[i][0] == FLAG_HORIZONTAL)? HORIZONTAL : VERTICAL;

	Container_t *top_container = calloc(1, sizeof(Container_t));
	*top_container = ascui_create_container(true, PERCENTAGE, 100, top_c_orientation, 
											count_subcontainers(ui_file, 0, -1));


	// Parse every top-level subtree 
	for (uint i = 0; i < (uint)N_LINES; i++)
		if(ui_file[i][0] == DECL_CONTAINER) // top-level Container
			parse_container_tree(ui_file, i, top_container, tag_list, style_list);
			
	return top_container;
}
