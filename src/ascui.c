#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include "common.h"
#include "ascui.h"
#include "raytiles.h"
#include "texts.h"

#define get_orientation(container) *(container_orientation_e *)(container->container_type_data)
#define INLINE_COLOR '\a'
#define INLINE_FONT '\b'

static container_t *create_container_stub(bool open, size_type_e s_type, uint8_t size, container_type_e c_type)
{
	container_t	*container = calloc(1, sizeof(container_t));
	container->open = open;
	container->scroll_offset = 0;
	container->size_type = s_type;
	container->size = size;
	container->container_type = c_type;

	return container;
}

container_t *ascui_container(bool open, size_type_e s_type, uint8_t size, container_orientation_e orientation, uint16_t n_subcontainers, ...)
{
	container_t *container = create_container_stub(open, s_type, size, CONTAINER);
	container->container_type_data = calloc(1, sizeof(container_data_t));
	((container_data_t *)container->container_type_data)->orientation = orientation;
	((container_data_t *)container->container_type_data)->n_subcontainers = n_subcontainers;
	((container_data_t *)container->container_type_data)->subcontainers = calloc(n_subcontainers, sizeof(container_t *));

	va_list subcontainers;
	va_start(subcontainers, n_subcontainers);
	for (uint16_t i = 0; i < n_subcontainers; i++) 
	{
        ((container_data_t *)container->container_type_data)->subcontainers[i] = va_arg(subcontainers, container_t *);
	}
	va_end(subcontainers);
	
	return container;
}

container_t *ascui_box(bool open, size_type_e s_type, uint8_t size, container_orientation_e orientation, container_style_t style, uint16_t n_subcontainers, ...)
{
	container_t *container = create_container_stub(open, s_type, size, BOX);
	container->container_type_data = calloc(1, sizeof(box_data_t));
	((box_data_t *)container->container_type_data)->orientation = orientation;
	((box_data_t *)container->container_type_data)->style = style;
	((box_data_t *)container->container_type_data)->n_subcontainers = n_subcontainers;
	((box_data_t *)container->container_type_data)->subcontainers = calloc(n_subcontainers, sizeof(container_t *));

	va_list subcontainers;
	va_start(subcontainers, n_subcontainers);
	for (uint16_t i = 0; i < n_subcontainers; i++) 
	{
        ((box_data_t *)container->container_type_data)->subcontainers[i] = va_arg(subcontainers, container_t *);
	}
	va_end(subcontainers);
	
	return container;
}

container_t *ascui_text(bool open, size_type_e s_type, uint8_t size, uint16_t text_len, char *text, container_style_t style)
{
	container_t *container = create_container_stub(open, s_type, size, TEXT);
	container->container_type_data = calloc(1, sizeof(text_data_t));
	((text_data_t *)container->container_type_data)->style = style;

	if(text != NULL)
	{
		((text_data_t *)container->container_type_data)->text = calloc(text_len, sizeof(char));
		strcpy(((text_data_t *)container->container_type_data)->text, text);
		((text_data_t *)container->container_type_data)->text_len = text_len;
	}

	return container;
}

container_t *ascui_subgrid(bool open, size_type_e s_type, uint8_t size, grid_t *subgrid)
{
	container_t *container = create_container_stub(open, s_type, size, SUBGRID);
	container->container_type_data = calloc(1, sizeof(subgrid_data_t));
	((subgrid_data_t *)container->container_type_data)->subgrid = subgrid;

	return container;
}

container_t *ascui_button(bool open, size_type_e s_type, uint8_t size, uint16_t text_len, char *text, container_style_t style, 
						  UI_side_effect_func side_effect_func, void *domain, void *function_data)
 {
 	container_t *container = create_container_stub(open, s_type, size, BUTTON);
	container->container_type_data = calloc(1, sizeof(button_data_t));
	((button_data_t *)container->container_type_data)->style = style;

	((button_data_t *)container->container_type_data)->text = calloc(text_len, sizeof(char));
	strcpy(((button_data_t *)container->container_type_data)->text, text);
	((button_data_t *)container->container_type_data)->text_len = text_len;
	
	((button_data_t *)container->container_type_data)->side_effect_func = side_effect_func;
	((button_data_t *)container->container_type_data)->domain = domain;
	((button_data_t *)container->container_type_data)->function_data = function_data;

	return container;
 }


container_data_t *ascui_get_container_data(container_t *container)
{
	return (container_data_t *)container->container_type_data;
}

