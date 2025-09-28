 #include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include "common.h"
#include "ascui.h"
#include "raytiles.h"

#define get_orientation(container) *(container_orientation_e *)(container->container_type_data)
#define INLINE_COLOR_CHANGE '\a'
#define INLINE_TEX_MAP_CHANGE '\b'
#define SCROLL_KNOB_CHAR 131

context_t *ascui_context(uint16_t binding_capacity, container_t *top_container)
{
	context_t *ctx = calloc(1, sizeof(context_t));
	ctx->top_container = top_container;
	ctx->binding_capacity = binding_capacity; 

	ctx->var_bindings = calloc(binding_capacity, sizeof(var_binding_t)); 

	// Mark all bindings as unused
	for (uint16_t i = 0; i < ctx->binding_capacity; i++)
		ctx->var_bindings[i].update_flag = -1;

	return ctx;
}

// Save the returned address for use in your program
var_binding_t *ascui_add_context_var_binding(context_t *ctx, void *ptr, parameter_type_e var_type)
{
	for (uint16_t i = 0; i < ctx->binding_capacity; i++)
		if(ctx->var_bindings[i].update_flag == -1)
		{
			ctx->var_bindings[i].var = ptr;
			ctx->var_bindings[i].update_flag = 2;
			ctx->var_bindings[i].var_type = var_type;
			return &ctx->var_bindings[i];
		}

	ERROR("\nascui_add_context_var_binding: capacity already reached!")
}

// Supply a saved address to remove
void ascui_retire_var_binding(var_binding_t *var_binding)
{
	var_binding->var = 0;
	var_binding->update_flag = -1;
}

void flag_bound_var(var_binding_t *binding)
{
	binding->update_flag = 2;
}

static container_t *create_container_stub(bool open, uint8_t selectability, size_type_e s_type, uint8_t size, container_type_e c_type)
{
	container_t	*container = calloc(1, sizeof(container_t));
	container->open = open;
	container->selectability = selectability;
	container->scroll_offset = 0;
	container->size_type = s_type;
	container->size = size;
	container->container_type = c_type;

	return container;
}

container_data_t *ascui_get_container_data(container_t *container) { return (container_data_t *)container->container_type_data; }

box_data_t *ascui_get_box_data(container_t *container) { return (box_data_t *)container->container_type_data; }

text_data_t *ascui_get_text_data(container_t *container) { return (text_data_t *)container->container_type_data; }

subgrid_data_t *ascui_get_subgrid_data(container_t *container) { return (subgrid_data_t *)container->container_type_data; }

button_data_t *ascui_get_button_data(container_t *container) { return (button_data_t *)container->container_type_data; }

input_data_t *ascui_get_input_data(container_t *container) { return (input_data_t *)container->container_type_data; }

slider_data_t *ascui_get_slider_data(container_t *container) { return (slider_data_t *)container->container_type_data; }

toggle_data_t *ascui_get_toggle_data(container_t *container) { return (toggle_data_t *)container->container_type_data; }

display_data_t *ascui_get_display_data(container_t *container) { return (display_data_t *)container->container_type_data; }

divider_data_t *ascui_get_divider_data(container_t *container) { return (divider_data_t *)container->container_type_data; }

/// BASE TYPES

// NOTE: since container_data_t matches box_data_t, we can assume it is a container_data_t
static void parse_subcontainers(uint16_t n_subcontainers, container_t **subcontainers, va_list incoming_subcontainers)
{
	for (uint16_t i = 0; i < n_subcontainers; i++) 
	{
        subcontainers[i] = va_arg(incoming_subcontainers, container_t *);
	}

	// Check for parameter substitutions
	container_t *c_subcontainer;
	for (uint16_t i = 0; i < n_subcontainers; i++) 
	{
		c_subcontainer = subcontainers[i];
		if(c_subcontainer->container_type == BUTTON && ascui_get_button_data(c_subcontainer)->param_substitution)
		{
			if(ascui_get_button_data(c_subcontainer)->domain == SUBST_NEXT_CNTR)
				ascui_get_button_data(c_subcontainer)->domain = subcontainers[i + 1];
			else if(ascui_get_button_data(c_subcontainer)->domain == SUBST_OWN_TEXT)
				ascui_get_button_data(c_subcontainer)->domain = ascui_get_button_data(c_subcontainer)->text;
				
			if(ascui_get_button_data(c_subcontainer)->function_data == SUBST_NEXT_CNTR)
				ascui_get_button_data(c_subcontainer)->function_data = subcontainers[i + 1];
			else if(ascui_get_button_data(c_subcontainer)->function_data == SUBST_OWN_TEXT)
				ascui_get_button_data(c_subcontainer)->function_data = ascui_get_button_data(c_subcontainer)->text;

		}
	}
	
} 

container_t *ascui_container(bool open, size_type_e s_type, uint8_t size, container_orientation_e orientation, uint16_t n_subcontainers, ...)
{
	container_t *container = create_container_stub(open, STATIC, s_type, size, CONTAINER);
	container->container_type_data = calloc(1, sizeof(container_data_t));
	((container_data_t *)container->container_type_data)->orientation = orientation;
	((container_data_t *)container->container_type_data)->n_subcontainers = n_subcontainers;
	((container_data_t *)container->container_type_data)->subcontainers = calloc(n_subcontainers, sizeof(container_t *));

	va_list incoming_subcontainers;
	va_start(incoming_subcontainers, n_subcontainers);

	parse_subcontainers(n_subcontainers, ((container_data_t *)container->container_type_data)->subcontainers, incoming_subcontainers);
	
	va_end(incoming_subcontainers);
	
	return container;
}

