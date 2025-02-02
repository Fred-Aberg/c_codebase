#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "common.h"
#include "ascui.h"
#include "raytiles.h"


static void check_cursor_selection(Cursor_t *cursor, Container_t *container, uint x0, uint y0, uint x1, uint y1)
{		
	if(x0 <= cursor->x && x1 >= cursor->x && y0 <= cursor->y && y1 >= cursor->y)
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


static uint ascui_draw_container(Grid_t *grid, Container_t *container, uint x0, uint y0, uint x1, uint y1, Container_orientation_e parent_orientation, Cursor_t *cursor);


static uint ascui_draw_container_percentage_split(Grid_t *grid, Container_t *container, uint x0, uint y0, uint x1, uint y1, Container_orientation_e parent_orientation, Cursor_t *cursor)
{
	if (!container->open)
		return 0;

	if (x1 <= x0 || y1 <= y0)
		return 0;

	check_cursor_selection(cursor, container, x0, y0, x1, y1);
		
	uint subcontainers_size = 0;
	float percentage;
	uint i = 0;
	uint drawn_tiles = 0;

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
			drawn_tiles += ascui_draw_container(grid, &subcontainers[i], x0, y0 + drawn_tiles, x1 , y0 + drawn_tiles + allotted_tiles[i] - 1, orientation, cursor);
		else	// VERTICAL
			drawn_tiles += ascui_draw_container(grid, &subcontainers[i], x0 + drawn_tiles, y0, x0 + drawn_tiles + allotted_tiles[i] - 1 , y1, orientation, cursor);		
	}

	if(container->container_type == BOX)
	{
		x0--;
		y0--;
		x1++;
		y1++;
		draw_box(grid, style, x0, y0, x1, y1);
	}
	return (parent_orientation == VERTICAL)? x1-x0 + 1 : y1-y0 + 1;
}