box_data_t *ascui_get_box_data(container_t *container)
{
	return (box_data_t *)container->container_type_data;
}

text_data_t *ascui_get_text_data(container_t *container)
{
	return (text_data_t *)container->container_type_data;
}

subgrid_data_t *ascui_get_subgrid_data(container_t *container)
{
	return (subgrid_data_t *)container->container_type_data;
}

button_data_t *ascui_get_button_data(container_t *container)
{
	return (button_data_t *)container->container_type_data;
}

container_t *ascui_get_nth_subcontainer(container_t *container, uint16_t n)
{
	 container_type_e type = container->container_type;

	container_t **subcontainers;
	uint16_t n_subcontainers;
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
		ERROR("\nscui_get_nth_subcontainer: wrong container type")
		
	assert(n < n_subcontainers);

	return (subcontainers[n]);
}

// ?
void ascui_set_nth_subcontainer(container_t *container, uint16_t n, container_t *subcontainer)
{
	container_t **subcontainer_address = &ascui_get_container_data(container)->subcontainers[n];
	*subcontainer_address = subcontainer;
}


static void _print_ui(container_t *container, uint16_t indentation, bool last_child)
{
	int line_len = 0;
	putc('\n', stdout);
	for (uint16_t i = 0; i < indentation; i++)
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
	switch (container->container_type)
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

	uint16_t n_subcontainers;
	container_t **subcontainers;
	
	if (container->container_type == CONTAINER)
	{
		container_data_t *c_data = ascui_get_container_data(container);
		n_subcontainers = c_data->n_subcontainers;
		subcontainers = c_data->subcontainers;
	}
	else if (container->container_type == BOX)
	{
		box_data_t *b_data = ascui_get_box_data(container);
		n_subcontainers = b_data->n_subcontainers;
		subcontainers = b_data->subcontainers;
	}
	else return;
	
	for (uint16_t i = 0; i < n_subcontainers; i++)
		_print_ui(subcontainers[i], indentation + 1, (i == n_subcontainers - 1));
}

void ascui_print_ui(container_t *container)
{
	_print_ui(container, 0, false);
	putc('\n', stdout);
}

void ascui_destroy(container_t *container)
{
	container_type_e type = container->container_type;

	container_t **subcontainers;
	uint16_t n_subcontainers = 0;
	switch (type)
	{
		case CONTAINER:
		subcontainers = ascui_get_container_data(container)->subcontainers;
		n_subcontainers = ascui_get_container_data(container)->n_subcontainers;
		if(n_subcontainers != 0)
		{
			for (uint16_t i = 0; i < n_subcontainers; i++)
			{
				ascui_destroy(subcontainers[i]);
			}
		}
		break;
		case BOX:
		subcontainers = ascui_get_box_data(container)->subcontainers;
		n_subcontainers = ascui_get_box_data(container)->n_subcontainers;
		if(n_subcontainers != 0)
		{
			for (uint16_t i = 0; i < n_subcontainers; i++)
			{
				ascui_destroy(subcontainers[i]);
			}
		}
		break;
		case BUTTON:
		free(ascui_get_button_data(container)->text);
		break;
		case TEXT:
		free(ascui_get_button_data(container)->text);
		break;
		default:
		break;
	}
	free(container->container_type_data);
	free(container);
}

