#pragma once
#include "common.h"
#include "raytiles.h"
#include <raylib.h>

/// @brief std. name: element_style
typedef struct 
{
    Color bg_col;
    Color char_col;    
}UIElementStyle_t;

/// @brief std. name: box_style
typedef struct 
{
    Color bg_col;
    Color border_col;
    Color char_col;
    char border_h_smbl;
    char border_v_smbl;
    char corner_smbl;
}UIBoxStyle_t;

typedef enum
{
    UI_TEXT,        // TODO: Implement 
    UI_BUTTON,      // TODO: Implement
    UI_TOGGLE,      // TODO: Implement
    UI_SLIDER,      // TODO: Implement
    UI_INPUT_FIELD  // TODO: Implement
}UIElemenType_e;

typedef int (*UI_side_effect_func)(void *domain, void *type_data, KeyboardKey input);

/// @brief std. name: ui_element
typedef struct
{
    UIElementStyle_t style;
    UIElemenType_e type;
    UI_side_effect_func side_effect_func;
    void *type_data;
    uint text_len;
    char *text;
}UIElement_t;

typedef enum
{
    UI_ACTION_LIST, // TODO: Implement
    UI_POPUP,       // TODO: Implement
    UI_CAMERA       // TODO: Implement
}UIBoxType_e;

typedef struct UIBox UIBox_t;

/// @brief std. name: ui_box
typedef struct UIBox
{
    UIBoxStyle_t style;
    UIBoxType_e type;
    void *domain;       // Elements' side effect functions must know what type this is. (Eg. Inventory struct)

    Pos_t pos;
    uint width;
    uint length;

    UIBox_t *n_ngbr;
    UIBox_t *s_ngbr;
    UIBox_t *w_ngbr;
    UIBox_t *e_ngbr;

    uint element_capacity;
    uint num_elements;
    uint element_selected;
    UIElement_t **ui_elements;

} UIBox_t;

typedef struct
{
    Pos_t pos; // For rendering in free spaces
    UIBox_t *box_selected;
} Cursor_t;

/// @brief std. name: ui_cntxt
typedef struct 
{
    Cursor_t cursor;
    uint num_boxes;
    uint box_capacity;
    UIBox_t **ui_boxes;

}UIContext_t;

///   UI-Context    ///

UIContext_t *ascui_context_create();

void ascui_ui_context_destroy(UIContext_t **ui_cntxt);

void ascui_ui_context_add_box(UIContext_t *ui_cntxt, UIBox_t *ui_box);

void ascui_draw_to_grid(UIContext_t *ui_cntxt);

///      UI-Box     ///

UIBox_t *ascui_ui_box_create(UIBoxType_e type, Pos_t pos, uint width, uint length);

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

void ascui_ui_box_add_element(UIBox_t *ui_box, UIElement_t *ui_element);

void ascui_ui_box_remove_element(UIBox_t *ui_box, uint i);

/// @brief 
/// @param ui_box 
/// @param domain data which the box can read and write to, keeping track of type is user-side.
void ascui_ui_box_set_domain(UIBox_t *ui_box, void *domain);

///    UI-Element   ///

UIElement_t *ascui_ui_element_create(UIElemenType_e type,
                                    UI_side_effect_func side_effect_func,
                                    void *type_data,
                                    char *text);

void ascui_ui_element_destroy(UIElement_t *ui_element);

void ascui_ui_element_set_style_with_ptr(UIElement_t *ui_element, UIElementStyle_t *style);

void ascui_ui_element_set_style(UIElement_t *ui_element, Color bg_col, Color char_col);