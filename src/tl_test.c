#include <raylib.h>
#include "raytiles.h"
#include <string.h>
#include <stdio.h>

#define DEF_COLOR (Color){20, 40, 29, 255}

// main SCREENSIZE_X SCREENSIZE_Y
int main(int argc, char *argv[]){

    if(argc > 3 || argc < 3) {
        printf("Parameters: screensize_x screensize_y\n");
        return 0;
    }

    int screensize_x = atoi(argv[1]);
    int screensize_y = atoi(argv[2]);

    InitWindow(screensize_x, screensize_y, "raytiles");
    Font square_font = LoadFontEx("Resources/Fonts/Ac437_TridentEarly_8x8.ttf", 32, 0, 252);
    SetTargetFPS(60);
    while (!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(DEF_COLOR);
        DrawFPS(0,0);
        EndDrawing();
    }
    UnloadFont(square_font);
    CloseWindow();
    return 0;
}