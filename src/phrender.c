#include "phrender.h"
#include "common.h"
#include <raylib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct
{
    uint8_t  resize_mode;
    pos16_t   pxl_offsets;
    uint16_t width;
    uint16_t height;
    uint16_t tile_pxl_size;
}ph_grid_t;

ph_grid_t *G;

texmap_t ph_load_texmap(const char *filepath, uint8_t subtex_size)
{
    Texture2D t = LoadTexture(filepath);
    return (texmap_t){.src_tex = t, .subtex_size = subtex_size, .texmap_size = pos16(t.width / subtex_size, t.height / subtex_size)};
}

void ph_unload_texmap(texmap_t tmap)
{
    UnloadTexture(tmap.src_tex);
}

void ph_init()
{
    G = calloc(1, sizeof(ph_grid_t));

    // Defaults
    ph_set_grid_width(64);
    ph_set_resize_mode(PRESERVE_GRID_WIDTH);
}

void ph_deinit()
{
    free(G);
}

void ph_set_resize_mode(uint8_t resize_mode)
{
    G->resize_mode = resize_mode;
}

// Mutually exclusive, will overwrite eachother's effects
void ph_set_grid_width(uint16_t width)
{
    G->width =          width;
    G->tile_pxl_size =  GetScreenWidth() / width;
    G->height =         GetScreenHeight() / G->tile_pxl_size;
}
void ph_set_grid_height(uint16_t height)
{
    G->height =         height;
    G->tile_pxl_size =  GetScreenHeight() / height;
    G->width =          GetScreenWidth() / G->tile_pxl_size;
}
void ph_set_grid_tile_size(uint16_t tile_size)
{
    G->tile_pxl_size =  tile_size;
    G->width =          GetScreenWidth() / G->tile_pxl_size;
    G->height =         GetScreenHeight() / G->tile_pxl_size;

}

uint16_t ph_get_grid_width() { return G->width; }
uint16_t ph_get_grid_height() { return G->height; }
uint16_t ph_get_grid_tile_size() { return G->tile_pxl_size; }

void ph_check_resize()
{
    if(IsWindowResized() == false)
        return;

    if (G->resize_mode == PRESERVE_TILESIZE)
    {
        G->width =  GetScreenWidth() / G->tile_pxl_size;
        G->height = GetScreenHeight() / G->tile_pxl_size;
    }
    else if (G->resize_mode == PRESERVE_GRID_WIDTH)
    {
        G->tile_pxl_size =  GetScreenWidth() / G->width;
        G->height =         GetScreenHeight() / G->tile_pxl_size;
    }
    else if (G->resize_mode == PRESERVE_GRID_HEIGHT)
    {
        G->tile_pxl_size =  GetScreenWidth() / G->height;
        G->height =         GetScreenWidth() / G->tile_pxl_size;
    }
}
void ph_zoom(int16_t zoom_amount)
{
    if (G->resize_mode == PRESERVE_TILESIZE)
        ph_set_grid_tile_size(MAX(1, G->tile_pxl_size + zoom_amount));
    else if (G->resize_mode == PRESERVE_GRID_WIDTH)
        ph_set_grid_width(MAX(1, G->width + zoom_amount));
    else if (G->resize_mode == PRESERVE_GRID_HEIGHT)
        ph_set_grid_height(MAX(1, G->width + zoom_amount));
}


void ph_set_offsets(pos16_t offsets)
{
    G->pxl_offsets = offsets;
}
void ph_center_grid()
{
    G->pxl_offsets.x = (GetScreenWidth() - (G->tile_pxl_size * G->width)) / 2;
    G->pxl_offsets.y = (GetScreenHeight() - (G->tile_pxl_size * G->height)) / 2;
}


void ph_place_textureX(texmap_t tmap, pos16_t tpos, pos16_t pos, col_t col, pos16_t pixel_offset)
{
    Rectangle srcRec = { tpos.x * tmap.subtex_size, tpos.y * tmap.subtex_size, tmap.subtex_size, tmap.subtex_size };
    Rectangle dstRec = {
        (G->tile_pxl_size * pos.x) + pixel_offset.x + G->pxl_offsets.x,
        (G->tile_pxl_size * pos.y) + pixel_offset.y + G->pxl_offsets.y,
        G->tile_pxl_size,
        G->tile_pxl_size };

    DrawTexturePro(tmap.src_tex, srcRec, dstRec, (Vector2){ 0, 0 }, 0.0f, col);
}
void ph_place_texture(texmap_t tmap, pos16_t tpos, pos16_t pos, col_t col) { ph_place_textureX( tmap,  tpos,  pos,  col,  pos16(0, 0)); }

void ph_paintX(pos16_t p0, pos16_t p1, col_t col, pos16_t pixel_offset)
{
    DrawRectangle(
        (G->tile_pxl_size * p0.x) + pixel_offset.x + G->pxl_offsets.x,
        (G->tile_pxl_size * p0.y) + pixel_offset.y + G->pxl_offsets.y,
        G->tile_pxl_size * (p1.x - p0.x + 1),
		G->tile_pxl_size * (p1.y - p0.y + 1), col);
}
void ph_paint(pos16_t p0, pos16_t p1, col_t col) { ph_paintX( p0,  p1,  col, pos16(0, 0)); }
