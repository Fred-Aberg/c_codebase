#include "raylib.h"
#include "math.h"
#include "stdlib.h"
#include "stdio.h"
#include "minesweeper.h"
#include "raytiles.h"
#include "string.h"
#include "ascui.h"

const int SCREENSIZE_X = 1000;
const int SCREENSIZE_Y = 650;
const int GAME_RES_X = 50; // _Y is calculated
const int APP_RES_X = GAME_RES_X + 1; // two border lines, _Y is calculated
const int UI_HEIGHT = 3;
const int APP_X_SIZE = APP_RES_X + 1;



#define DEF_COLOR (Color){20, 40, 29, 255}

#define MIN(a, b) (a < b)? a : b
#define MAX(a, b) (a < b)? b : a

#define pos(x,y) (Pos_t){x, y}

int main(void)
{
    /// SETUP
    InitWindow(SCREENSIZE_X, SCREENSIZE_Y, "ascUI 2024");

    Font square_font = LoadFontEx("Resources/Fonts/Ac437_TridentEarly_8x8.ttf", 32, 0, 252);
    uint APP_Y_SIZE = tl_init_grid(APP_X_SIZE, SCREENSIZE_X, SCREENSIZE_Y, (Color){20, 40, 29, 255}, &square_font);

    UIContext_t *ui_cntxt = ascui_context_create();

    UIBox_t *test_ui_box = ascui_ui_box_create(UI_ACTION_LIST, pos(17,3), 10, 10);
    ascui_ui_box_set_style(test_ui_box, RAYWHITE, GRAY, DARKGRAY, '-', '|', '+');

    UIBox_t *test_2_ui_box = ascui_ui_box_create(UI_ACTION_LIST, pos(0,0), 25, APP_Y_SIZE-5);
    ascui_ui_box_set_style(test_2_ui_box, RAYWHITE, GRAY, DARKGRAY, '=', 'I', 'O');

    // ascui_ui_context_add_box(ui_cntxt, test_ui_box);
    ascui_ui_context_add_box(ui_cntxt, test_2_ui_box);

    UIElement_t *test_1_ui_element = ascui_ui_element_create(UI_TEXT, NULL, NULL, "> HEMLO!\n | +1 sad");
    ascui_ui_element_set_style(test_1_ui_element, RAYWHITE, DARKBLUE);
    UIElement_t *test_2_ui_element = ascui_ui_element_create(UI_TEXT, NULL, NULL, "> Nir nor\n | +10 HP\n | +100 shame");
    ascui_ui_element_set_style(test_2_ui_element, RAYWHITE, DARKBLUE);
    UIElement_t *test_3_ui_element = ascui_ui_element_create(UI_TEXT, NULL, NULL, "> Eat gammloger pesto\n | -5 HP\n | +10 pride");
    ascui_ui_element_set_style(test_3_ui_element, RAYWHITE, DARKBLUE);
    UIElement_t *test_4_ui_element = ascui_ui_element_create(UI_TEXT, NULL, NULL, "> your daily subliminal message:\nyou want to play animal crossing. you love animal crossing");
    ascui_ui_element_set_style(test_4_ui_element, RAYWHITE, DARKBLUE);
    UIElement_t *test_5_ui_element = ascui_ui_element_create(UI_TEXT, NULL, NULL, "> Commit die\n  right now!\n | -100 HP");
    ascui_ui_element_set_style(test_5_ui_element, RAYWHITE, DARKBLUE);
    UIElement_t *test_6_ui_element = ascui_ui_element_create(UI_TEXT, NULL, NULL, "> Tengsoeda > Boda, tru facts\n | +100 fact");
    ascui_ui_element_set_style(test_6_ui_element, RAYWHITE, DARKBLUE);
    UIElement_t *test_7_ui_element = ascui_ui_element_create(UI_TEXT, NULL, NULL, "> Smell big fart\n | 5 HP\n | +2 vitality");
    ascui_ui_element_set_style(test_7_ui_element, RAYWHITE, DARKBLUE);
    UIElement_t *test_8_ui_element = ascui_ui_element_create(UI_TEXT, NULL, NULL, "> Do big crunch when drunk\n | +10 funni\n | +10 vitality\n | -2 friends");
    ascui_ui_element_set_style(test_8_ui_element, RAYWHITE, DARKBLUE);

    ascui_ui_box_add_element(test_2_ui_box, test_1_ui_element);
    ascui_ui_box_add_element(test_2_ui_box, test_2_ui_element);
    ascui_ui_box_add_element(test_2_ui_box, test_3_ui_element);
    ascui_ui_box_add_element(test_2_ui_box, test_4_ui_element);
    ascui_ui_box_add_element(test_2_ui_box, test_5_ui_element);
    ascui_ui_box_add_element(test_2_ui_box, test_6_ui_element);
    ascui_ui_box_add_element(test_2_ui_box, test_7_ui_element);
    ascui_ui_box_add_element(test_2_ui_box, test_8_ui_element);

    SetTargetFPS(60);
    double blink_var = 0.0f;
    int blink_alpha = 0;
    
    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_UP) && test_2_ui_box->element_selected != 0)
        {
            test_2_ui_box->element_selected--;
        }
        if (IsKeyPressed(KEY_DOWN) && test_2_ui_box->element_selected < test_2_ui_box->num_elements - 1)
        {
            test_2_ui_box->element_selected++;
        }
        

        blink_var++; 
        blink_alpha = ((sin((blink_var / 60.0f) * 2.0f * PI) + 1.0f) / 2.0f) * 255.0f;

        BeginDrawing();
            ClearBackground(DEF_COLOR);
            tl_draw_rect(0,0, APP_X_SIZE - 1, APP_Y_SIZE - 1, '.', BLACK, DEF_COLOR, &square_font);
            ascui_draw_to_grid(ui_cntxt);

            tl_render_grid();
            DrawFPS(0,0);
        EndDrawing();

        if ((int)blink_var % 60 == 0) blink_var = 0;
    }

    ascui_ui_context_destroy(&ui_cntxt);
    UnloadFont(square_font);
    tl_deinit_grid();
    CloseWindow();
    return 0;
}