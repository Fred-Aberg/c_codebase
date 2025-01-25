#include "minesweeper.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

void lay_mines(Minefield_t *mf, int mine_density);
void calculate_cell_nums(Minefield_t *mf);

/// Top level functions
Minefield_t *minefield_create(uint size_x, uint size_y, int mine_density)
{
    Minefield_t *mf = calloc(1, sizeof(Minefield_t));
    mf->size_x = size_x;
    mf->size_y = size_y;
    mf->init = true;
    mf->cells = calloc(size_x * size_y, sizeof(Cell_t));
    printf("\nminefield created:\n    size: %d x %d -> %d cells\n   coords: (0-%d, 0-%d)", mf->size_x, mf->size_y, mf->size_x * mf->size_y, mf->size_x - 1, mf->size_y - 1);

    lay_mines(mf, mine_density);

    return mf;
}

// Convert array index of cell to cell's coordinates
static void i_to_pos(int i, uint width, uint *x, uint *y)
{
    *y = i / width; // C autimatically "floors" the result 
    *x = i - (*y * width);
}

Cell_t *get_cell(Minefield_t *mf, uint x, uint y)
{
    assert(x < mf->size_x && y < mf->size_y); // kys
    return &mf->cells[x + y * mf->size_x];
}

void minefield_destroy(Minefield_t **mf)
{
    free((*mf)->cells);
    free(*mf);
    *mf = NULL;
}

void mark_mine(Minefield_t *mf, uint x, uint y)
{
    Cell_t *cell = get_cell(mf, x, y);

    // Only flag if it is not already revealed
    if (cell->state == COVERED) cell->state = FLAGGED;
    else if (cell->state == FLAGGED) cell->state = COVERED;
}

void cascade (Minefield_t *mf, uint x, uint y);
int change_mine_count(Minefield_t *mf, uint x, uint y, int change);
static void remove_inital_mines(Minefield_t *mf, uint x, uint y);
bool traverse(Minefield_t *mf, uint x, uint y)
{
    Cell_t *cell = get_cell(mf, x, y);

    // Do not allow traversal of flagged cells (suicide)
    if (cell->state == FLAGGED) return false;

    // Starting cell is never a mine.
    if (mf->init)
    {
        if (cell->mined)
            change_mine_count(mf, x, y, -1);
        cell->mined = false;

        // Remove any neighbours with mines
        remove_inital_mines(mf, x, y);

        mf->init = false;
    }

    if (cell->mined) 
    {
        reveal_field(mf);
        return true; // GG
    }
    
    cascade(mf, x, y);
    return false;
}

void reveal_field(Minefield_t *mf) 
{
    uint size = mf->size_x * mf->size_y;
    for (uint i = 0; i < size; i++)
    {
        mf->cells[i].state = REVEALED;
    }
}

bool game_won(Minefield_t *mf)
{
    /// TODO: Keep track of cells uncovered instead of looping
    uint size = mf->size_x * mf->size_y;
    Cell_t cell; 
    for (uint i = 0; i < size; i++)
    {
        cell = mf->cells[i]; 
        if(cell.state == COVERED && !cell.mined) return false;
    }
    return true;
}

/// Helper functions
#define MIN(a, b) (a < b)? a : b
int change_mine_count(Minefield_t *mf, uint x, uint y, int change)
{
    #define inc_num(x, y) do {Cell_t * cell = get_cell(mf, x, y); cell->num +=  change;} while(0) 

    bool has_w_ngbr = x != 0;
    bool has_e_ngbr = x != mf->size_x - 1;
    bool has_n_ngbr = y != mf->size_y - 1;
    bool has_s_ngbr = y != 0;

    if (has_w_ngbr) inc_num(x - 1, y);                      // W
    if (has_e_ngbr) inc_num(x + 1, y);                      // E
    if (has_n_ngbr) inc_num(x, y + 1);                      // N
    if (has_s_ngbr) inc_num(x, y - 1);                      // S
    if (has_n_ngbr && has_w_ngbr) inc_num(x - 1, y + 1);    // NW
    if (has_n_ngbr && has_e_ngbr) inc_num(x + 1, y + 1);    // NE
    if (has_s_ngbr && has_w_ngbr) inc_num(x - 1, y - 1);    // SW
    if (has_s_ngbr && has_e_ngbr) inc_num(x + 1, y - 1);    // SE
}

