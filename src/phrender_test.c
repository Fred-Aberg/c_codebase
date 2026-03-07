#include "common.h"
#include "phrender.c"
#include "phrender.h"
#include <math.h>
#include <raylib.h>
#include <stdint.h>

int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1000, 500, "Phrender test");
    ph_init(10);

    ph_set_resize_mode(PRESERVE_TILESIZE);

    texmap_t tmap = ph_load_texmap("Resources/Fonts/font_base.png", 10);

    while (!WindowShouldClose()) {
        ph_check_resize();
        if (GetMouseWheelMove() != 0) {
            ph_zoom(ceilf(GetMouseWheelMove()));
        }

        double t = GetTime();
        BeginDrawing();
        ph_clear();

        pos16_t p_ = pos16(ph_get_grid_width() / 2, ph_get_grid_height() / 2);
        pos16_t p0 = pos16(p_.x + sin(t) * 5, p_.y + cos(t) * 5);
        pos16_t p1 = pos16(p0.x + 10, p0.y + 10);


        uint8_t r = MAX(255 * (sin(t) + 1) / 2, 100);
        uint8_t g = MAX(255 * (cos(4.6f*t) + 1) / 2, 100);
        uint8_t b = MAX(255 * (sin(2.3f*t - PI / 2) + 1) / 2, 100);

        ph_paint(p0, p1, c(r,g,b));
        ph_paint(p0, p0, GREEN);
        ph_paint(p_, p_, BLACK);
        ph_paint(pos16(0, 0), pos16(0, 0), RED);
        ph_paint(pos16(ph_get_grid_width() - 1, ph_get_grid_height() - 1),
            pos16(ph_get_grid_width() - 1, ph_get_grid_height() - 1), RED);

        ph_place_texture(tmap, pos16(0, 3), pos16(2, 2), c(r,g,b));
        ph_place_textureX(tmap, pos16(0, 3), pos16(2, 2), c(b,r,g), 10 * sin(t), 10 * cos(t));
        ph_render();

        DrawFPS(0, 0);

        EndDrawing();
    }

    ph_deinit();
    ph_unload_texmap(tmap);
    return 0;
}
