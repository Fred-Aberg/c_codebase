#pragma once
#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"

#define ERROR(str) {fprintf(stdout, str); putc('\n', stdout); fflush(stdout); assert(false);}
#define ERRORF(str, ...) {fprintf(stdout, str, __VA_ARGS__); putc('\n', stdout); fflush(stdout); assert(0);}
#define WARNINGF(str, ...) {fprintf(stdout, str, __VA_ARGS__); putc('\n', stdout);}

typedef struct
{
    uint8_t x;
    uint8_t y;
} pos8_t;

typedef struct
{
    uint16_t x;
    uint16_t y;
} pos16_t;

#define c(r, g, b) (Color){r, g, b, 255}
#define pos8(x, y) (pos8_t){x, y}
#define pos16(x, y) (pos16_t){x, y}

uint16_t umin(uint16_t a, uint16_t b);

uint16_t umax(uint16_t a, uint16_t b);

uint16_t uclamp(uint16_t min, uint16_t x, uint16_t max);

int16_t min(int16_t a, int16_t b);

int16_t max(int16_t a, int16_t b);

int16_t clamp(int16_t minv, int16_t x, int16_t maxv);


/// ARRAYS

// static, not neccessarily contiguous

typedef struct
{
    uint8_t *items;
    uint32_t capacity;
} ui8_array_t;

uint8_t ui8_array_get(ui8_array_t array, uint32_t index);
void ui8_array_set(ui8_array_t array, uint32_t index, uint8_t value);

typedef struct
{
    uint16_t *items;
    uint32_t capacity;
} ui16_array_t;

uint16_t ui16_array_get(ui16_array_t array, uint32_t index);
void ui16_array_set(ui16_array_t array, uint32_t index, uint16_t value);

typedef struct
{
    uint32_t *items;
    uint32_t capacity;
} ui32_array_t;

uint32_t ui32_array_get(ui32_array_t array, uint32_t index);
void ui32_array_set(ui32_array_t array, uint32_t index, uint32_t value);

/// LISTS

#define LIST_REALLOC_INCREASE 1.5f

// Unordered or ordered
// Contiguous

typedef struct
{
    uint32_t *items;
    uint32_t count;
    uint32_t capacity;
    bool ordered;
} ui32_list_t;

uint32_t ui32_list_get(ui32_list_t list, uint32_t index);
void ui32_list_add(ui32_list_t *list, uint32_t value);
void ui32_list_remove(ui32_list_t *list, uint32_t index);