container_t *ascui_box(bool open, uint8_t selectability, size_type_e s_type, uint8_t size, container_orientation_e orientation, container_style_t style, uint16_t n_subcontainers, ...)
{
	container_t *container = create_container_stub(open, selectability, s_type, size, BOX);
	container->container_type_data = calloc(1, sizeof(box_data_t));
	((box_data_t *)container->container_type_data)->orientation = orientation;
	((box_data_t *)container->container_type_data)->style = style;
	((box_data_t *)container->container_type_data)->n_subcontainers = n_subcontainers;
	((box_data_t *)container->container_type_data)->subcontainers = calloc(n_subcontainers, sizeof(container_t *));

	va_list incoming_subcontainers;
	va_start(incoming_subcontainers, n_subcontainers);

	parse_subcontainers(n_subcontainers, ((box_data_t *)container->container_type_data)->subcontainers, incoming_subcontainers);
	
	va_end(incoming_subcontainers);
	
	return container;
}

// NOTE: does not support parameter substitutions
container_t *ascui_empty_container(bool open, size_type_e s_type, uint8_t size, container_orientation_e orientation, uint16_t n_subcontainers)
{
	container_t *container = create_container_stub(open, STATIC, s_type, size, CONTAINER);
	container->container_type_data = calloc(1, sizeof(container_data_t));
	((container_data_t *)container->container_type_data)->orientation = orientation;
	((container_data_t *)container->container_type_data)->n_subcontainers = n_subcontainers;
	((container_data_t *)container->container_type_data)->subcontainers = calloc(n_subcontainers, sizeof(container_t *));

	return container;
}

// NOTE: does not support parameter substitutions
container_t *ascui_empty_box(bool open, uint8_t selectability, size_type_e s_type, uint8_t size, container_orientation_e orientation, container_style_t style, uint16_t n_subcontainers)
{
	container_t *container = create_container_stub(open, selectability, s_type, size, BOX);
	container->container_type_data = calloc(1, sizeof(box_data_t));
	((box_data_t *)container->container_type_data)->orientation = orientation;
	((box_data_t *)container->container_type_data)->style = style;
	((box_data_t *)container->container_type_data)->n_subcontainers = n_subcontainers;
	((box_data_t *)container->container_type_data)->subcontainers = calloc(n_subcontainers, sizeof(container_t *));

	return container;
}

container_t *ascui_text(bool open, uint8_t selectability, size_type_e s_type, uint8_t size, str_t *text, uint8_t h_align, uint8_t v_align, container_style_t style)
{
	container_t *container = create_container_stub(open, selectability, s_type, size, TEXT);
	container->container_type_data = calloc(1, sizeof(text_data_t));
	ascui_get_text_data(container)->text = text;
	ascui_get_text_data(container)->style = style;
	ascui_get_text_data(container)->baked_available_width = 0;
	ascui_get_text_data(container)->baked_line_widths = (ui8_list_t)new_list(uint8_t, 5, UNORDERED, DUPLICATES_ALLOWED);
	ascui_get_text_data(container)->h_alignment = h_align;
	ascui_get_text_data(container)->v_alignment = v_align;

	return container;
}

container_t *ascui_subgrid(bool open, size_type_e s_type, uint8_t size, grid_t *subgrid)
{
	container_t *container = create_container_stub(open, HOVERABLE, s_type, size, SUBGRID);
	container->container_type_data = calloc(1, sizeof(subgrid_data_t));
	((subgrid_data_t *)container->container_type_data)->subgrid = subgrid;

	return container;
}

container_t *_ascui_button(bool substitution, bool open, uint8_t selectability, size_type_e s_type, uint8_t size, str_t *text, uint8_t h_align, uint8_t v_align, container_style_t style, 
						  UI_side_effect_func side_effect_func, void *domain, void *function_data)
{
 	container_t *container = create_container_stub(open, selectability, s_type, size, BUTTON);
	container->container_type_data = calloc(1, sizeof(button_data_t));
	ascui_get_button_data(container)->style = style;
	ascui_get_button_data(container)->text = text;
	ascui_get_button_data(container)->baked_available_width = 0;
	ascui_get_button_data(container)->baked_line_widths = (ui8_list_t)new_list(uint8_t, 5, UNORDERED, DUPLICATES_ALLOWED);
	ascui_get_button_data(container)->h_alignment = h_align;
	ascui_get_button_data(container)->v_alignment = v_align;
	
	ascui_get_button_data(container)->param_substitution = substitution;
	ascui_get_button_data(container)->side_effect_func = side_effect_func;
	ascui_get_button_data(container)->domain = domain;
	ascui_get_button_data(container)->function_data = function_data;

	return container;
}

container_t *ascui_button(bool open, uint8_t selectability, size_type_e s_type, uint8_t size, str_t *text, uint8_t h_align, uint8_t v_align, container_style_t style, 
						  UI_side_effect_func side_effect_func, void *domain, void *function_data)
{
	return _ascui_button(false, open, selectability, s_type, size, text, h_align, v_align, style, side_effect_func, domain, function_data);
}

container_t *ascui_button_subst(bool open, uint8_t selectability, size_type_e s_type, uint8_t size, str_t *text, uint8_t h_align, uint8_t v_align, container_style_t style, 
						  UI_side_effect_func side_effect_func, void *domain, void *function_data)
{
	return _ascui_button(true, open, selectability, s_type, size, text, h_align, v_align, style, side_effect_func, domain, function_data);
}
						  
container_t *ascui_input(bool open, size_type_e s_type, uint8_t size, container_style_t style, 
						  int32_t min, int32_t max, var_binding_t *var_binding)
{
 	container_t *container = create_container_stub(open, SELECTABLE, s_type, size, INPUT);
	container->container_type_data = calloc(1, sizeof(input_data_t));
	ascui_get_input_data(container)->style = style;

	ascui_get_input_data(container)->min = min;
	ascui_get_input_data(container)->max = max;
	ascui_get_input_data(container)->var_binding = var_binding;

	return container;
}

