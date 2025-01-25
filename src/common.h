#pragma once
#include "stdlib.h"

typedef unsigned int uint;
typedef unsigned char uchar;

typedef struct
{
    uint x;
    uint y;
} Pos_t;

#define pos(x, y) (Pos_t){x, y}