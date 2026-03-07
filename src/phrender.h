#pragma once
#include <raylib.h>
#include <stdint.h>
#include "common.h"

typedef Color col_t;
typedef struct
{
    Image       src_img;
    uint8_t     subtex_size;     // Size of each sub-texture in texture map - always square
    pos16_t     texmap_size;    // Dimensions in terms of sub-textures not pixels - not necessarily square
}texmap_t;

texmap_t ph_load_texmap(const char *filepath, uint8_t subtex_size);
void ph_unload_texmap(texmap_t tmap);

void ph_init(uint16_t tile_resolution);
void ph_deinit();

void ph_clear();
void ph_render();

#define PRESERVE_TILESIZE 0
#define PRESERVE_GRID_WIDTH 1
#define PRESERVE_GRID_HEIGHT 2
void ph_set_resize_mode(uint8_t resize_mode);
void ph_zoom(int16_t zoom_amount);

// Mutually exclusive, will overwrite eachother's effects
void ph_set_grid_width(uint16_t width);
void ph_set_grid_height(uint16_t height);
void ph_set_grid_tile_size(uint16_t tile_size);
void ph_set_tile_resolution(uint16_t tiles_res);
uint16_t ph_get_tile_resolution();

uint16_t ph_get_grid_width();
uint16_t ph_get_grid_height();
uint16_t ph_get_grid_tile_size();

void ph_check_resize();
void ph_set_global_offsets(int16_t x_off, int16_t y_off);
int16_t *ph_get_global_offsets();

void ph_place_texture(texmap_t tmap, pos16_t tpos, pos16_t pos, col_t col);
void ph_place_textureX(texmap_t tmap, pos16_t tpos, pos16_t pos, col_t col, int16_t x_p_offset, int16_t y_p_offset);
void ph_place_textureF(texmap_t tmap, pos16_t tpos, float x, float y, col_t col);

void ph_paint(pos16_t p0, pos16_t p1, col_t col);
void ph_paintX(pos16_t p0, pos16_t p1, col_t col, int16_t x_p_offset, int16_t y_p_offset);
void ph_paintF(float x, float y, float w, float h, col_t col);
void ph_paintPF(pos16_t p0, float width, float height, col_t col);

#define ph_plot(p, col) ph_paint(p, p, col)