container_t *ascui_slider(bool open, size_type_e s_type, uint8_t size, container_style_t style, 
						  int32_t min, int32_t max, var_binding_t *var_binding)
{
 	container_t *container = create_container_stub(open, HOVERABLE, s_type, size, SLIDER);
	container->container_type_data = calloc(1, sizeof(slider_data_t));
	ascui_get_slider_data(container)->style = style;
	if(var_binding->var_type == STRING)
		ERROR("\nascui_slider: input_type = STRING is illegal!")

	ascui_get_slider_data(container)->min = min;
	ascui_get_slider_data(container)->max = max;
	ascui_get_slider_data(container)->var_binding = var_binding;

	return container;
}

container_t *ascui_toggle(bool *var, container_style_t style_on, container_style_t style_off)
{
 	container_t *container = create_container_stub(true, HOVERABLE, TILES, 3, TOGGLE);
	container->container_type_data = calloc(1, sizeof(toggle_data_t));
	ascui_get_toggle_data(container)->style_on = style_on;
	ascui_get_toggle_data(container)->style_off = style_off;
	ascui_get_toggle_data(container)->var = var;

	return container;
}

container_t *ascui_display(bool open, uint8_t selectability, size_type_e s_type, uint8_t size, str_t *fmt, var_binding_t *var_binding, 
							uint8_t h_align, uint8_t v_align, container_style_t style)
{
	container_t *container = create_container_stub(open, selectability, s_type, size, DISPLAY);
	container->container_type_data = calloc(1, sizeof(display_data_t));
	ascui_get_display_data(container)->style = style;
	ascui_get_display_data(container)->fmt = fmt;
	ascui_get_display_data(container)->var_binding = var_binding;
	ascui_get_display_data(container)->baked_available_width = 0;
	ascui_get_display_data(container)->baked_line_widths = (ui8_list_t)new_list(uint8_t, 5, UNORDERED, DUPLICATES_ALLOWED);
	ascui_get_display_data(container)->h_alignment = h_align;
	ascui_get_display_data(container)->v_alignment = v_align;

	ascui_get_display_data(container)->display_text = str_empty(12);
	
	return container;
}

container_t *ascui_divider(container_style_t style)
{
	container_t *container = create_container_stub(true, STATIC, TILES, 1, DIVIDER);
	container->container_type_data = calloc(1, sizeof(divider_data_t));
	ascui_get_divider_data(container)->style = style;

	return container;
}
/// COMPOSITES

container_t *ascui_input_w_desc(bool open, size_type_e txt_s_type, uint8_t txt_size, str_t *text, uint8_t h_align, uint8_t v_align,
								size_type_e input_s_type, uint8_t input_size, container_style_t style, 
						  		int32_t min, int32_t max, var_binding_t *var_binding)
{
	container_t *input_field_carrier = ascui_container(open, input_s_type, input_size, VERTICAL, 2,
		ascui_text(true, STATIC, txt_s_type, txt_size, text, h_align, v_align, style),
		ascui_input(true, TILES, 1, style, min, max, var_binding)
	);

	return input_field_carrier;
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
		ERROR("\nascui_get_nth_subcontainer: wrong container type")
		
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
		case CONTAINER: printf("[CONTAINER]"); 	line_len += strlen("[CONTAINER]"); 	break;
		case BOX: 		printf("[BOX]"); 		line_len += strlen("[BOX]"); 		break;
		case TEXT: 		printf("[TEXT]"); 		line_len += strlen("[TEXT]"); 		break;
		case SUBGRID: 	printf("[SUBGRID]"); 	line_len += strlen("[SUBGRID]"); 	break;
		case BUTTON:	printf("[BUTTON]");		line_len += strlen("[BUTTON]"); 	break;
		case INPUT: 	printf("[INPUT]");		line_len += strlen("[INPUT]"); 		break;
		case SLIDER: 	printf("[SLIDER]");		line_len += strlen("[SLIDER]"); 	break;
		case TOGGLE: 	printf("[TOGGLE]"); 	line_len += strlen("[TOGGLE]"); 	break;
		case DISPLAY: 	printf("[DISPLAY]");	line_len += strlen("[DISPLAY]"); 	break;
		case DIVIDER: 	printf("[DIVIDER]"); 	line_len += strlen("[DIVIDER]");	break;
		default: 		printf("[?]"); 			line_len += strlen("[?]"); 			break;
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
				free(subcontainers);
			}break;
		case BOX:
			subcontainers = ascui_get_box_data(container)->subcontainers;
			n_subcontainers = ascui_get_box_data(container)->n_subcontainers;
			if(n_subcontainers != 0)
			{
				for (uint16_t i = 0; i < n_subcontainers; i++)
				{
					ascui_destroy(subcontainers[i]);
				}
				free(subcontainers);
			}break;
		case BUTTON: 
			free(ascui_get_button_data(container)->text); 					
			free_list(&ascui_get_button_data(container)->baked_line_widths); 	
			break;
		case TEXT: 
			free(ascui_get_text_data(container)->text); 						
			free_list(&ascui_get_text_data(container)->baked_line_widths); 	
			break;					
		case DISPLAY: 
			free(ascui_get_display_data(container)->display_text); 						
			free(ascui_get_display_data(container)->fmt); 						
			free_list(&ascui_get_display_data(container)->baked_line_widths); 						
			break;
		case SUBGRID: tl_deinit_grid(ascui_get_subgrid_data(container)->subgrid); 	break;
		default:break;
	}
	free(container->container_type_data);
	free(container);
}

