#include "ascui.h"
#include "assert.h"
#include "stdio.h"
#include "string.h"
#include "common.h"

#define SIZE_INCREASE_ON_REALLOC 4

static void array_make_contiguos(void **array, uint len)
{
    uint gaps = 0;
    for (size_t i = 0; i < len; i++)
    {
        if (array[i] == NULL) {gaps++; continue;}

        array[i - gaps] = array[i];
        array[i] = NULL;
    }
}

/// NOTE: Only works with arrays of addresses
static void **array_realloc(void **array_ptr, uint current_len, uint increase)
{
    uint new_len = current_len + increase;
    array_ptr = realloc(array_ptr, new_len * sizeof(void *));

    /// The realloc can fail, TODO: better handling of error than crashing
    if(array_ptr == NULL) 
    {
        printf("\nERROR:\n    Could not reallocate array!");
        assert(false); // GG
    }

    // NULL all new adresses
    for (size_t i = current_len; i < new_len; i++) array_ptr[i] = NULL;   
    return array_ptr;    
}

UIContext_t *ascui_context_create()
{
    UIContext_t * ui_cntxt = calloc(1, sizeof(UIContext_t));
    ui_cntxt->cursor = (Cursor_t){.pos = (Pos_t){0,0}, .box_selected = 0};
}

void ascui_ui_context_destroy(UIContext_t **ui_cntxt)
{
    for (size_t i = 0; i < (*ui_cntxt)->num_boxes; i++)
    {
        if((*ui_cntxt)->ui_boxes[i] != NULL) ascui_ui_box_destroy((*ui_cntxt)->ui_boxes[i]);
    }
    free(*ui_cntxt);
    *ui_cntxt = NULL;
}

void draw_box(Pos_t pos, uint width, uint length, UIBoxStyle_t style)
{
    // Background, we offset by +1 and -2 to account for the borders.
    tl_draw_rect(pos.x + 1, pos.y + 1, width - 2, length - 2, NO_SMBL, NO_BG, style.bg_col, NULL); 

    // Borders and corners
    uint length_coord = pos.y + length;
    uint width_coord = pos.x + width;

    tl_draw_line(pos.x, pos.y,width_coord, pos.y, style.border_h_smbl, style.border_col, style.char_col, NULL);
    tl_draw_line(width_coord, pos.y, width_coord, length_coord, style.border_v_smbl, style.border_col, style.char_col, NULL);
    tl_draw_line(width_coord, length_coord, pos.x, length_coord, style.border_h_smbl, style.border_col, style.char_col, NULL);
    tl_draw_line(pos.x, length_coord, pos.x, pos.y, style.border_v_smbl, style.border_col, style.char_col, NULL);

    tl_draw_tile(pos.x, pos.y, style.corner_smbl, style.border_col, style.char_col, NULL);
    tl_draw_tile(width_coord, pos.y, style.corner_smbl, style.border_col, style.char_col, NULL);
    tl_draw_tile(width_coord, length_coord, style.corner_smbl, style.border_col, style.char_col, NULL);
    tl_draw_tile(pos.x, length_coord, style.corner_smbl, style.border_col, style.char_col, NULL);
}


char *ascui_wrap_str(char *str, uint *calc_rows, uint *len, uint wrap);

bool *ui_element_scheduler(UIBox_t *ui_box)
{
    uint *length_profile = ui_element_get_length_profile(ui_box->ui_elements, ui_box->num_elements, ui_box->width - 2);
    bool *schedule = calloc(ui_box->num_elements, sizeof(bool));

    uint i = ui_box->element_selected;
    uint steps_up = 1;
    uint steps_down = 1;
    uint c_total_len = 0;

    bool up_done = false;
    bool down_done = false; 

    // Starting UI element
    schedule[i] = length_profile[i];
    c_total_len += length_profile[i];

    while (c_total_len <= ui_box->length)
    {
        uint i_up = i - steps_up;   // This needs to overflow HERE, compiler will do fucky-wucky if done directly in the if-statement
                                    // The compiler will pretend that overflows can't happen -> it evals to true even if it's false
        uint i_down = i + steps_down;
        /// NOTE: i - steps_up will overflow when invalid, hence checking for a large number
        if (i_up < ui_box->num_elements && length_profile[i_up] + c_total_len <= ui_box->length)
        {
            schedule[i_up] = true;
            steps_up++;
            c_total_len += length_profile[i_up];
        }
        else up_done = true;
        
        if (i_down < ui_box->num_elements && length_profile[i_down] + c_total_len <= ui_box->length)
        {
            schedule[i_down] = true;
            steps_down++;
            c_total_len += length_profile[i_down];
        }
        else down_done = true;

        if(up_done && down_done) break;
    }
    free(length_profile);
    return schedule;
}

