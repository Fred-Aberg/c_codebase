#pragma once

typedef unsigned int uint;
typedef unsigned char uchar;

typedef struct
{
    uint x;
    uint y;
} Pos_t;

#define pos(x, y) (Pos_t){x, y}

int min(int a, int b);

uint umin(uint a, uint b);

int max(int a, int b);

uint umax(uint a, uint b);

int clamp(int min, int x, int max);

uint uclamp(uint min, uint x, uint max);