static bool check_cursor_hover(cursor_t *cursor, container_t *container, uint8_t x0, int y0, uint8_t x1, int y1)
{
	if(container->selectability == STATIC)
		return false;
		
	if(container->container_type == BOX)	
	{
		bool inside_box = cursor->y >= y0 && cursor->y <= y1 && cursor->x >= x0 && cursor->x <= x1;
		bool inside_child = cursor->y >= y0 + 1 && cursor->y <= y1 - 1 && cursor->x >= x0 + 1 && cursor->x <= x1 - 1;

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

static void calc_line_widths(ui8_list_t *line_widths, str_t *text, uint8_t available_width)
{
	clear_list(line_widths);
	
	uint8_t c_width = 0;
	bool inline_col = false;
	bool inl_tex_map_change = false;

	// printf("\n\n%u -> %u", text_len, available_width);
	for(uint16_t i = 0; i < text->length; i++)
	{	
		if(str_get(text, i) == INLINE_TEX_MAP_CHANGE)
		{
			if(inl_tex_map_change)
				{ inl_tex_map_change = false; i++; }
			else
				{ inl_tex_map_change = true; i += 3; }
			continue; 
		}
		
		if (str_get(text, i) == INLINE_COLOR_CHANGE)
		{
			if(inline_col)
				{ inline_col = false; i++; }
			else
				{ inline_col = true; i += 3; }
			continue;
		}
		
		if(str_get(text, i) == '\n')
		{
			ui8_list_add(line_widths, c_width);
			c_width = 0;
			continue;
		}

		if(str_get(text, i) == '\0')
		{
			if(c_width)
				ui8_list_add(line_widths, c_width);
			return;
		}

		// -> text[i] has width
		c_width++;

		if(c_width == available_width)
		{
			ui8_list_add(line_widths, available_width);
			c_width = 0;
			continue;
		}

	}

	if(c_width)
		ui8_list_add(line_widths, c_width);
}

static void draw_text(grid_t *grid, str_t *text, uint8_t h_align, uint8_t v_align, container_style_t style, 
						uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool invert_cols, ui8_list_t *line_widths, 
						uint8_t *baked_available_width, uint16_t *scroll_offset, bool scrolling)
{
	// Invertion
	color8b_t bg_col;
	color8b_t smbl_col;
	if(invert_cols)
	{
		bg_col = style.char_col;
		smbl_col = style.bg_col;
	}
	else
	{
		bg_col = style.bg_col;
		smbl_col = style.char_col;
	}

	tl_draw_rect_bg(grid, x0, y0, x1, y1, bg_col);

	if(!text)
		return;
	if(text->length == 1)
		return;

	// Rebake line widths if needed
	uint8_t available_width = x1 - x0 + 1;
	if (*baked_available_width != x1 - x0 + 1) // change in available line width -> rebake
	{
		*baked_available_width = x1 - x0 + 1;
		calc_line_widths(line_widths, text, available_width);
	}

	// Set max-scroll
	uint8_t max_scroll = max(line_widths->count - 1 - (y1 - y0), 0);
	*scroll_offset = min(*scroll_offset, max_scroll); // line_widths->count ~ max scroll

	uint8_t c_line = 0;
	uint8_t horizontal_start;
	uint8_t vertical_start;

	// Calc vertical start
	switch (v_align)
	{
		case ALIGN_TOP: 	vertical_start = 0;																					break;
		case ALIGN_MIDDLE:	vertical_start = max((int)(y1 - y0 + 1) - (int)line_widths->count, 0) / 2;							break;
		case ALIGN_BOTTOM: 	vertical_start = max((int)(y1 - y0 + 1) - (int)line_widths->count, 0);								break;
		default:			vertical_start = 0; WARNINGF("ascui: Warning - invalid V alignment in text: %s", str_charr(text))		break;
	}

	
	int16_t _y = -(*scroll_offset) + vertical_start;
	uint8_t _x = 0;
	bool inl_font_active = false;
	bool inl_color_active = false;
	uint8_t tex_map = style.tex_map;
	color8b_t inl_color;

	// Set initial line width
	switch (h_align)
	{
		case ALIGN_LEFT: 	horizontal_start = 0;																				break;
		case ALIGN_MIDDLE: 	horizontal_start = max((int)(available_width) - (int)ui8_list_get(*line_widths, 0), 0) / 2;			break;
		case ALIGN_RIGHT: 	horizontal_start = max((int)(available_width) - (int)ui8_list_get(*line_widths, 0), 0);				break;
		default:			horizontal_start = 0; WARNINGF("ascui: Warning - invalid H alignment in text: %s", str_charr(text))	break;
	}

	char c;
	
	for(uint16_t i = 0; i < text->length; i++)
	{
		c = str_get(text, i);

		if(c == 0)
			break;
		
		if(c == INLINE_TEX_MAP_CHANGE)
		{
			if(inl_font_active)
			{
				tex_map = style.tex_map; // Restore old font
				inl_font_active = false;
			}
			else
			{
				inl_font_active = true;
				tex_map = atoi(&str_charr(text)[i + 1]);
				i += 3;
			}
			continue;
		}

		if (c == INLINE_COLOR_CHANGE)
		{
			if(inl_color_active)
				inl_color_active = false;
			else
			{
				inl_color_active = true;
				inl_color = col8bt(str_get(text, i + 1) - 48, str_get(text, i + 2) - 48, str_get(text, i + 3) - 48);
				i += 3;
			}
			continue;
		}

		if (c == '\n' || _x + horizontal_start >= available_width) // new line if overflow or \n
		{	
			_y++;
			_x = 0;
			c_line++;
			// New line -> new line width
			if(c_line < line_widths->count)
			{
				
				switch (h_align)
				{
					case ALIGN_LEFT: 	horizontal_start = 0;																					break;
					case ALIGN_MIDDLE: 	horizontal_start = max((int)(available_width) - (int)ui8_list_get(*line_widths, c_line), 0) / 2;		break;
					case ALIGN_RIGHT: 	horizontal_start = max((int)(available_width) - (int)ui8_list_get(*line_widths, c_line), 0);			break;
					default:			horizontal_start = 0; WARNINGF("ascui: Warning - invalid H alignment in text: %s", str_charr(text)) 		break;
				}

			}
			
			if(c == '\n') // skip \n
				continue; 
		}

		if(_y >= 0 && !(y0 + max(0, _y) > y1)) // do not render if tiles have been "scrolled" out of view - or if the text overflows
		{
			if(inl_color_active)
				tl_plot_smbl(grid, x0 + _x + horizontal_start, y0 + _y, c, inl_color, tex_map);
			else
				tl_plot_smbl(grid, x0 + _x + horizontal_start, y0 + _y, c, smbl_col, tex_map);
		}
		_x++;
	}
	if(scrolling && max_scroll != 0 && invert_cols)
		tl_plot_smbl_w_bg(grid, x0, y0 + (y1 - y0) * ((float)(*scroll_offset) / max_scroll), 
							SCROLL_KNOB_CHAR, style.char_col, style.bg_col, style.tex_map);
}

static void draw_box(grid_t *grid, container_style_t style, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool invert_cols, 
					uint16_t scroll, uint16_t max_scroll, container_orientation_e orientation)
{
	if (invert_cols)
	{
		color8b_t tmp = style.char_col;
		style.char_col = style.border_col;
		style.border_col = tmp;
	}

	tl_draw_line_smbl_w_bg(grid, x0 + 1, y0, x1 - 1, y0, style.border_h_symbol, style.char_col, style.border_col, style.tex_map); // Top
	tl_draw_line_smbl_w_bg(grid, x0 + 1, y1, x1 - 1, y1, style.border_h_symbol, style.char_col, style.border_col, style.tex_map); // Bottom
	tl_draw_line_smbl_w_bg(grid, x0, y0 + 1, x0, y1 - 1, style.border_v_symbol, style.char_col, style.border_col, style.tex_map); // Left
	tl_draw_line_smbl_w_bg(grid, x1, y0 + 1, x1, y1 - 1, style.border_v_symbol, style.char_col, style.border_col, style.tex_map); // Right

	tl_plot_smbl_w_bg(grid, x0, y0, style.corner_symbol, style.char_col, style.border_col, style.tex_map);
	tl_plot_smbl_w_bg(grid, x0, y1, style.corner_symbol, style.char_col, style.border_col, style.tex_map);
	tl_plot_smbl_w_bg(grid, x1, y0, style.corner_symbol, style.char_col, style.border_col, style.tex_map);
	tl_plot_smbl_w_bg(grid, x1, y1, style.corner_symbol, style.char_col, style.border_col, style.tex_map);

	if(scroll != 0 && max_scroll != 0 && invert_cols)
	{
		if(orientation == VERTICAL)
			tl_plot_smbl_w_bg(grid, x0 + (x1 - x0) * ((float)scroll / max_scroll), y0, SCROLL_KNOB_CHAR, style.border_col, style.bg_col, style.tex_map);
		else if(orientation == HORIZONTAL)
			tl_plot_smbl_w_bg(grid, x0, y0 + (y1 - y0) * ((float)scroll / max_scroll), SCROLL_KNOB_CHAR, style.border_col, style.bg_col, style.tex_map);
	}
}

static uint16_t ascui_draw_container(grid_t *grid, container_t *container, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
								 container_orientation_e parent_orientation, cursor_t *cursor)
{
	if (!container->open)
		return 0;

	if (x1 < x0 || y1 < y0)
		return 0;

	bool hovered = check_cursor_hover(cursor, container, x0, y0 , x1, y1);
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

		for (uint16_t i = 0; container->scroll_offset + i < n_subcontainers; i++)
		{
			if(i == 0 && subcontainers[container->scroll_offset + i]->container_type == DIVIDER)
				continue; // Skip if first rendered container is a divider
		
			c_subcontainer = subcontainers[container->scroll_offset + i];
			// Edge case: Last subcontainer
			if (i == n_subcontainers - 1)
			{
				if (orientation == HORIZONTAL)
					tiles_drawn += ascui_draw_container(grid, c_subcontainer, x0, y0 + tiles_drawn, x1, y1, orientation, cursor);
				else if (orientation == VERTICAL)
					tiles_drawn += ascui_draw_container(grid, c_subcontainer, x0 + tiles_drawn, y0, x1, y1, orientation, cursor);

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
			
			draw_box(grid, style, x0, y0, x1, y1, hovered || selected, container->scroll_offset, max_scroll, orientation);
		}

			
		

		if (parent_orientation == HORIZONTAL)
			tiles_drawn = y1-y0 + 1;
		else if (parent_orientation == VERTICAL)
			tiles_drawn = x1-x0 + 1;

		return tiles_drawn;
	}
	else if(c_type == TEXT)
	{
		text_data_t *t_data = ascui_get_text_data(container);

		draw_text(grid, t_data->text, t_data->h_alignment, t_data->v_alignment, t_data->style, 
								x0, y0, x1, y1, selected || hovered, &t_data->baked_line_widths, 
								&t_data->baked_available_width, &container->scroll_offset, cursor->scroll);

		if(cursor->scroll && max_scroll != 0)
			tl_plot_smbl_w_bg(grid, x0, y0 + (y1 - y0) * (container->scroll_offset / max_scroll), 
								'#', t_data->style.bg_col, t_data->style.char_col, t_data->style.tex_map);

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
		button_data_t *bt_data = ascui_get_button_data(container);

		draw_text(grid, bt_data->text, bt_data->h_alignment, bt_data->v_alignment, bt_data->style, 
								x0, y0, x1, y1, selected || hovered, &bt_data->baked_line_widths, 
								&bt_data->baked_available_width, &container->scroll_offset, cursor->scroll);

		return (parent_orientation == VERTICAL)? x1-x0 + 1 : y1-y0 + 1;
	}
	else if(container->container_type == INPUT)
	{
		input_data_t *input_data = ascui_get_input_data(container);

		color8b_t bg_col;
		color8b_t smbl_col;
		uint8_t vertical_midpoint = (y1 - y0) / 2;
		if(hovered || selected)
		{
			bg_col = input_data->style.char_col;
			smbl_col = input_data->style.bg_col;
		}
		else
		{
			bg_col = input_data->style.bg_col;
			smbl_col = input_data->style.char_col;
		}

		tl_draw_rect_bg(grid, x0, y0, x1, y1, bg_col);

		if(selected)
		{
			uint8_t t = 2 * GetTime();
			t = t & 1; 
			char c = GetCharPressed();
			int k = GetKeyPressed();

			if (k == KEY_ENTER)
			{
				if(input_data->var_binding->var_type == STRING)
					input_data->buf[min(input_data->max, input_data->buf_i)] = '\0'; // Cut off string at max, or input length
				else
					input_data->buf[input_data->buf_i] = '\0'; // Cut off at end of input

				char *end_ptr = &input_data->buf[input_data->buf_i];
					
				switch (input_data->var_binding->var_type)
				{
					case U32_INT:  	*(uint32_t *)(input_data->var_binding->var) = clamp(input_data->min, atol(input_data->buf), input_data->max); 			break;			
					case U16_INT:  	*(uint16_t *)(input_data->var_binding->var) = clamp(input_data->min, atoi(input_data->buf), input_data->max); 			break;
					case U8_INT: 	*(uint8_t *)(input_data->var_binding->var) = clamp(input_data->min, atoi(input_data->buf), input_data->max); 			break;
					case S32_INT:  	*(int32_t *)(input_data->var_binding->var) = clamp(input_data->min, atol(input_data->buf), input_data->max); 			break;
					case S16_INT:  	*(int16_t *)(input_data->var_binding->var) = clamp(input_data->min, atoi(input_data->buf), input_data->max);			break;	
					case S8_INT: 	*(int8_t *)(input_data->var_binding->var) = clamp(input_data->min, atoi(input_data->buf), input_data->max); 			break;	
					case FLOAT:		*(float *)(input_data->var_binding->var) = flclamp(input_data->min, strtof(input_data->buf, &end_ptr), input_data->max);break;			
					case STRING: 	str_write_from_buf((str_t **)(&input_data->var_binding->var), input_data->buf, INPUT_BUF_MAX_LEN); 						break;
					default:		 																														break;
				}
				flag_bound_var(input_data->var_binding);
				cursor->selected_container = NULL; // Deselect after ENTER
			}
			else if(k == KEY_BACKSPACE && input_data->buf_i > 0)
				input_data->buf_i--;
			else if(c && input_data->buf_i < INPUT_BUF_MAX_LEN)
			{
				input_data->buf[input_data->buf_i] = c; 
				input_data->buf_i++;
			}
			
			uint16_t i = 0;
			tl_plot_smbl(grid, x0, y0 + vertical_midpoint, '>', smbl_col, input_data->style.tex_map);
			while (input_data->buf[i] != '\0' && i < x1 - x0 && i < input_data->buf_i)
			{
				tl_plot_smbl(grid, x0 + i + 1, y0 + vertical_midpoint, input_data->buf[i], smbl_col, input_data->style.tex_map);
				i++;
			}
			// Cursor bar
			if(t)
				tl_plot_smbl(grid, x0 + input_data->buf_i + 1, y0 + vertical_midpoint, '|', smbl_col, input_data->style.tex_map);
		}
		else	// NOT SELECTED
		{
			if(input_data->var_binding->update_flag)
			{
				input_data->buf[INPUT_BUF_MAX_LEN] = 0;	// Setup buffer overflow detection
				switch (input_data->var_binding->var_type)
				{
					case U32_INT: 	sprintf(input_data->buf, "%u", *(uint32_t *)input_data->var_binding->var); 			break;
					case U16_INT: 	sprintf(input_data->buf, "%u", *(uint16_t *)input_data->var_binding->var); 			break;
					case U8_INT: 	sprintf(input_data->buf, "%u", *(uint8_t *)input_data->var_binding->var); 			break;
					case S32_INT: 	sprintf(input_data->buf, "%d", *(int32_t *)input_data->var_binding->var); 			break;
					case S16_INT: 	sprintf(input_data->buf, "%d", *(int16_t *)input_data->var_binding->var);			break;
					case S8_INT: 	sprintf(input_data->buf, "%d", *(int8_t *)input_data->var_binding->var); 			break;
					case STRING: 	sprintf(input_data->buf, "%s", str_charr((str_t *)input_data->var_binding->var)); 	break;
					case FLOAT: 	sprintf(input_data->buf, "%f", *(float *)input_data->var_binding->var); 			break;
					default: 		sprintf(input_data->buf, "???"); 													break;
				}
				if(input_data->buf[INPUT_BUF_MAX_LEN] != 0)	// Check buffer overflow
					ERRORF("ascui: input buffer overflow, buf='%s'", input_data->buf)
					
				input_data->buf_i = min(strlen(input_data->buf), INPUT_BUF_MAX_LEN);
			}

			uint16_t i = 0;
			while (input_data->buf[i] != '\0' && i < x1 - x0 && i < INPUT_BUF_MAX_LEN)
			{
				tl_plot_smbl(grid, x0 + i, y0 + vertical_midpoint, input_data->buf[i], smbl_col, input_data->style.tex_map);
				i++;
			}
		}
		
		return (parent_orientation == VERTICAL)? x1-x0 + 1 : y1-y0 + 1;
	}
	else if(c_type == TOGGLE)
	{
		toggle_data_t *tg_data = ascui_get_toggle_data(container);

		if(hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			*tg_data->var = !(*tg_data->var);
		
		container_style_t style = (*tg_data->var)? tg_data->style_on : tg_data->style_off;
		
		draw_box(grid, style, x0, y0, x0 + 2, y0 + 2, hovered, 0, 0, 0);

		if(*tg_data->var)
			tl_plot_smbl(grid, x0 + 1, y0 + 1, '#', style.char_col, style.tex_map);
		
		return 3;
	}
	else if(c_type == DISPLAY)
	{
		display_data_t *d_data = ascui_get_display_data(container);

		if(d_data->var_binding->update_flag)	// Reconstruct buf and signal rebake of lines if var is updated
		{
			d_data->baked_available_width = 0;
			switch (d_data->var_binding->var_type)
			{
				case U32_INT: 	str_write_from_sprintf(d_data->fmt->length + 20, &d_data->display_text, d_data->fmt, *(uint32_t *)d_data->var_binding->var);	break;
				case U16_INT: 	str_write_from_sprintf(d_data->fmt->length + 20, &d_data->display_text, d_data->fmt, *(uint16_t *)d_data->var_binding->var); 	break;
				case U8_INT: 	str_write_from_sprintf(d_data->fmt->length + 20, &d_data->display_text, d_data->fmt, *(uint8_t *)d_data->var_binding->var); 	break;
				case S32_INT: 	str_write_from_sprintf(d_data->fmt->length + 20, &d_data->display_text, d_data->fmt, *(int32_t *)d_data->var_binding->var);		break;
				case S16_INT: 	str_write_from_sprintf(d_data->fmt->length + 20, &d_data->display_text, d_data->fmt, *(int16_t *)d_data->var_binding->var); 	break;
				case S8_INT: 	str_write_from_sprintf(d_data->fmt->length + 20, &d_data->display_text, d_data->fmt, *(int8_t *)d_data->var_binding->var); 		break;
				case FLOAT: 	str_write_from_sprintf(d_data->fmt->length + 20, &d_data->display_text, d_data->fmt, *(float *)d_data->var_binding->var); 		break;
				case STRING: 	str_write_from_str(&d_data->display_text, (str_t *)d_data->var_binding->var);													break;
				default: 		str_write(&d_data->display_text, "???"); 																						break;
			}
		}

		draw_text(grid, d_data->display_text, d_data->h_alignment, d_data->v_alignment, d_data->style, 
								x0, y0, x1, y1, selected || hovered, &d_data->baked_line_widths, 
								&d_data->baked_available_width, &container->scroll_offset, cursor->scroll);

		return (parent_orientation == VERTICAL)? x1-x0 + 1 : y1-y0 + 1;
	}
	else if(c_type == DIVIDER)
	{
		container_style_t style = ascui_get_divider_data(container)->style;

		if(parent_orientation == HORIZONTAL)
		{
			tl_draw_line_smbl_w_bg(grid, x0 + 1, y0, x1 - 1, y0, style.border_h_symbol, style.char_col, style.border_col, style.tex_map);
			tl_plot_smbl_w_bg(grid, x0, y0, style.corner_symbol, style.char_col, style.border_col, style.tex_map);
			tl_plot_smbl_w_bg(grid, x1, y0, style.corner_symbol, style.char_col, style.border_col, style.tex_map);
		}
		else
		{
			tl_draw_line_smbl_w_bg(grid, x0, y0 + 1, x0, y1 - 1, style.border_v_symbol, style.char_col, style.border_col, style.tex_map);
			tl_plot_smbl_w_bg(grid, x0, y0, style.corner_symbol, style.char_col, style.border_col, style.tex_map);
			tl_plot_smbl_w_bg(grid, x0, y1, style.corner_symbol, style.char_col, style.border_col, style.tex_map);
		}

		return 1;
	}
	else if(c_type == SLIDER)
	{
		slider_data_t *sl_data = ascui_get_slider_data(container);
		
		color8b_t bg_col;
		color8b_t smbl_col;
		uint8_t vertical_midpoint = y0 + (y1 - y0) / 2;
		if(hovered || selected)
		{
			bg_col = sl_data->style.char_col;
			smbl_col = sl_data->style.bg_col;
		}
		else
		{
			bg_col = sl_data->style.bg_col;
			smbl_col = sl_data->style.char_col;
		}

		tl_draw_rect_bg(grid, x0, y0, x1, y1, bg_col);

		// variable updated from elsewhere
		if(sl_data->var_binding->update_flag)
		{
			switch (sl_data->var_binding->var_type)
			{
				case U32_INT:  	sl_data->slide_percentage = (double)(*(uint32_t *)sl_data->var_binding->var - sl_data->min) / (abs(sl_data->max - sl_data->min)); 	break;
				case U16_INT:  	sl_data->slide_percentage = (double)(*(uint16_t*)sl_data->var_binding->var - sl_data->min) / (abs(sl_data->max - sl_data->min)); 	break;
				case U8_INT: 	sl_data->slide_percentage = (double)(*(uint8_t*)sl_data->var_binding->var - sl_data->min) / abs(sl_data->max - sl_data->min); 	break;
				case S32_INT:  	sl_data->slide_percentage = (double)(*(int32_t*)sl_data->var_binding->var - sl_data->min) / (abs(sl_data->max - sl_data->min)); 	break;
				case S16_INT:  	sl_data->slide_percentage = (double)(*(int16_t*)sl_data->var_binding->var - sl_data->min) / (abs(sl_data->max - sl_data->min));		break;
				case S8_INT: 	sl_data->slide_percentage = (double)(*(int8_t*)sl_data->var_binding->var - sl_data->min) / (abs(sl_data->max - sl_data->min)); 		break;
				case FLOAT: 	sl_data->slide_percentage = (double)(*(float *)sl_data->var_binding->var - sl_data->min) / (abs(sl_data->max - sl_data->min));		break;
				default:		 																															break;
			}
		}

		// variable updated from UI
		if(hovered && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
		{
			sl_data->slide_percentage = (double)(cursor->x - x0) / (double)(x1 - x0);

			float val = abs(sl_data->max - sl_data->min) * sl_data->slide_percentage;	// Calculate at a high precision (float), downgrade later...
			val += sl_data->min;
			
			switch (sl_data->var_binding->var_type)
			{
				case U32_INT:  	*(uint32_t *)(sl_data->var_binding->var) = clamp(sl_data->min, val, sl_data->max); 	break;			
				case U16_INT:  	*(uint16_t *)(sl_data->var_binding->var) = clamp(sl_data->min, val, sl_data->max); 	break;
				case U8_INT: 	*(uint8_t *)(sl_data->var_binding->var) = clamp(sl_data->min, val, sl_data->max); 	break;
				case S32_INT:  	*(int32_t *)(sl_data->var_binding->var) = clamp(sl_data->min, val, sl_data->max); 	break;
				case S16_INT:  	*(int16_t *)(sl_data->var_binding->var) = clamp(sl_data->min, val, sl_data->max);	break;	
				case S8_INT: 	*(int8_t *)(sl_data->var_binding->var) = clamp(sl_data->min, val, sl_data->max); 	break;	
				case FLOAT:		*(float *)(sl_data->var_binding->var) = flclamp(sl_data->min, val, sl_data->max);	break;			
				default:		 																					break;
			}
			flag_bound_var(sl_data->var_binding);
		}
		uint8_t knob_x = x0 + (x1 - x0) * sl_data->slide_percentage;

		tl_draw_line_bg(grid, x0, vertical_midpoint, x1, vertical_midpoint, smbl_col);
		tl_draw_line_bg(grid, knob_x, y0, knob_x, y1, sl_data->style.border_col);
		
		return (parent_orientation == VERTICAL)? x1-x0 + 1 : y1-y0 + 1;
	}

	return 0;
}

void ascui_draw_ui(grid_t *grid, container_t *top_container, cursor_t *cursor)
{
	cursor->hovered_container = NULL; // Reset hovered container

	pos16_t grid_size = tl_grid_get_dimensions(grid);
	container_data_t *c_data = ascui_get_container_data(top_container);
	ascui_draw_container(grid, top_container, 0, 0, grid_size.x - 1, grid_size.y - 1, c_data->orientation, cursor);
}

void ascui_update_cursor(grid_t *grid, cursor_t *cursor)
{
	pos16_t mouse_grid_pos = tl_screen_to_grid_coords(grid, pos16(GetMouseX(), GetMouseY()));
	cursor->x = mouse_grid_pos.x;
	cursor->y = mouse_grid_pos.y;
	cursor->scroll = GetMouseWheelMove();
}

void ascui_navigate_ui(grid_t *grid, container_t *top_container, cursor_t *cursor, double *ascui_drawing_time, Sound *click_sound, Sound *scroll_sound)
{
	double t_0;
	double t_1;
	if(ascui_drawing_time)
		t_0 = GetTime();
		
	ascui_draw_ui(grid, top_container, cursor);

	if(ascui_drawing_time)
		{ t_1 = GetTime(); *ascui_drawing_time += t_1 - t_0; }

	// Container hovering and selecting

	if(cursor->hovered_container != NULL)
	{
		if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && (cursor->hovered_container->container_type == BUTTON || cursor->hovered_container->selectability == SELECTABLE))
		{
			if(click_sound)
				PlaySound(*click_sound);
		}
		
		if (!(cursor->scroll > 0 && cursor->hovered_container->scroll_offset == 0) && cursor->scroll != 0)
		{
			cursor->hovered_container->scroll_offset -= cursor->scroll;
			if (scroll_sound && !IsSoundPlaying(*scroll_sound))
				PlaySound(*scroll_sound);
		}
			
		if (cursor->hovered_container->container_type == BUTTON)
		{
			button_data_t *bt_data = ascui_get_button_data(cursor->hovered_container);
			if(bt_data->side_effect_func)
				bt_data->side_effect_func(bt_data->domain, bt_data->function_data, cursor);
		}

		// Select
		if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && cursor->hovered_container->selectability == SELECTABLE)
		{
			cursor->selected_container = cursor->hovered_container;
		}
		// Deselect
		if(cursor->hovered_container == cursor->selected_container && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
			cursor->selected_container = NULL;
	}
}

void ascui_adapt_grid_to_screen(grid_t *grid, int zoom_in_key, int zoom_out_key)
{
		int screensize_x;
		int screensize_y;

		if(IsKeyDown(zoom_in_key))
		{
			screensize_x = GetScreenWidth();
			screensize_y = GetScreenHeight();
			
			uint16_t new_tile_size = grid->tile_p_w + 1;
			tl_resize_grid(grid, 0, 0, screensize_x, screensize_y, new_tile_size);
			tl_center_grid_on_screen(grid, screensize_x, screensize_y);
		}
		else if(IsKeyDown(zoom_out_key))
		{
			screensize_x = GetScreenWidth();
			screensize_y = GetScreenHeight();
		
			uint16_t new_tile_size = grid->tile_p_w - 1;
			tl_resize_grid(grid, 0, 0, screensize_x, screensize_y, new_tile_size);
			tl_center_grid_on_screen(grid, screensize_x, screensize_y);
		}

		if(IsWindowResized())
		{
			screensize_x = GetScreenWidth();
			screensize_y = GetScreenHeight();
			
			tl_resize_grid(grid, 0, 0, screensize_x, screensize_y, grid->tile_p_w);
			tl_center_grid_on_screen(grid, screensize_x, screensize_y);
		}
}

void ascui_run_ui(grid_t *grid, context_t *ctx, double *ascui_drawing_time, Sound *click_sound, Sound *scroll_sound,
						int zoom_in_key, int zoom_out_key, cursor_t *cursor)
{
	for (uint16_t i = 0; i < ctx->binding_capacity; i++)
		if(ctx->var_bindings[i].update_flag > 0)
			ctx->var_bindings[i].update_flag--;

	ascui_update_cursor(grid, cursor);
	ascui_navigate_ui(grid, ctx->top_container, cursor, ascui_drawing_time, click_sound, scroll_sound);
	ascui_adapt_grid_to_screen(grid, zoom_in_key, zoom_out_key);
}

/// DEFAULT BUTTON FUNCTIONS

void ascui_dropdown_button_func(void *dropdown_cntr, void *button_text, cursor_t *cursor)
{
	UNUSED(cursor);
	if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		return;
	container_t *container = (container_t *)dropdown_cntr; 
	container->open = !container->open;
	str_set(button_text, 0, (container->open)? 'V' : '^');
}