void draw_box_elements(UIBox_t *ui_box)
{
    uint row = 0;
    UIElement_t current_ui_element;
    bool *schedule = ui_element_scheduler(ui_box);

    Color bg_col;

    for (size_t i = 0; i < ui_box->num_elements; i++)
    {
        if(schedule[i] == false) continue;

        current_ui_element = *ui_box->ui_elements[i];

        bg_col = (ui_box->element_selected == i)? (Color){ 0, 180, 255, 255 } : current_ui_element.style.bg_col;

        row += tl_draw_text(ui_box->pos.x + 1, ui_box->pos.y + 1 + row, ui_box->pos.x + ui_box->width,
                            current_ui_element.text, current_ui_element.text_len, current_ui_element.style.char_col,
                            bg_col, NULL);
        // row++;        
    }
    free(schedule);
}


void ascui_draw_to_grid(UIContext_t *ui_cntxt)
{
    Pos_t grid_size = tl_grid_get_size();
    uint x_edge = grid_size.x - 1;
    uint y_edge = grid_size.y - 1;

    UIBox_t *c_ui_box = NULL;
    Pos_t pos = (Pos_t){0,0};
    uint width = 0;
    uint length = 0;
    UIBoxStyle_t style;

    for (size_t i = 0; i < ui_cntxt->num_boxes; i++)
    {
        c_ui_box = ui_cntxt->ui_boxes[i];

        // Skip removed boxes - this assumes the list of boxes
        // is continuos with no gaps.
        if(c_ui_box == NULL) break;
        pos = c_ui_box->pos;
        width = c_ui_box->width;
        length = c_ui_box->length;
        style = c_ui_box->style;

        // BOX
        draw_box(pos, width, length, style);
        draw_box_elements(c_ui_box);
    }
    
}

void ascui_ui_context_add_box(UIContext_t *ui_cntxt, UIBox_t *ui_box)
{
    Pos_t grid_size = tl_grid_get_size();
    uint x_edge = grid_size.x - 1;
    uint y_edge = grid_size.y - 1;

    assert(ui_box->pos.x <= x_edge);
    assert(ui_box->pos.y <= y_edge);

    // Reallocate to fit more boxes if full
    if (ui_cntxt->num_boxes + 1 >= ui_cntxt->box_capacity)
    {
        ui_cntxt->ui_boxes = (UIBox_t **)array_realloc((void **)(ui_cntxt->ui_boxes), ui_cntxt->box_capacity, SIZE_INCREASE_ON_REALLOC);
        ui_cntxt->box_capacity += SIZE_INCREASE_ON_REALLOC;
    }

    // 'num_boxes' should represent the latest box + 1 since it is a size.
    UIBox_t **new_ui_box_slot = &ui_cntxt->ui_boxes[ui_cntxt->num_boxes]; 

    // We do not want to overwrite anything in this stage, if we are
    // then something is wrong --> crash
    assert(*new_ui_box_slot == NULL);

    *new_ui_box_slot = ui_box;
    ui_cntxt->num_boxes++;
}

static void ui_context_remove_dangling_neighbours(UIContext_t *ui_cntxt, UIBox_t *dangling_ptr)
{
    UIBox_t *current_ui_box = NULL;
    for (size_t i = 0; i < ui_cntxt->num_boxes; i++)
    {
        current_ui_box = ui_cntxt->ui_boxes[i];

        if(current_ui_box == NULL) break;

        if(current_ui_box->n_ngbr == dangling_ptr) current_ui_box->n_ngbr = NULL;
        if(current_ui_box->s_ngbr == dangling_ptr) current_ui_box->s_ngbr = NULL;
        if(current_ui_box->e_ngbr == dangling_ptr) current_ui_box->e_ngbr = NULL;
        if(current_ui_box->w_ngbr == dangling_ptr) current_ui_box->w_ngbr = NULL;
    }
} 