static uint ascui_draw_container(Grid_t *grid, Container_t *container, uint x0, uint y0, uint x1, uint y1, Container_orientation_e parent_orientation, Cursor_t *cursor)
{
	if (!container->open)
		return 0;

	if (x1 <= x0 || y1 <= y0)
		return 0;

	check_cursor_selection(cursor, container, x0, y0, x1, y1);

	uint tiles_drawn = 0; 	// Number of pixels required for this container and all it's subcontainers
							// Cannot exceed x1-x0 if VERTICAL, y1-y0 if HORIZONTAL
		

	float percentage;
	uint sub_x1;
	uint sub_y1;
	uint sub_x0;
	uint sub_y0;
	uint n_subcontainers;
	Container_t *c_subcontainer;
	Container_Type_e c_type = container->container_type;
	bool percentage_split = true;
	switch (c_type)
	{
		case CONTAINER:
			Container_data_t c_data = *ascui_get_container_data(*container);
			n_subcontainers = c_data.n_subcontainers;

			for (uint i = 0; i < n_subcontainers; i++)
				if (c_data.subcontainers[i].size_type != PERCENTAGE)
					percentage_split = false;

			if (percentage_split && n_subcontainers != 0)
				return ascui_draw_container_percentage_split(grid, container, x0, y0, x1, y1, parent_orientation, cursor);

			for (uint i = 0; i < n_subcontainers; i++)
			{
				c_subcontainer = &c_data.subcontainers[i];

				// Edge case: Last subcontainer
				if (i == n_subcontainers - 1)
				{
					if (c_data.orientation == HORIZONTAL)
					{
						ascui_draw_container(grid, c_subcontainer, x0, y0 + tiles_drawn, x1, y1, c_data.orientation, cursor);
						return y1-y0 + 1;
					}
					else if (c_data.orientation == VERTICAL)
					{
						tiles_drawn += ascui_draw_container(grid, c_subcontainer, x0 + tiles_drawn, y0, x1, y1, c_data.orientation, cursor);
						return x1-x0 + 1;
					}
				}

					
				if (c_subcontainer->size_type == TILES)
				{
					if (c_data.orientation == HORIZONTAL)
					{
						sub_x1 = x1;
						sub_y1 = y0 + tiles_drawn + c_subcontainer->size;
						if (sub_y1 > y1)
							break;
						tiles_drawn += ascui_draw_container(grid, c_subcontainer, x0, y0 + tiles_drawn, sub_x1, sub_y1, c_data.orientation, cursor);
					}
					else if (c_data.orientation == VERTICAL)
					{
						sub_x1 = x0 + tiles_drawn + c_subcontainer->size;
						sub_y1 = y1;
						if (sub_x1 > x1)
							break;
						tiles_drawn += ascui_draw_container(grid, c_subcontainer, x0 + tiles_drawn, y0, sub_x1, sub_y1, c_data.orientation, cursor);
					} 					
				}
				else if (c_subcontainer->size_type == PERCENTAGE)
				{
					percentage = (float)c_subcontainer->size / 100;
					if (c_data.orientation == HORIZONTAL)
					{
						sub_x1 = x1;
						sub_y1 = y0 + tiles_drawn + (y1-y0) * percentage - 1; // (y1-y0) * percentage is a length, -1 to make a coord
						if (sub_y1 > y1)
							break;
						tiles_drawn += ascui_draw_container(grid, c_subcontainer, x0, y0 + tiles_drawn, sub_x1, sub_y1, c_data.orientation, cursor);
					}
					else if (c_data.orientation == VERTICAL)
					{
						sub_x1 = x0 + tiles_drawn + (x1-x0) * percentage - 1;
						sub_y1 = y1;
						if (sub_x1 > x1)
							break;
						tiles_drawn += ascui_draw_container(grid, c_subcontainer, x0 + tiles_drawn, y0, sub_x1, sub_y1, c_data.orientation, cursor);
					} 					
				}
				else if (c_subcontainer->size_type == GROW)
				{
					if (c_data.orientation == HORIZONTAL)
					{
						sub_x1 = x1;
						sub_y1 = y1;
						tiles_drawn += ascui_draw_container(grid, c_subcontainer, x0, y0 + tiles_drawn, sub_x1, sub_y1, c_data.orientation, cursor);
					}
					else if (c_data.orientation == VERTICAL)
					{
						sub_x1 = x1;
						sub_y1 = y1;
						tiles_drawn += ascui_draw_container(grid, c_subcontainer, x0 + tiles_drawn, y0, sub_x1, sub_y1, c_data.orientation, cursor);
					} 					
				}
			}
			
			if (container->size_type != GROW)
			{
				// !GROW => Force the size to be x1-x0 or y1-y0
				if (parent_orientation == HORIZONTAL)
					tiles_drawn = y1-y0 + 1;
				else if (parent_orientation == VERTICAL)
					tiles_drawn = x1-x0 + 1;
			}
			
			return tiles_drawn;
		case BOX:
			Box_data_t b_data = *ascui_get_box_data(*container);
			n_subcontainers = b_data.n_subcontainers;
			sub_x0 = x0 + 1;
			sub_y0 = y0 + 1;

			for (uint i = 0; i < n_subcontainers; i++)
							if (b_data.subcontainers[i].size_type != PERCENTAGE)
								percentage_split = false;
			
			if (percentage_split && n_subcontainers != 0)
				return ascui_draw_container_percentage_split(grid, container, x0, y0, x1, y1, parent_orientation, cursor);
			

			for (uint i = 0; i < n_subcontainers; i++)
			{
				c_subcontainer = &b_data.subcontainers[i];

				// Edge case: Last subcontainer
				if (i == n_subcontainers - 1)
				{
					if (b_data.orientation == HORIZONTAL)
					{
						ascui_draw_container(grid, c_subcontainer, x0 + 1, y0 + tiles_drawn + 1, x1 - 1, y1 - 1, b_data.orientation, cursor);
						return y1-y0 + 1;
					}
					else if (c_data.orientation == VERTICAL)
					{
						tiles_drawn += ascui_draw_container(grid, c_subcontainer, x0 + tiles_drawn + 1, y0 + 1, x1 - 1, y1 - 1, b_data.orientation, cursor);
						return x1-x0 + 1;
					}
				}

				if (c_subcontainer->size_type == TILES)
				{
					if (b_data.orientation == HORIZONTAL)
					{
						sub_x1 = x1 - 1;
						sub_y1 = sub_y0 + tiles_drawn + c_subcontainer->size;
						if (sub_y1 > y1 - 1)
							break;
						tiles_drawn += ascui_draw_container(grid, c_subcontainer, sub_x0, sub_y0 + tiles_drawn, sub_x1, sub_y1, b_data.orientation, cursor);
					}
					else if (b_data.orientation == VERTICAL)
					{
						sub_x1 = sub_x0 + tiles_drawn + c_subcontainer->size;
						sub_y1 = y1 - 1;
						if (sub_x1 > x1 - 1)
							break;
						tiles_drawn += ascui_draw_container(grid, c_subcontainer, sub_x0 + tiles_drawn, sub_y0, sub_x1, sub_y1, b_data.orientation, cursor);
					} 					
				}
				else if (c_subcontainer->size_type == PERCENTAGE)
				{
					float percentage = (float)c_subcontainer->size / 100;
					if (b_data.orientation == HORIZONTAL)
					{
						sub_x1 = x1 - 1;
						sub_y1 = sub_y0 + tiles_drawn + (y1-y0-2) * percentage;
						if (sub_y1 > y1 - 1)
							break;
						tiles_drawn += ascui_draw_container(grid, c_subcontainer, sub_x0, sub_y0 + tiles_drawn, sub_x1, sub_y1, b_data.orientation, cursor);
					}
					else if (b_data.orientation == VERTICAL)
					{
						sub_x1 = sub_x0 + tiles_drawn + (x1-x0-2) * percentage;
						sub_y1 = y1 - 1;
						if (sub_x1 > x1 - 1)
							break;
						tiles_drawn += ascui_draw_container(grid, c_subcontainer, sub_x0 + tiles_drawn, sub_y0, sub_x1, sub_y1, b_data.orientation, cursor);
					} 					
				}
				else if (c_subcontainer->size_type == GROW)
				{
					if (b_data.orientation == HORIZONTAL)
					{
						sub_x1 = x1 - 1;
						sub_y1 = y1 - 1;
						tiles_drawn += ascui_draw_container(grid, c_subcontainer, sub_x0, sub_y0 + tiles_drawn, sub_x1, sub_y1, b_data.orientation, cursor);
					}
					else if (b_data.orientation == VERTICAL)
					{
						sub_x1 = x1 - 1;
						sub_y1 = y1 - 1;
						tiles_drawn += ascui_draw_container(grid, c_subcontainer, sub_x0 + tiles_drawn, sub_y0, sub_x1, sub_y1, b_data.orientation, cursor);
					} 					
				}
			}

			if (container->size_type == GROW)
			{
				// GROW => tiles_drawn is the deciding factor in how many tiles is to be drawn
				if (parent_orientation == HORIZONTAL)
					y1 = y0 + tiles_drawn + 1; 
				else if (parent_orientation == VERTICAL)
					x1 = x0 + tiles_drawn + 1; 
				tiles_drawn += 2; // Account for box's sides
			}
			else
			{
				// !GROW => The given size (x1-x0 or y1-y0) determines how many tiles are drawn
				if (parent_orientation == HORIZONTAL)
					tiles_drawn = y1-y0 + 1;
				else if (parent_orientation == VERTICAL)
					tiles_drawn = x1-x0 + 1;
			}

			draw_box(grid, b_data.style, x0, y0, x1, y1);
			
			return tiles_drawn;
		case TEXT:
			Text_data_t t_data = *ascui_get_text_data(*container);

			uint _x = 0;
			uint _y = 0;
			uint gap;

			for (uint i = 0; i < t_data.text_len; i++)
			{
				if (x0 + _x > x1)
					{ _y++; _x = 0; }
				
				if (y0 + _y > y1)
					return _y + 1; // tiles_drawn
				if (t_data.text[i] == '\n')
				{
					gap = x1-x0-_x+1;
					while (gap != 0)
					{
						tl_draw_tile(grid, x0 + _x, y0 + _y, 0, t_data.text_col, t_data.bg_col, NULL);
						gap--;
						_x++;
					}	
					_y++;
					_x = 0;
					continue; 
				}
				
				tl_draw_tile(grid, x0 + _x, y0 + _y, t_data.text[i], t_data.text_col, t_data.bg_col, NULL);
				_x++;
			}

			// Gap after text
			gap = x1-x0-_x + 1;
			while (gap != 0)
			{
				tl_draw_tile(grid, x0 + _x, y0 + _y, 0, t_data.text_col, t_data.bg_col, NULL);
				gap--;
				_x++;
			}
		
			// Gap below text
			for (uint y = _y + 1; y + y0 <= y1; y++)
				for (uint x = 0; x + x0 <= x1; x++)
					tl_draw_tile(grid, x0 + x, y0 + y, 0, t_data.text_col, t_data.bg_col, NULL);

			return (parent_orientation == VERTICAL)? x1-x0 + 1 : y1-y0 + 1;

		case SUBGRID:
			Subgrid_data_t *subg_data = ascui_get_subgrid_data(*container);
			tl_fit_subgrid(grid, &subg_data->subgrid, x0, y0, x1, y1);
		default:
			return 0;
	}
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

	t_data->text = text; // container owns text
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
		subg_data->subgrid = tl_init_grid(0,0, 10,10,4,4, default_color, default_font);

	return subgrid_container;
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


///	UI CONSTRUCTOR ///

Container_t ascui_construct_ui_from_file(char *ui_file_name)
{	
	FILE *fptr;
	
	// Open a file in read mode
	fptr = fopen(ui_file_name, "r");
	
	// Store the content of the file
	char myString[100];
	
	// Read the content and store it inside myString
	fgets(myString, 100, fptr);
	
	// Print the file content
	printf("%s", myString);
	
	// Close the file
	fclose(fptr); 
}
