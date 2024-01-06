#include "raylib.h"
#include "math.h"
#include "stdlib.h"
#include "stdio.h"
#include "minesweeper.h"
#include "raytiles.h"
#include "string.h"

const int SCREENSIZE_X = 1000;
const int SCREENSIZE_Y = 650;
const int GAME_RES_X = 50; // _Y is calculated
const int APP_RES_X = GAME_RES_X + 1; // two border lines, _Y is calculated
const int UI_HEIGHT = 3;
const int APP_X_SIZE = APP_RES_X + 1;



#define DEF_COLOR (Color){20, 40, 29, 255}

#define MIN(a, b) (a < b)? a : b
#define MAX(a, b) (a < b)? b : a









int main(void)
{
    /// SETUP
    InitWindow(SCREENSIZE_X, SCREENSIZE_Y, "ascUI 2024");

    Font square_font = LoadFontEx("Resources/Fonts/Ac437_TridentEarly_8x8.ttf", 32, 0, 252);
    uint APP_Y_SIZE = tl_init_grid(APP_X_SIZE, SCREENSIZE_X, SCREENSIZE_Y, (Color){20, 40, 29, 255}, &square_font);

    SetTargetFPS(60);
    double blink_var = 0.0f;
    int blink_alpha = 0;
    
    while (!WindowShouldClose())
    {
        blink_var++; 
        blink_alpha = ((sin((blink_var / 60.0f) * 2.0f * PI) + 1.0f) / 2.0f) * 255.0f;

        BeginDrawing();
            ClearBackground(DEF_COLOR);
            DrawFPS(0,0);
        EndDrawing();

        if ((int)blink_var % 60 == 0) blink_var = 0;
    }

    UnloadFont(square_font);
    tl_deinit_grid();
    CloseWindow();
    return 0;
}