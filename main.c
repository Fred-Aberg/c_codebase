#include "raylib.h"
#include "math.h"
#include "stdlib.h"
#include "stdio.h"
#include "minesweeper.h"
#include "raytiles.h"
#include "string.h"
#include "common.h"

const int SCREENSIZE_X = 1000;
const int SCREENSIZE_Y = 650;
const int GAME_RES_X = 50; // _Y is calculated
const int APP_RES_X = GAME_RES_X + 1; // two border lines, _Y is calculated
const int UI_HEIGHT = 3;
const int APP_X_SIZE = APP_RES_X + 1;

#define DEF_COLOR (Color){20, 40, 29, 255}

typedef struct Cursor 
{
    uint x;
    uint y;
    uint y_l_bnds;
    uint y_h_bnds;
    uint x_l_bnds;
    uint x_h_bnds;
} Cursor_t;

void clamp_cursor(Cursor_t *crs)
{
    crs->x = clamp(crs->x_l_bnds, crs->x, crs->x_h_bnds);
    crs->y = clamp(crs->y_l_bnds, crs->y, crs->y_h_bnds);
}

typedef enum Gamestate 
{
    GAME,
    GAME_OVER,
    GAME_WON
} Gamestate_e;

bool handle_inputs(Minefield_t *mf, Cursor_t *cursor)
{
    int key = GetKeyPressed();
    switch (key)
    {
    case KEY_DOWN:
        cursor->y++;
        break;
    case KEY_UP:
        cursor->y--;
        break;
    case KEY_LEFT:
        cursor->x--;
        break;
    case KEY_RIGHT:
        cursor->x++;
        break;
    case KEY_SPACE:
        return traverse(mf, cursor->x, cursor->y);
    case KEY_RIGHT_SHIFT:
        return traverse(mf, cursor->x, cursor->y);
    case KEY_LEFT_CONTROL:
        mark_mine(mf, cursor->x, cursor->y);
        break;
    case KEY_SLASH: // OBS: NORDIC KEYBOARD ONLY
        mark_mine(mf, cursor->x, cursor->y);
        break;
    default:
        break;
    }

    return false;
}

char *int_to_str(int a)
{
    int length = snprintf(NULL, 0,"%d", a);
    char *str = malloc((length + 1) * sizeof(char));

    sprintf(str, "%d", a);
    return str;
}

Color num_to_color(int num)
{
    switch (num)
    {
    case 1:
        return (Color){0, 255, 0,255};
    case 2:
        return (Color){0, 200, 0,255};
    case 3:
        return (Color){50, 150, 0,255};
    case 4:
        return (Color){100, 100, 0,255};
    case 5:
        return (Color){150, 50, 0,255};
    case 6:
        return (Color){200, 25, 0,255};
    case 7:
        return (Color){150, 0, 0,255};
    case 8:
        return (Color){100, 0, 50,255};
    default:
        return BLACK;
    }
}

char num_to_char(short num)
{
    return (char)(num + 48);
}

void grid_draw_minefield(Minefield_t *mf, Gamestate_e game_state, uint x_offset, uint y_offset, Font *font)
{
    #define CELL_COVERED (cell.state == COVERED && game_state == GAME)
    #define CELL_REVEALED (cell.state == REVEALED || game_state != GAME)

    for (uint y = 0; y < mf->size_y; y++)
    {        
        for (uint x = 0; x < mf->size_x; x++)
        {
            Cell_t cell = *get_cell(mf, x, y);

            if (cell.state == FLAGGED && game_state == GAME)
            {   
                tl_draw_tile(x + x_offset, y + y_offset, 'P', RED, DARKGRAY, font);
            } 
            else if (CELL_REVEALED && !cell.mined)
            {
                if (cell.num == 0)
                {
                    tl_draw_tile(x + x_offset, y + y_offset, '.', GRAY, NO_BG, font);
                }
                else
                {
                    tl_draw_tile(x + x_offset, y + y_offset, num_to_char(cell.num), num_to_color(cell.num), NO_BG, font);
                }
            }
            else if (CELL_COVERED)
            {
                tl_draw_tile(x + x_offset, y + y_offset, NO_SMBL, NO_BG, DARKGRAY, font);
            }
            else if (CELL_REVEALED && cell.mined)
            {
                tl_draw_tile(x + x_offset, y + y_offset, '@', RED, NO_BG, font);
            }
        }
    }
}


