#pragma once
#include "stdlib.h"

typedef unsigned int uint;
typedef unsigned char uchar;

typedef struct
{
    uint x;
    uint y;
} Pos_t;

#define c(r, g, b) (Color){r, g, b, 255}
#define pos(x, y) (Pos_t){x, y}

uint umin(uint a, uint b);

uint umax(uint a, uint b);

uint uclamp(uint min, uint x, uint max);

int min(int a, int b);

int max(int a, int b);

int clamp(int minv, int x, int maxv);