void ascui_ui_context_remove_box(UIContext_t *ui_cntxt, uint i)
{
    // Don't try to remove anything outside of the array
    assert(i < ui_cntxt->num_boxes);

    UIBox_t *ui_box_slot = ui_cntxt->ui_boxes[i]; 

    // Don't remove an empty slot
    assert(ui_box_slot != NULL);

    // We need to NULL all references to this ui_box
    ui_context_remove_dangling_neighbours(ui_cntxt, ui_box_slot);

    ascui_ui_box_destroy(ui_box_slot);
    ui_cntxt->ui_boxes[i] = NULL; // Null the actual slot in memory
    ui_cntxt->num_boxes--;

    array_make_contiguos((void **)ui_cntxt->ui_boxes, ui_cntxt->num_boxes);
}


UIBox_t *ascui_ui_box_create(UIBoxType_e type, Pos_t pos, uint width, uint length)
{
    UIBox_t *ui_box = calloc(1, sizeof(UIBox_t));

    ui_box->type = type;

    ui_box->pos = pos;
    ui_box->width = width;
    ui_box->length = length;

    ui_box->ui_elements = calloc(SIZE_INCREASE_ON_REALLOC, sizeof(UIBox_t *));
    ui_box->element_capacity = SIZE_INCREASE_ON_REALLOC;
    ui_box->num_elements = 0;
    ui_box->element_selected = 0;
}


void ascui_ui_box_destroy(UIBox_t *ui_box)
{
    for (size_t i = 0; i < ui_box->num_elements; i++)
    {
        if(ui_box->ui_elements[i] != NULL) ascui_ui_element_destroy(ui_box->ui_elements[i]);
    }
    free(ui_box);
}

void ascui_ui_box_set_style_with_ptr(UIBox_t *ui_box, UIBoxStyle_t *style)
{
    ui_box->style = *style;
}

void ascui_ui_box_set_style(UIBox_t *ui_box, 
                            Color bg_col,
                            Color border_col,
                            Color char_col,
                            char border_h_smbl,
                            char border_v_smbl,
                            char corner_smbl)
{
    UIBoxStyle_t style = (UIBoxStyle_t){bg_col, border_col, char_col, border_h_smbl, border_v_smbl, corner_smbl};
    ui_box->style = style;
}

void ascui_ui_box_set_ngbrs(UIBox_t *ui_box,
                            UIBox_t *n_ngbr,
                            UIBox_t *s_ngbr,
                            UIBox_t *w_ngbr,
                            UIBox_t *e_ngbr)
{
    ui_box->n_ngbr = n_ngbr;
    ui_box->s_ngbr = s_ngbr;
    ui_box->w_ngbr = w_ngbr;
    ui_box->e_ngbr = e_ngbr;
}

void ascui_ui_box_add_element(UIBox_t *ui_box, UIElement_t *ui_element, bool text_allocated)
{
    // Reallocate to fit more elements if full
    if (ui_box->num_elements + 1 >= ui_box->element_capacity)
    {
        ui_box->ui_elements = (UIElement_t **)array_realloc((void **)(ui_box->ui_elements), ui_box->element_capacity, SIZE_INCREASE_ON_REALLOC);   
        ui_box->element_capacity += SIZE_INCREASE_ON_REALLOC;
    }

    // 'num_elements' should represent the latest element + 1 since it is a size.
    UIElement_t **new_ui_element_slot = &ui_box->ui_elements[ui_box->num_elements]; 

    // We do not want to overwrite anything in this stage, if we are
    // then something is wrong --> crash
    assert(*new_ui_element_slot == NULL);

	// Sanitize new element
	if(ui_element->type == UI_TEXT)
	{
		char *old_txt = ui_element->text; 
	
		ui_element->text = ascui_wrap_str(ui_element->text, &(ui_element->rows_required), &(ui_element->text_len), ui_box->width-2);

		if(text_allocated)
			free(old_txt);
	}

    *new_ui_element_slot = ui_element;
    ui_box->num_elements++;
}