int main(void)
{
    /// SETUP
    bool init = true;

    Gamestate_e game_state = GAME;

    InitWindow(SCREENSIZE_X, SCREENSIZE_Y, "Minesmeeper 2023");

    Font square_font = LoadFontEx("Resources/Fonts/Ac437_TridentEarly_8x8.ttf", 32, 0, 252);
    uint APP_Y_SIZE = tl_init_grid(APP_X_SIZE, SCREENSIZE_X, SCREENSIZE_Y, (Color){20, 40, 29, 255}, &square_font);
    uint APP_RES_Y = APP_Y_SIZE - 1;

    //                max_res - UI  - lines
    uint GAME_RES_Y = APP_RES_Y - UI_HEIGHT - 3;
    printf("\nNew Minefield: %d x %d\n", GAME_RES_X, GAME_RES_Y);
    Minefield_t *mf = minefield_create(GAME_RES_X, GAME_RES_Y, 10);
    // print_minefield(mf);
    Cursor_t cursor;
    cursor.x = 10;
    cursor.y = 10;
    cursor.x_h_bnds = GAME_RES_X - 1;
    cursor.x_l_bnds = 0;
    cursor.y_h_bnds = GAME_RES_Y - 1;
    cursor.y_l_bnds = 0;

    SetTargetFPS(60);
    double blink_var = 0.0f;
    int blink_alpha = 0;
    
    while (!WindowShouldClose())
    {
        if(game_state == GAME)
        {
            if(game_won(mf)) game_state = GAME_WON;
            else if(handle_inputs(mf, &cursor)) game_state = GAME_OVER;
        }
        clamp_cursor(&cursor);


        blink_var++; 
        blink_alpha = ((sin((blink_var / 60.0f) * 2.0f * PI) + 1.0f) / 2.0f) * 255.0f;

        BeginDrawing();
            ClearBackground(DEF_COLOR);

            tl_draw_rect(0, 0, APP_RES_X, APP_RES_Y - GAME_RES_Y, NO_SMBL, NO_BG, DEF_COLOR, &square_font);

            tl_draw_line(0,0, APP_RES_X, 0, '-', DARKGREEN, GREEN, &square_font);
            tl_draw_line(APP_RES_X, 0, APP_RES_X, APP_RES_Y, '|', DARKGREEN, GREEN, &square_font);
            tl_draw_line(APP_RES_X, APP_RES_Y, 0, APP_RES_Y, '-', DARKGREEN, GREEN, &square_font);
            tl_draw_line(0,APP_RES_Y, 0, 0, '|', DARKGREEN, GREEN, &square_font);


            tl_draw_tile(0,0, '+', DARKGREEN, GREEN, &square_font);
            tl_draw_tile(APP_RES_X,0, '+', DARKGREEN, GREEN, &square_font);
            tl_draw_tile(APP_RES_X, APP_RES_Y, '+', DARKGREEN, GREEN, &square_font);
            tl_draw_tile(0,APP_RES_Y, '+', DARKGREEN, GREEN, &square_font);


            tl_draw_line(0, GAME_RES_Y + 1, GAME_RES_X + 1, GAME_RES_Y + 1, '=', DARKGREEN, GREEN, &square_font);
            tl_draw_tile(0,GAME_RES_Y + 1, '+', DARKGREEN, GREEN, &square_font);
            tl_draw_tile(GAME_RES_X + 1, GAME_RES_Y + 1, '+', DARKGREEN, GREEN, &square_font);

            grid_draw_minefield(mf, game_state,1,1, &square_font);

            char *over_txt = "-GAME--\n-OVER!-";
            char *won_txt = "-GAME-\n-WON!--";
            if (game_state == GAME_OVER)
                tl_draw_text(abs(GAME_RES_X / 2 - strlen(over_txt)/2), GAME_RES_Y/2 + 1, APP_RES_X - 1, over_txt, strlen(over_txt), RED, (Color){blink_alpha, 0, 0, 255}, &square_font);
            else if (game_state == GAME_WON)
                tl_draw_text(abs(GAME_RES_X / 2 - strlen(won_txt)/2), GAME_RES_Y/2 + 1, APP_RES_X - 1 , won_txt, strlen(won_txt), GREEN, (Color){0, blink_alpha, 0, 255}, &square_font);

            tl_set_tile_bg(cursor.x + 1, cursor.y + 1, (Color){blink_alpha,blink_alpha,blink_alpha, 255});

            char *num_str = int_to_str(round(cursor.x));
            tl_draw_text(1, APP_RES_Y - 4, 30, "X: ", strlen("X: "), DARKGREEN, NO_BG, &square_font);
            tl_draw_text(1 + strlen("X: "), APP_RES_Y - 4, 30, num_str, strlen(num_str), DARKGREEN, NO_BG, &square_font);
            free(num_str);

            num_str = int_to_str(round(cursor.y));
            tl_draw_text(1, APP_RES_Y - 3, 30, "Y: ", strlen("Y: "), DARKGREEN, NO_BG, &square_font);
            tl_draw_text(1 + strlen("X: "), APP_RES_Y - 3, 30, num_str, strlen(num_str), DARKGREEN, NO_BG, &square_font);
            free(num_str);
            
            tl_render_grid();
            DrawFPS(0,0);
        EndDrawing();

        if ((int)blink_var % 60 == 0) blink_var = 0;
    }

    UnloadFont(square_font);
    tl_deinit_grid();
    CloseWindow();
    minefield_destroy(&mf);
    return 0;
}