static bool check_cursor_hover(cursor_t *cursor, container_t *container, uint8_t x0, int y0, uint8_t x1, int y1)
{
	if(container->container_type == CONTAINER)
		return false;
		
	if(container->container_type == BOX)	
	{
		bool inside_box = cursor->y >= y0 && cursor->y <= y1 && cursor->x >= x0 && cursor->x <= x1;
		bool inside_child = cursor->y >= y0 + 1 && cursor->y <= y1 - 1 && cursor->x >= x0 + 1 && cursor->x <= x1 - 1;
		// bool y_match = (cursor->y == y0) || (cursor->y == y1);
		// bool x_match = (cursor->x == x0) || (cursor->x == x1);
		if(inside_box && !inside_child)
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

static void draw_box(grid_t *grid, container_style_t style, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool invert_cols)
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

static uint16_t ascui_draw_container(grid_t *grid, container_t *container, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
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

	uint8_t sub_x1;
	uint8_t sub_y1;

	uint16_t n_subcontainers;
	container_t *c_subcontainer;
	container_type_e c_type = container->container_type;
	// bool percentage_split = true;

	uint16_t tiles_drawn = 0;
	uint16_t max_scroll = 0;
	 	
	if (c_type == CONTAINER || c_type == BOX)
	{
		container_t **subcontainers;
		container_orientation_e orientation;
		container_style_t style;

		if (c_type == CONTAINER)
		{
			container_data_t *c_data = ascui_get_container_data(container);
			n_subcontainers = c_data->n_subcontainers;
			subcontainers = c_data->subcontainers;
			orientation = c_data->orientation;
			
		}
		else if (c_type == BOX)
		{
			box_data_t *b_data = ascui_get_box_data(container);
			n_subcontainers = b_data->n_subcontainers;
			subcontainers = b_data->subcontainers;
			orientation = b_data->orientation;
			style = b_data->style;
		}

		max_scroll = max((int)n_subcontainers - 1, 0);
		container->scroll_offset = umin(container->scroll_offset, max_scroll); // Clamp scroll

		// Account for box edges when delegating space for subcontainers
		if (c_type == BOX) {x0++; y0++; x1--; y1--;}

		for (uint16_t i = container->scroll_offset; i < n_subcontainers; i++)
		{
			c_subcontainer = subcontainers[i];
			// Edge case: Last subcontainer
			if (i == n_subcontainers - 1)
			{
				if (orientation == HORIZONTAL)
				{
					tiles_drawn += ascui_draw_container(grid, c_subcontainer, x0, y0 + tiles_drawn, x1, y1, orientation, cursor);
				}
				else if (orientation == VERTICAL)
				{
					tiles_drawn += ascui_draw_container(grid, c_subcontainer, x0 + tiles_drawn, y0, x1, y1, orientation, cursor);
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
		
		text_data_t *t_data = ascui_get_text_data(container);

		color8b_t bg_col;
		color8b_t smbl_col;
		if(hovered || selected)
		{
			bg_col = t_data->style.char_col;
			smbl_col = t_data->style.bg_col;
		}
		else
		{
			bg_col = t_data->style.bg_col;
			smbl_col = t_data->style.char_col;
		}
		
		uint16_t _x = 0;
		int _y = - container->scroll_offset; // = 0 at no scroll
		uint16_t total_text_len = 0;

		color8b_t inl_color = BLACK8B;
		bool inl_color_active = false;

		uint8_t font = t_data->style.font;
		bool inl_font_active = false;
		
		tl_draw_rect_bg(grid, x0, y0, x1, y1, bg_col);

		for (uint16_t i = 0; i < t_data->text_len; i++)
		{
			if (t_data->text[i] == '\0')
				break;
				
			if(t_data->text[i] == INLINE_FONT)
			{
				if(inl_font_active)
				{
					font = t_data->style.font; // Restore old font
					inl_font_active = false;
				}
				else
				{
					inl_font_active = true;
					font = atoi(&t_data->text[i + 1]);
					i += 3;
				}
				continue;
			}
			
			if (t_data->text[i] == INLINE_COLOR)
			{
				if(inl_color_active)
					inl_color_active = false;
				else
				{
					inl_color_active = true;
					inl_color = col8bt(t_data->text[i + 1] - 48, t_data->text[i + 2] - 48, t_data->text[i + 3] - 48);
					i += 3;
				}
				continue;
			}
			
			if (x0 + _x > x1)
				{total_text_len++;  _y++; _x = 0; }
			
			if (t_data->text[i] == '\n')
			{	
				_y++;
				total_text_len++;
				_x = 0;
				continue; 
			}

			
			if(_y >= 0 && !(y0 + max(0, _y) > y1)) // do not render if tiles have been "scrolled" out of view - or if the text overflows
			{
				if(inl_color_active)
					tl_plot_smbl(grid, x0 + _x, y0 + _y, t_data->text[i], inl_color, font);
				else
					tl_plot_smbl(grid, x0 + _x, y0 + _y, t_data->text[i], smbl_col, font);
			}
			_x++;
		}
			
		uint16_t max_scroll = max((int)total_text_len - (int)(y1-y0), 0);
		container->scroll_offset = umin(container->scroll_offset, max_scroll);

		if(hovered && cursor->scroll != 0)
			tl_plot_bg(grid, x0, y0 + (y1-y0) * ((float)container->scroll_offset / (float)max_scroll), BLACK8B);
			

		return (parent_orientation == VERTICAL)? x1-x0 + 1 : y1-y0 + 1;
	}
	else if(c_type == SUBGRID)
	{
		subgrid_data_t *subg_data = ascui_get_subgrid_data(container);

		if(!subg_data->subgrid)
			ERROR("\n UI drawing error: Subgrid not initialized!")
		
		if(cursor->hovered_container == container)
			subg_data->subgrid->tile_p_w -= cursor->scroll;		// "Suggest" new tile width
		tl_fit_subgrid(grid, subg_data->subgrid, x0, y0, x1, y1);	// new tile width sanitized by tl_fit_subgrid
	}
	else if(c_type == BUTTON)
	{
		hovered = check_cursor_hover(cursor, container, x0, y0, x1, y1);
		button_data_t *bt_data = ascui_get_button_data(container);

		color8b_t bg_col;
		color8b_t smbl_col;
		if(hovered || selected)
		{
			bg_col = bt_data->style.char_col;
			smbl_col = bt_data->style.bg_col;
		}
		else
		{
			bg_col = bt_data->style.bg_col;
			smbl_col = bt_data->style.char_col;
		}


		uint8_t horizontal_space = x1 - x0 + 1;
		uint8_t vertical_space = y1 - y0 + 1;
		uint8_t vertical_midpoint = vertical_space / 2;
		int  horizontal_start = ((int)horizontal_space - (int)bt_data->text_len) / 2;

		tl_draw_rect_bg(grid, x0, y0, x1, y1, bg_col);
		color8b_t inl_color = BLACK8B;
		bool inl_color_active = false;

		uint8_t font = bt_data->style.font;
		bool inl_font_active = false;

		if (horizontal_start >= 0)
		{
			uint i_offset = 0;
			for (uint16_t i = 0; i < bt_data->text_len; i++)
			{
				if(bt_data->text[i] == INLINE_FONT)
				{
					if (inl_font_active)
					{
						font = bt_data->style.font; // Restore old font
						inl_font_active = false;
					}
					else
					{
						inl_font_active = true;
						font = atoi(&bt_data->text[i + 1]);
						i += 3;
					}
					continue;
				}
				
				if (bt_data->text[i] == INLINE_COLOR)
				{
					if(inl_color_active)
						inl_color_active = false;
					else
					{
						inl_color_active = true;
						inl_color = col8bt(bt_data->text[i + 1] - 48, bt_data->text[i + 2] - 48, bt_data->text[i + 3] - 48);
						i += 3;
					}
					continue;
				}
				if(inl_color_active)
					tl_plot_smbl(grid, x0 + horizontal_start + i_offset, y0 + vertical_midpoint, bt_data->text[i], inl_color, font);
				else
					tl_plot_smbl(grid, x0 + horizontal_start + i_offset, y0 + vertical_midpoint, bt_data->text[i], smbl_col, font);
				i_offset++; // only increment if something was drawn
			}
		}
		else
		{
			uint8_t _x = 0;
			uint8_t _y = 0;

			
			for (uint16_t i = 0; i < bt_data->text_len; i++)
			{
				if (bt_data->text[i] == '\0')
					break;
				if(bt_data->text[i] == INLINE_FONT)
				{
					if(inl_font_active)
					{
						font = bt_data->style.font; // Restore old font
						inl_font_active = false;
					}
					else
					{
						inl_font_active = true;
						font = atoi(&bt_data->text[i + 1]);
						i += 3;
					}
					continue;
				}
				
				if (bt_data->text[i] == INLINE_COLOR)
				{
					if(inl_color_active)
						inl_color_active = false;
					else
					{
						inl_color_active = true;
						inl_color = col8bt(bt_data->text[i + 1] - 48, bt_data->text[i + 2] - 48, bt_data->text[i + 3] - 48);
						i += 3;
					}
					continue;
				}
			
				if (x0 + _x > x1)
					{_y++; _x = 0; }
					
				if(y0 + _y < y1)
					break;
				
				if (bt_data->text[i] == '\n')
				{	
					_y++;
					_x = 0;
					continue; 
				}
				
				if(inl_color_active)
					tl_plot_smbl(grid, x0 + _x, y0 + _y, bt_data->text[i], inl_color, font);
				else
					tl_plot_smbl(grid, x0 + _x, y0 + _y, bt_data->text[i], smbl_col, font);
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

	pos16_t grid_size = tl_grid_get_dimensions(grid);
	container_data_t *c_data = ascui_get_container_data(top_container);
	ascui_draw_container(grid, top_container, 0, 0, grid_size.x - 1, grid_size.y - 1, c_data->orientation, cursor);
}
