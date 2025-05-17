#pragma once
#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"

#define ERROR(str) {fprintf(stdout, str); putc('\n', stdout); fflush(stdout); assert(0);}
#define ERRORF(str, ...) {fprintf(stdout, str, __VA_ARGS__); putc('\n', stdout); fflush(stdout); assert(0);}
#define WARNING(str) {fprintf(stdout, str); putc('\n', stdout);}
#define WARNINGF(str, ...) {fprintf(stdout, str, __VA_ARGS__); putc('\n', stdout);}
#define UNUSED(var) (void) var

typedef struct
{
    uint8_t x;
    uint8_t y;
} pos8_t;
#define pos8_to_ui16(p) (uint16_t)((p.y << 8) + p.x)

typedef struct
{
    uint16_t x;
    uint16_t y;
} pos16_t;
#define pos16_to_ui32(p) (uint32_t)((p.y << 16) + p.x)

#define c(r, g, b) (Color){r, g, b, 255}
#define pos8(x, y) (pos8_t){x, y}
#define pos16(x, y) (pos16_t){x, y}

uint16_t umin(uint16_t a, uint16_t b);

uint16_t umax(uint16_t a, uint16_t b);

uint16_t uclamp(uint16_t min, uint16_t x, uint16_t max);

int16_t min(int16_t a, int16_t b);

int16_t max(int16_t a, int16_t b);

int16_t clamp(int16_t minv, int16_t x, int16_t maxv);

float flmin(float a, float b);

float flmax(float a, float b);

float flclamp(float minv, float x, float maxv);

/// ARRAYS

// static, not neccessarily contiguous

#define new_array(type, capacity) {calloc(capacity, sizeof(type)), capacity}

#define ORDERED 1
#define UNORDERED 0
#define DUPLICATES_ALLOWED 1
#define NO_DUPLICATES 0
#define new_list(type, init_capacity, ordered, dups_allowed) \
{calloc(init_capacity, sizeof(type)), 0, init_capacity, ordered, dups_allowed }


#define clear_list(list) ((ui8_list_t *)list)->count = 0

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

void free_array(void *array);

/// LISTS

// 1.5 by default (50% increase)
void set_realloc_increase(float percentage_increase);

// Unordered or ordered
// Contiguous

typedef struct
{
    uint8_t *items;
    uint32_t count;
    uint32_t capacity;
    bool ordered;
    bool duplicates_allowed;
} ui8_list_t;

uint8_t ui8_list_get(ui8_list_t list, uint8_t index);
void ui8_list_add(ui8_list_t *list, uint8_t value);
void ui8_list_remove(ui8_list_t *list, uint8_t index);

typedef struct
{
    uint16_t *items;
    uint32_t count;
    uint32_t capacity;
    bool ordered;
    bool duplicates_allowed;
} ui16_list_t;

uint16_t ui16_list_get(ui16_list_t list, uint16_t index);
void ui16_list_add(ui16_list_t *list, uint16_t value);
void ui16_list_remove(ui16_list_t *list, uint16_t index);

typedef struct
{
    uint32_t *items;
    uint32_t count;
    uint32_t capacity;
    bool ordered;
	bool duplicates_allowed;
} ui32_list_t;

uint32_t ui32_list_get(ui32_list_t list, uint32_t index);
void ui32_list_add(ui32_list_t *list, uint32_t value);
void ui32_list_remove(ui32_list_t *list, uint32_t index);

typedef struct
{
    pos8_t *items;
    uint32_t count;
    uint32_t capacity;
    bool ordered;
	bool duplicates_allowed;
} pos8_list_t;

pos8_t pos8_list_get(pos8_list_t list, uint16_t index);
void pos8_list_add(pos8_list_t *list, pos8_t value);
void pos8_list_remove(pos8_list_t *list, uint16_t index);

typedef struct
{
    pos16_t *items;
    uint32_t count;
    uint32_t capacity;
    bool ordered;
	bool duplicates_allowed;
} pos16_list_t;