static void demine(Minefield_t *mf, uint x, uint y)
{
    Cell_t *cell = get_cell(mf, x, y);
    cell->mined = false;
}

static bool is_mined(Minefield_t *mf, uint x, uint y)
{
    Cell_t *cell = get_cell(mf, x, y);
    return cell->mined;
}

static void remove_inital_mines(Minefield_t *mf, uint x, uint y)
{

    bool has_w_ngbr = x != 0;
    bool has_e_ngbr = x != mf->size_x - 1;
    bool has_n_ngbr = y != mf->size_y - 1;
    bool has_s_ngbr = y != 0;

    if (has_w_ngbr && is_mined(mf, x - 1, y)) {demine(mf, x - 1, y); change_mine_count(mf, x - 1, y, -1);}                       // W
    if (has_e_ngbr && is_mined(mf, x + 1, y)) {demine(mf, x + 1, y); change_mine_count(mf, x + 1, y, -1);}                       // E
    if (has_n_ngbr && is_mined(mf, x, y + 1)) {demine(mf, x, y + 1); change_mine_count(mf, x, y + 1, -1);}                       // N
    if (has_s_ngbr && is_mined(mf, x, y - 1)) {demine(mf, x, y - 1); change_mine_count(mf, x, y - 1, -1);}                       // S
    if (has_n_ngbr && has_w_ngbr && is_mined(mf, x - 1, y + 1)) {demine(mf, x - 1, y + 1); change_mine_count(mf, x - 1, y + 1, -1);}     // NW
    if (has_n_ngbr && has_e_ngbr && is_mined(mf, x + 1, y + 1)) {demine(mf, x + 1, y + 1); change_mine_count(mf, x + 1, y + 1, -1);}     // NE
    if (has_s_ngbr && has_w_ngbr && is_mined(mf, x - 1, y - 1)) {demine(mf, x - 1, y - 1); change_mine_count(mf, x - 1, y - 1, -1);}     // SW
    if (has_s_ngbr && has_e_ngbr && is_mined(mf, x + 1, y - 1)) {demine(mf, x + 1, y - 1); change_mine_count(mf, x + 1, y - 1, -1);}     // SE
}

void lay_mines(Minefield_t *mf, int mine_density)
{
    srand(time(0));
    uint size = mf->size_x * mf->size_y;
    Cell_t *cell; 
    int x = 0;
    int y = 0;
    for (uint i = 0; i < size; i++)
    {
        cell = &mf->cells[i]; 
        cell->state = COVERED;

        int rnd = (rand() % 101);
        if (rnd >= mine_density) continue;
        
        cell->mined = true;
        // Both cell.flagged & cell.revealed should be false due to calloc

        /// TODO: Make sure 3x3s of mines don't appear
        i_to_pos(i, mf->size_x, &x, &y);
        // printf("\n%d -> (%d, %d)", i, x, y);
        change_mine_count(mf, x, y, 1);
    }
}

void cascade (Minefield_t *mf, uint x, uint y)
{
    Cell_t *cell = get_cell(mf, x, y);

    if(cell->state != COVERED) return;

    // Reveal cell
    cell->state = REVEALED;

    if(cell->num != 0) return;
    
    // Cell cascades further
    bool has_w_ngbr = x != 0;
    bool has_e_ngbr = x != mf->size_x - 1;
    bool has_n_ngbr = y != mf->size_y - 1;
    bool has_s_ngbr = y != 0;

    if (has_w_ngbr) cascade(mf, x - 1, y);
    if (has_e_ngbr) cascade(mf, x + 1, y);
    if (has_n_ngbr) cascade(mf, x, y + 1);
    if (has_s_ngbr) cascade(mf, x, y - 1);
    if (has_n_ngbr && has_w_ngbr) cascade(mf, x - 1, y + 1);
    if (has_n_ngbr && has_e_ngbr) cascade(mf, x + 1, y + 1);
    if (has_s_ngbr && has_w_ngbr) cascade(mf, x - 1, y - 1);
    if (has_s_ngbr && has_e_ngbr) cascade(mf, x + 1, y - 1);
}

void print_minefield(Minefield_t *mf)
{
    uint size = mf->size_x * mf->size_y;
    Cell_t cell; 
    for (uint i = 0; i < size; i++)
    {
        if(i % mf->size_x == 0) printf("\n");

        cell = mf->cells[i]; 
        if(cell.mined)  printf("Ó");
        else            printf("%d", cell.num);
    }
    printf("\n");
}