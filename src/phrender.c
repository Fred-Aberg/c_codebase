#include "phrender.h"
#include "common.h"
#include <assert.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    uint8_t  resize_mode;
    int16_t  gl_pxl_offset_x;
    int16_t  gl_pxl_offset_y;
    uint16_t width;
    uint16_t height;
    float    pxl_scgr_ratio;    // Screen pxl compared to "game" pxl
    uint16_t tile_size;         // Tile size in "game" pxls
    float    pxl_adj_tile_size; // Tile size in screen pxls (with exact decimals)

}ph_grid_t;

ph_grid_t *G;

texmap_t ph_load_texmap(const char *filepath, uint8_t subtex_size){
    Texture2D tex = LoadTexture(filepath);
    return (texmap_t){.src_tex = tex, .subtex_size = subtex_size, .texmap_size = pos16(tex.width / subtex_size, tex.height / subtex_size)};
}

void ph_unload_texmap(texmap_t tmap){
    UnloadTexture(tmap.src_tex);
}

void ph_init(uint16_t tile_resolution){
    G = calloc(1, sizeof(ph_grid_t));
    G->tile_size = tile_resolution;

    // Defaults
    ph_set_resize_mode(PRESERVE_GRID_WIDTH);
    ph_set_grid_width(32);
}

void ph_deinit(){
    free(G);
}

void ph_set_tile_resolution(uint16_t tiles_res){
    G->tile_size = tiles_res;
}
uint16_t ph_get_tile_resolution(){
    return G->tile_size;
}

void ph_set_resize_mode(uint8_t resize_mode){
    G->resize_mode = resize_mode;
}

// Mutually exclusive, will overwrite eachother's effects
void ph_set_grid_width(uint16_t width){
    G->width =              width;
    G->pxl_scgr_ratio =     (float)GetScreenWidth() / (width*G->tile_size);
    G->height =             GetScreenHeight() / (G->tile_size*G->pxl_scgr_ratio);
    G->pxl_adj_tile_size = G->tile_size*G->pxl_scgr_ratio;
}
void ph_set_grid_height(uint16_t height){
    G->height =            height;
    G->pxl_scgr_ratio =    (float)GetScreenHeight() / (height*G->tile_size);
    G->width =             GetScreenWidth() / (G->tile_size*G->pxl_scgr_ratio);
    G->pxl_adj_tile_size = G->tile_size*G->pxl_scgr_ratio;
}

uint16_t ph_get_grid_width() { return G->width; }
uint16_t ph_get_grid_height() { return G->height; }
uint16_t ph_get_grid_tile_size() { return G->tile_size; }
uint16_t ph_get_grid_pxl_width() { return G->width*G->tile_size; }
uint16_t ph_get_grid_pxl_height() { return G->height*G->tile_size; }

void ph_check_resize(){
    if(IsWindowResized() == false)
        return;

    // Trigger recalculation
    if (G->resize_mode == PRESERVE_GRID_WIDTH)
        ph_set_grid_width(G->width);
    else if (G->resize_mode == PRESERVE_GRID_HEIGHT)
        ph_set_grid_height(G->height);
}
void ph_zoom(int16_t zoom_amount){
    if (G->resize_mode == PRESERVE_GRID_WIDTH)
        ph_set_grid_width(MAX(1, G->width + zoom_amount));
    else if (G->resize_mode == PRESERVE_GRID_HEIGHT)
        ph_set_grid_height(MAX(1, G->width + zoom_amount));
}


void ph_set_global_offsets(int16_t x_off, int16_t y_off){
    G->gl_pxl_offset_x = x_off;
    G->gl_pxl_offset_y = y_off;
}

int16_t *ph_get_global_offsets(){
    return &G->gl_pxl_offset_x;
}


void ph_place_textureP(texmap_t tmap, pos16_t tpos, int16_t x, int16_t y, col_t col){
    Rectangle srcRec = { tpos.x * tmap.subtex_size, tpos.y * tmap.subtex_size, tmap.subtex_size, tmap.subtex_size };
    Rectangle dstRec = {
        (x + G->gl_pxl_offset_x)*G->pxl_scgr_ratio,
        (y + G->gl_pxl_offset_y)*G->pxl_scgr_ratio,
        tmap.subtex_size*G->pxl_scgr_ratio,
        tmap.subtex_size*G->pxl_scgr_ratio
    };
    DrawTexturePro(tmap.src_tex, srcRec, dstRec, (Vector2){0,0} , 0.0f, col);
}

void ph_place_textureX(texmap_t tmap, pos16_t tpos, pos16_t pos, col_t col, int16_t x_p_offset, int16_t y_p_offset)
{

    Rectangle srcRec = { tpos.x * tmap.subtex_size, tpos.y * tmap.subtex_size, tmap.subtex_size, tmap.subtex_size };
    Rectangle dstRec = {
        ((G->tile_size * pos.x) + x_p_offset + G->gl_pxl_offset_x)*G->pxl_scgr_ratio,
        ((G->tile_size * pos.y) + y_p_offset + G->gl_pxl_offset_y)*G->pxl_scgr_ratio,
        G->pxl_adj_tile_size,
        G->pxl_adj_tile_size };
    DrawTexturePro(tmap.src_tex, srcRec, dstRec, (Vector2){0,0} , 0.0f, col);
}
void ph_place_texture(texmap_t tmap, pos16_t tpos, pos16_t pos, col_t col) { ph_place_textureX( tmap,  tpos,  pos,  col,  0, 0); }

void ph_paintX(pos16_t p0, pos16_t p1, col_t col, int16_t x_p_offset, int16_t y_p_offset)
{
    DrawRectangle(
        ((G->tile_size * p0.x) + x_p_offset + G->gl_pxl_offset_x)*G->pxl_scgr_ratio,
        ((G->tile_size * p0.y) + y_p_offset + G->gl_pxl_offset_y)*G->pxl_scgr_ratio,
        G->pxl_adj_tile_size*(p1.x - p0.x + 1),
		G->pxl_adj_tile_size*(p1.y - p0.y + 1), col);
}
void ph_paintP(pos16_t p0, pos16_t p1, col_t col)
{
    DrawRectangle(
        (p0.x + G->gl_pxl_offset_x)*G->pxl_scgr_ratio,
        (p0.y + G->gl_pxl_offset_y)*G->pxl_scgr_ratio,
        (p1.x - p0.x + 1)*G->pxl_scgr_ratio,
		(p1.y - p0.y + 1)*G->pxl_scgr_ratio, col);
}
void ph_paint(pos16_t p0, pos16_t p1, col_t col) { ph_paintX( p0,  p1,  col, 0, 0); }
void ph_paintF(float x, float y, float w, float h, col_t col)
{
    DrawRectangle(
        G->pxl_adj_tile_size*(x + G->gl_pxl_offset_x),
        G->pxl_adj_tile_size*(y + G->gl_pxl_offset_y),
        G->pxl_adj_tile_size*w,
		G->pxl_adj_tile_size*h, col);
}

scpos_t ph_scpos_to_phpos(scpos_t scpos)
{
    return scpos(scpos.x/G->pxl_scgr_ratio, scpos.y/G->pxl_scgr_ratio);
}
float ph_get_scgr_ratio() { return G->pxl_scgr_ratio; }