void ascui_ui_box_remove_element(UIBox_t *ui_box, uint i)
{
    assert(i < ui_box->num_elements);

    UIElement_t *ui_element_slot = ui_box->ui_elements[i]; 

    // Don't remove an empty slot
    assert(ui_element_slot != NULL);

    ascui_ui_element_destroy(ui_element_slot);
    ui_box->ui_elements[i] = NULL; // Null the actual slot in memory
    ui_box->num_elements--;
   
    array_make_contiguos((void **)ui_box->ui_elements, ui_box->num_elements);
}

void ascui_ui_box_set_domain(UIBox_t *ui_box, void *domain)
{
    ui_box->domain = domain;
}

UIElement_t *ascui_ui_element_create(UIElemenType_e type,
                                    UI_side_effect_func side_effect_func,
                                    void *type_data,
                                    char *text)
{
    UIElement_t *ui_element = calloc(1, sizeof(UIElement_t));
    ui_element->type = type;
    ui_element->type_data = type_data;
    ui_element->side_effect_func = side_effect_func;
    ui_element->text_len = strlen(text);
    ui_element->text = calloc(ui_element->text_len + 1, sizeof(char));
    memcpy(ui_element->text, text, ui_element->text_len + 1);

    return ui_element;
}

void ascui_ui_element_destroy(UIElement_t *ui_element)
{
    free(ui_element->text);
    free(ui_element->type_data);
    free(ui_element);
}

void ascui_ui_element_set_style_with_ptr(UIElement_t *ui_element, UIElementStyle_t *style)
{
    ui_element->style = *style;
}

void ascui_ui_element_set_style(UIElement_t *ui_element, Color bg_col, Color char_col)
{
    ui_element->style.bg_col = bg_col;
    ui_element->style.char_col = char_col;
}

bool is_word_char(char c)
{
	return (c != ' ' && c != '\n');
}

// Allocates, modifies calc_rows & len
char *ascui_wrap_str(char *str, uint *calc_rows, uint *len, uint wrap)
{
	assert(wrap > 2); // Fuck off, no tiny texts. 2 hard 2 code.

	char *wrapped_str = calloc(*len * 3, sizeof(char)); // len*3 is the maximum theoretical increase in size
	uint wrapped_index_offset = 0; // As new characters are inserted the indexes of 'str' and 'wrapped_str' diverge

	uint c_line_len = 0;
	uint num_rows = 1;

	for(size_t i = 0; i < *len; i++)
	{
		if(str[i] == '\n')
		{
			wrapped_str[i + wrapped_index_offset] = '\n';
			num_rows++;
			c_line_len = 0;
			continue;
		}
	
		// Determine if we need to wrap, also how we should wrap 
		if(c_line_len >= wrap - 1) 	// WRAP
		{
			bool pre_char = is_word_char(str[i - 1]);
			bool this_char = is_word_char(str[i]);
			bool post_char = is_word_char(str[i + 1]);

			if(!this_char || (this_char && !post_char)) 	
			{
				// PUT CHAR THEN BREAK LINE
				wrapped_str[i + wrapped_index_offset] = str[i];	
				wrapped_str[i + wrapped_index_offset + 1] = '\n';
				wrapped_index_offset++;
			}
			else if(pre_char)								
			{
				// SPLIT: PUT '-' THEN BREAK LINE, MOVE BACK i
				wrapped_str[i + wrapped_index_offset] = '-';	
				wrapped_str[i + wrapped_index_offset + 1] = '\n';
				wrapped_index_offset += 2;	
				i--; 						
			}
			else if(!pre_char)								
			{
				// MOVE: BREAK LINE, MOVE BACK i
				wrapped_str[i + wrapped_index_offset] = '\n'; 
				wrapped_index_offset++;
				i--; 			
			}
			else											
				// KYS
				assert(0); 
				
			num_rows++;
			c_line_len = 0;	
		}
		else						// PUT CHAR 
		{
			wrapped_str[i + wrapped_index_offset] = str[i];
			c_line_len++;			
		}
	}

	*calc_rows = num_rows;
	return wrapped_str;
}


