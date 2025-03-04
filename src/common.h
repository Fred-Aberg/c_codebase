#pragma once
#include "stdlib.h"

typedef unsigned char uchar_t;
typedef unsigned short ushort_t;
typedef unsigned int uint_t;
typedef unsigned long ulong_t;

#define ERROR(str) {fprintf(stdout, str); putc('\n', stdout); fflush(stdout); assert(false);}
#define ERRORF(str, ...) {fprintf(stdout, str, __VA_ARGS__); putc('\n', stdout); fflush(stdout); assert(false);}
#define WARNINGF(str, ...) {fprintf(stdout, str, __VA_ARGS__); putc('\n', stdout);}


typedef struct
{
    uint_t x;
    uint_t y;
} Pos_t;

#define c(r, g, b) (Color){r, g, b, 255}
#define pos(x, y) (Pos_t){x, y}

uint_t umin(uint_t a, uint_t b);

uint_t umax(uint_t a, uint_t b);

uint_t uclamp(uint_t min, uint_t x, uint_t max);

int min(int a, int b);

int max(int a, int b);

int clamp(int minv, int x, int maxv);
