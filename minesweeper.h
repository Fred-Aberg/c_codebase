#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "common.h"


typedef enum
{
    FLAGGED,
    COVERED,
    REVEALED
}CellState_e;

typedef struct 
{
    bool mined;
    CellState_e state;
    short num;
} Cell_t;

typedef struct 
{
    bool init;
    uint size_x;
    uint size_y;
    Cell_t *cells;
} Minefield_t;

/// @brief Create and generate an initialized minefield
/// @param size_x
/// @param size_y
/// @param mine_density % of cells that are mines
/// @param return Allocated minefield ptr
/// @param x starting cell x
/// @param y starting cell y
Minefield_t *minefield_create(uint size_x, uint size_y, int mine_density);

Cell_t *get_cell(Minefield_t *mf, uint x, uint y);

/// @brief Free minefield struct
/// @param mf (double ptr), NULLs mf
void minefield_destroy(Minefield_t **mf);

/// @brief Mark/unmark a spot as a mine
/// @param mf 
/// @param x 
/// @param y 
void mark_mine(Minefield_t *mf, uint x, uint y);

/// @brief Mark a spot as safe
/// @param mf 
/// @param x 
/// @param y 
/// @return true if mine, false if not
bool traverse(Minefield_t *mf, uint x, uint y);

/// @brief Reveal the entire minefield
/// @param mf 
void reveal_field(Minefield_t *mf);

bool game_won(Minefield_t *mf);

void print_minefield(Minefield_t *mf);