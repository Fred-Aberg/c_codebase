#include "phrender.h"
#include "common.h"
#include <assert.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct
{
    uint8_t  resize_mode;
    int16_t  gl_pxl_offset_x;
    int16_t  gl_pxl_offset_y;
    uint16_t width;
    uint16_t height;
    uint16_t tile_pxl_size;
    uint16_t tile_resolution;
    Image       rndr_img;   // CPU Texture
    Texture2D   rndr_tex;   // GPU Texture
}ph_grid_t;

ph_grid_t *G;

texmap_t ph_load_texmap(const char *filepath, uint8_t subtex_size)
{
    Image img = LoadImage(filepath);
    return (texmap_t){.src_img = img, .subtex_size = subtex_size, .texmap_size = pos16(img.width / subtex_size, img.height / subtex_size)};
}

void ph_unload_texmap(texmap_t tmap)
{
    UnloadImage(tmap.src_img);
}

void ph_init(uint16_t tile_resolution)
{
    G = calloc(1, sizeof(ph_grid_t));
    G->tile_resolution = tile_resolution;

    // Defaults
    G->rndr_img = GenImageColor(1, 1, BLACK);
    ph_set_resize_mode(PRESERVE_GRID_WIDTH);
    ph_set_grid_width(32);
}

void ph_deinit()
{
    free(G);
}

void resize_rndr_img()
{
    ImageResize(&G->rndr_img, G->width * G->tile_resolution, G->height * G->tile_resolution);
}

void ph_set_tile_resolution(uint16_t tiles_res)
{
    G->tile_resolution = tiles_res;
}
uint16_t ph_get_tile_resolution()
{
    return G->tile_resolution;
}
void ph_clear()
{
    ImageClearBackground(&G->rndr_img, BLACK);
}

void ph_render()
{
    static bool first_pass = true;

    if(first_pass)
        first_pass = false;
    else
        UnloadTexture(G->rndr_tex);

    G->rndr_tex = LoadTextureFromImage(G->rndr_img);
    assert(IsTextureValid(G->rndr_tex));
    Rectangle srcRec = { 0, 0, G->rndr_tex.width, G->rndr_tex.height };
    Rectangle dstRec = { 0, 0, GetScreenWidth(), GetScreenHeight() };
    // GenTextureMipmaps(&G->rndr_tex);
    // SetTextureFilter(G->rndr_tex, TEXTURE_FILTER_BILINEAR);
    DrawTexturePro(G->rndr_tex, srcRec, dstRec, (Vector2){0,0}, 0, WHITE);
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
    resize_rndr_img();
}
void ph_set_grid_height(uint16_t height)
{
    G->height =         height;
    G->tile_pxl_size =  GetScreenHeight() / height;
    G->width =          GetScreenWidth() / G->tile_pxl_size;
    resize_rndr_img();
}
void ph_set_grid_tile_size(uint16_t tile_size)
{
    G->tile_pxl_size =  tile_size;
    G->width =          GetScreenWidth() / G->tile_pxl_size;
    G->height =         GetScreenHeight() / G->tile_pxl_size;
    resize_rndr_img();
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

    resize_rndr_img();
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


void ph_set_global_offsets(int16_t x_off, int16_t y_off)
{
    G->gl_pxl_offset_x = x_off;
    G->gl_pxl_offset_y = y_off;
}

int16_t *ph_get_global_offsets()
{
    return &G->gl_pxl_offset_x;
}

void ph_place_textureF(texmap_t tmap, pos16_t tpos, float x, float y, col_t col)
{
    Rectangle srcRec = { tpos.x * tmap.subtex_size, tpos.y * tmap.subtex_size, tmap.subtex_size, tmap.subtex_size };
    Rectangle dstRec = {
        (G->tile_resolution * x) + G->gl_pxl_offset_x,
        (G->tile_resolution * y) + G->gl_pxl_offset_y,
        G->tile_resolution,
        G->tile_resolution
    };
    ImageDraw(&G->rndr_img, tmap.src_img, srcRec, dstRec, col);
}

void ph_place_textureX(texmap_t tmap, pos16_t tpos, pos16_t pos, col_t col, int16_t x_p_offset, int16_t y_p_offset)
{
    Rectangle srcRec = { tpos.x * tmap.subtex_size, tpos.y * tmap.subtex_size, tmap.subtex_size, tmap.subtex_size };
    Rectangle dstRec = {
        (G->tile_resolution * pos.x) + x_p_offset + G->gl_pxl_offset_x,
        (G->tile_resolution * pos.y) + y_p_offset + G->gl_pxl_offset_y,
        G->tile_resolution,
        G->tile_resolution };
    ImageDraw(&G->rndr_img, tmap.src_img, srcRec, dstRec, col);
}
void ph_place_texture(texmap_t tmap, pos16_t tpos, pos16_t pos, col_t col) { ph_place_textureX( tmap,  tpos,  pos,  col,  0, 0); }

void ph_paintX(pos16_t p0, pos16_t p1, col_t col, int16_t x_p_offset, int16_t y_p_offset)
{

    ImageDrawRectangle(&G->rndr_img,
        (G->tile_resolution * p0.x) + x_p_offset + G->gl_pxl_offset_x,
        (G->tile_resolution * p0.y) + y_p_offset + G->gl_pxl_offset_y,
        G->tile_resolution * (p1.x - p0.x + 1),
		G->tile_resolution * (p1.y - p0.y + 1), col);
}
void ph_paint(pos16_t p0, pos16_t p1, col_t col) { ph_paintX( p0,  p1,  col, 0, 0); }
void ph_paintF(float x, float y, float w, float h, col_t col)
{
    ImageDrawRectangle(&G->rndr_img,
        (G->tile_resolution * x) + G->gl_pxl_offset_x,
        (G->tile_resolution * y) + G->gl_pxl_offset_y,
        G->tile_resolution * w,
		G->tile_resolution * h, col);
}