pos16_t pos16_list_get(pos16_list_t list, uint32_t index);
void pos16_list_add(pos16_list_t *list, pos16_t value);
void pos16_list_remove(pos16_list_t *list, uint32_t index);

void free_list(void *list);

typedef struct
{
    void **items;
    uint32_t count;
    uint32_t capacity;
    bool ordered;
	bool duplicates_allowed;
} ptr_list_t;

void *ptr_list_get(ptr_list_t list, uint32_t index);
void ptr_list_add(ptr_list_t *list, void *value);
void ptr_list_remove(ptr_list_t *list, uint32_t index);

/// STRINGS

// Always owns its own array of chars
// A string's length shall always include a null-terminator 
//		=> (&str->c)[length - 1] = '\0', (&str->c)[length] => illegal access.

// A string's length is never 0, the smallest string is "" with length: 1 
	//	ex. str 	|	length	|	array
	//		""		|	1		|	{'\0'}
	//		"a"		|	2 		|	{'a','\0'}
	//		"abc"	|	4		|	{'a', 'b', 'c', '\0'}
 
typedef struct
{
	uint32_t length;
	uint32_t capacity;
	char c;
}str_t;

// Never owns its own array of chars
typedef struct
{
	uint32_t start_index;
	uint32_t end_index;
	str_t *str;
}str_slice_t;

/// String functions ///
// new allocations should be returned

#define slice(str, start, end) (str_slice_t) {start, end, str}
#define str_ref(str) (str_slice_t) {0, str->length, str}

// Reads the i:th char of str
//	Pre:	0 <= i <= (length - 1)
char str_get(str_t *str, uint32_t i);

// Writes to the i:th char of str
//	Pre:	0 <= i < (length - 1)		You cannot write to the null character
void str_set(str_t *str, uint32_t i, char c);

void str_append(str_t **str, char c);

str_t *str_realloc(str_t **str, uint32_t new_capacity);

// return char *ptr from str
char *str_charr(str_t *str);

str_t *str_empty(uint32_t init_capacity);

str_t *str_from_char_ptr(char *src, uint32_t length);
	// ex: 	str_t *new_str = str_from_char_ptr(src, strlen(src));

#define str(literal) str_from_char_ptr(literal, sizeof(literal))
	// ex:	str_t *new_str = str("abc");

str_t *str_copy(str_t *src);
	
str_t *str_from_buf(char *buf, uint32_t buf_max_size);
	// ex: 	char buf[256];								: buffer *theoretically* large enough to hold sprintf output
	//	 	sprintf(buf, fmt, vars);			
	//	 	str_t *new_str = str_from_buf(buf, 256);	: will throw an error if buf isn't nulled (which implies overflow)


void str_write_from_buf(str_t **dest, char *buf, uint32_t buf_max_size);
	// ex: 	char buf[256];								: buffer *theoretically* large enough to hold sprintf output
	//	 	sprintf(buf, fmt, vars);			
	//	 	str_write_from_buf(str, buf, 256);			: will throw an error if buf isn't nulled (which implies overflow)
	
#define str_write_from_sprintf(buf_max_size, dest, fmt, ...)		\
	{char buf[buf_max_size];										\
	buf[buf_max_size - 1] = 0;										\
	sprintf(buf, str_charr(fmt), __VA_ARGS__);						\
	str_write_from_buf(dest, buf, buf_max_size);}

#define str_write_from_sprintf_lit_fmt(buf_max_size, dest, fmt, ...)\
	{char buf[buf_max_size];										\
	buf[buf_max_size - 1] = 0;										\
	sprintf(buf,fmt, __VA_ARGS__);									\
	str_write_from_buf(dest, buf, buf_max_size);}

void str_write_from_char_ptr(str_t **dest, char *src, uint32_t length);

void str_write_from_str(str_t **dest, str_t *src);

#define str_write(str, literal) str_write_from_char_ptr(str, literal, sizeof(literal))
