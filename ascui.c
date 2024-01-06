#include "ascui.h"
#include "assert.h"

#define SIZE_INCREASE_ON_REALLOC 4

UIContext_t *ascui_context_create()
{
    UIContext_t * ui_cntxt = calloc(1, sizeof(UIContext_t));
    ui_cntxt->cursor = (Cursor_t){.pos = (Pos_t){0,0}, .box_selected = 0};
}

void ascui_context_destroy(UIContext_t **ui_cntxt)
{
    for (size_t i = 0; i < (*ui_cntxt)->num_boxes; i++)
    {
        if((*ui_cntxt)->ui_boxes[i] != NULL) ascui_ui_box_destroy((*ui_cntxt)->ui_boxes[i]);
    }
    free(*ui_cntxt);
    *ui_cntxt = NULL;
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

        // Background, we offset by +1 and -2 to account for the borders.
        tl_draw_rect(pos.x + 1, pos.y + 1, width - 2, length - 2, NO_SMBL, NO_BG, style.bg_col, NULL); 

        // Borders and corners
        uint length_coord = pos.y + length;
        uint width_coord = pos.x + width;

        tl_draw_line(pos.x, pos.y, width_coord, pos.y, '-', style.border_col, style.char_col, NULL);
        tl_draw_line(width_coord, 0, width_coord, length_coord, '|', style.border_col, style.char_col, NULL);
        tl_draw_line(width_coord, length_coord, 0, length_coord, '-', style.border_col, style.char_col, NULL);
        tl_draw_line(0,length_coord, 0, 0, '|', style.border_col, style.char_col, NULL);


        tl_draw_tile(pos.x, pos.y, '+', style.border_col, style.char_col, NULL);
        tl_draw_tile(width_coord, pos.y, '+', style.border_col, style.char_col, NULL);
        tl_draw_tile(width_coord, length_coord, '+', style.border_col, style.char_col, NULL);
        tl_draw_tile(pos.x, length_coord, '+', style.border_col, style.char_col, NULL);
    }
    
}

void ascui_ui_context_add_box(UIContext_t *ui_cntxt, UIBox_t *ui_box)
{
    // Reallocate to fit more boxes if full
    if (ui_cntxt->num_boxes + 1 >= ui_cntxt->box_capacity)
    {
        ui_cntxt->box_capacity += SIZE_INCREASE_ON_REALLOC;
        ui_cntxt->ui_boxes = realloc(ui_cntxt->ui_boxes, ui_cntxt->box_capacity);    
    }

    // 'num_boxes' should represent the latest box + 1 since it is a size.
    UIBox_t *new_ui_box_slot = ui_cntxt->ui_boxes[ui_cntxt->num_boxes]; 

    // We do not want to overwrite anything in this stage, if we are
    // then something is wrong --> crash
    assert(new_ui_box_slot == NULL);

    new_ui_box_slot = ui_box;
    ui_cntxt->num_boxes++;
}

void ascui_ui_context_remove_box(UIContext_t *ui_cntxt, uint i)
{
    // Don't try to remove anything outside of the array
    assert(i < ui_cntxt->num_boxes);

    UIBox_t *ui_box_slot = ui_cntxt->ui_boxes[i]; 

    // Don't remove an empty slot
    assert(ui_box_slot != NULL);

    ascui_ui_box_destroy(ui_box_slot);
    ui_cntxt->ui_boxes[i] = NULL; // Null the actual slot in memory
    ui_cntxt->num_boxes--;
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

void ascui_ui_box_destroy(UIBox_t *ui_box);

void ascui_ui_box_set_style_with_ptr(UIBox_t *ui_box, UIBoxStyle_t *style);

void ascui_ui_box_set_style(UIBox_t *ui_box, 
                            Color bg_col,
                            Color border_col,
                            Color char_col,
                            char border_h_smbl,
                            char border_v_smbl,
                            char corner_smbl);

void ascui_ui_box_set_ngbrs(UIBox_t *ui_box,
                            UIBox_t *n_ngbr,
                            UIBox_t *s_ngbr,
                            UIBox_t *w_ngbr,
                            UIBox_t *e_ngbr);

void ascui_ui_box_add_element(UIBox_t *ui_box, UIElement_t *element);

void ascui_ui_box_remove_element(UIBox_t *ui_box, uint i);

void ascui_ui_box_set_domain(UIBox_t *ui_box, void *domain)
{
    ui_box->domain = domain;
}

UIElement_t *ascui_ui_element_create(UIElemenType_e type,
                                    UI_side_effect_func side_effect_func,
                                    void *type_data,
                                    char *text);

void ascui_ui_element_destroy(UIElement_t *ui_element);

void ascui_ui_element_set_style_with_ptr(UIElement_t *ui_element, UIElementStyle_t *style);

void ascui_ui_element_set_style(UIElement_t *ui_element, Color bg_col, Color char_col);