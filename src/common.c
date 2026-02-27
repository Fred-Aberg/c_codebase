#include "common.h"
#include "assert.h"
#include "stdio.h"
#include "string.h"
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

uint16_t umin16(uint16_t a, uint16_t b) { return (a < b)? a : b; }
uint16_t umax16(uint16_t a, uint16_t b) { return (a < b)? b : a; }
uint16_t uclamp16(uint16_t minv, uint16_t x, uint16_t maxv) { return umin16(maxv, umax16(minv, x)); }
int16_t min16(int16_t a, int16_t b) { return (a < b)? a : b; }
int16_t max16(int16_t a, int16_t b){ return (a < b)? b : a; }
int16_t clamp16(int16_t minv, int16_t x, int16_t maxv) { return min16(maxv, max16(minv, x)); }

float flmin(float a, float b)
{
	return (a < b)? a : b;
}

float flmax(float a, float b)
{
	return (a < b)? b : a;
}

float flclamp(float minv, float x, float maxv)
{
	return flmin(maxv, flmax(minv, x));
}

uint8_t ui8_array_get(ui8_array_t array, uint32_t index)
{
	if(index < array.capacity)
		return array.items[index];
	else
		ERRORF("\nui8_array_t GET index overflow! i=%u cap=%u", index, array.capacity)
}
void ui8_array_set(ui8_array_t array, uint32_t index, uint8_t value)
{
	if(index < array.capacity)
		array.items[index] = value;
	else
		ERRORF("\nui8_array_t SET index overflow! i=%u cap=%u", index, array.capacity)
}



uint16_t ui16_array_get(ui16_array_t array, uint32_t index)
{
	if(index < array.capacity)
		return array.items[index];
	else
		ERRORF("\nui16_array_t GET index overflow! i=%u cap=%u", index, array.capacity)
}
void ui16_array_set(ui16_array_t array, uint32_t index, uint16_t value)
{
	if(index < array.capacity)
		array.items[index] = value;
	else
		ERRORF("\nui16_array_t SET index overflow! i=%u cap=%u", index, array.capacity)
}


uint32_t ui32_array_get(ui32_array_t array, uint32_t index)
{
	if(index < array.capacity)
		return array.items[index];
	else
		ERRORF("\nui32_array_t GET index overflow! i=%u cap=%u", index, array.capacity)
}
void ui32_array_set(ui32_array_t array, uint32_t index, uint32_t value)
{
	if(index < array.capacity)
		array.items[index] = value;
	else
		ERRORF("\nui32_array_t SET index overflow! i=%u cap=%u", index, array.capacity)
}

void free_array(void *array)
{
	if(((ui32_array_t *)array)->capacity != 0)
	{
		free(((ui32_array_t *)array)->items);
		((ui32_array_t *)array)->items = NULL;
		((ui32_array_t *)array)->capacity = 0;
	}
}

float LIST_REALLOC_INCREASE = 1.5f;
void set_realloc_increase(float percentage_increase)
{
	if(percentage_increase <= 1.0f)
		ERRORF("\nset_realloc_increase: percentage_increase too low (%f <= 1.0)", percentage_increase)
	LIST_REALLOC_INCREASE = percentage_increase;
}


uint8_t ui8_list_get(ui8_list_t list, uint8_t index)
{
	if(index < list.count)
		return list.items[index];
	else
		ERRORF("\nui8_list_t GET index overflow! i=%u cap=%u", index, list.count)
}
void ui8_list_add(ui8_list_t *list, uint8_t value)
{
	if (list->capacity <= list->count)
	{
		list->capacity = list->capacity * LIST_REALLOC_INCREASE + 1;
		list->items = (uint8_t *)realloc(list->items, (list->capacity * sizeof(uint8_t)));
	}

	if(list->ordered) // ordered and maybe duplicates disallowed
		for (uint32_t i = 0; i < list->count; i++)
			if(list->items[i] >= value)
			{
				if(!list->duplicates_allowed)
					return;
				// Move all items from and including items[i] one step forward
				memmove(&list->items[i + 1], &list->items[i], (list->count - i) * sizeof(uint8_t));

				list->items[i] = value;
				list->count++;
				return;
			}

	if(!list->duplicates_allowed) // unordered and duplicates disallowed
		for (uint32_t i = 0; i < list->count; i++)
			if(list->items[i] == value)
				return;

	// Unordered, not a duplicate or largest value
	list->items[list->count] = value;
	list->count++;
}

void ui8_list_remove(ui8_list_t *list, uint8_t index)
{
	if (index >= list->count)
		ERRORF("\nui8_list_t REMOVE: index overflow! i=%u cap=%u", index, list->count)
	if(list->count == 0)
		ERROR("\nui8_list_t REMOVE: list is empty!")


	if(list->count == 1 || index == list->count - 1) // Last element or only element
	{
		list->count--;
		return;
	}

	// Move all items from and including items[i] one step backward
	memmove(&list->items[index], &list->items[index + 1], (list->count - index) * sizeof(uint8_t));

	list->count--;
	return;
}

uint16_t ui16_list_get(ui16_list_t list, uint16_t index)
{
	if(index < list.count)
		return list.items[index];
	else
		ERRORF("\nui16_list_t GET index overflow! i=%u cap=%u", index, list.count)
}
void ui16_list_add(ui16_list_t *list, uint16_t value)
{
	if (list->capacity <= list->count)
	{
		list->capacity = list->capacity * LIST_REALLOC_INCREASE + 1;
		list->items = (uint16_t *)realloc(list->items, (list->capacity * sizeof(uint16_t)));
	}

	if(list->ordered) // ordered and maybe duplicates disallowed
		for (uint32_t i = 0; i < list->count; i++)
			if(list->items[i] >= value)
			{
				if(!list->duplicates_allowed)
					return;
				// Move all items from and including items[i] one step forward
				memmove(&list->items[i + 1], &list->items[i], (list->count - i) * sizeof(uint16_t));

				list->items[i] = value;
				list->count++;
				return;
			}

	if(!list->duplicates_allowed) // unordered and duplicates disallowed
		for (uint32_t i = 0; i < list->count; i++)
			if(list->items[i] == value)
				return;

	// Unordered, not a duplicate or largest value
	list->items[list->count] = value;
	list->count++;
}

void ui16_list_remove(ui16_list_t *list, uint16_t index)
{
	if (index >= list->count)
		ERRORF("\nui16_list_t REMOVE: index overflow! i=%u cap=%u", index, list->count)
	if(list->count == 0)
		ERROR("\nui16_list_t REMOVE: list is empty!")


	if(list->count == 1 || index == list->count - 1) // Last element or only element
	{
		list->count--;
		return;
	}

	// Move all items from and including items[i] one step backward
	memmove(&list->items[index], &list->items[index + 1], (list->count - index) * sizeof(uint16_t));

	list->count--;
	return;
}

uint32_t ui32_list_get(ui32_list_t list, uint32_t index)
{
	if(index < list.count)
		return list.items[index];
	else
		ERRORF("\nui32_list_t GET index overflow! i=%u cap=%u", index, list.count)
}
void ui32_list_add(ui32_list_t *list, uint32_t value)
{
	if (list->capacity <= list->count)
	{
		list->capacity = list->capacity * LIST_REALLOC_INCREASE + 1;
		list->items = (uint32_t *)realloc(list->items, (list->capacity * sizeof(uint32_t)));
	}

	if(list->ordered) // ordered and maybe duplicates disallowed
		for (uint32_t i = 0; i < list->count; i++)
			if(list->items[i] >= value)
			{
				if(!list->duplicates_allowed)
					return;
				// Move all items from and including items[i] one step forward
				memmove(&list->items[i + 1], &list->items[i], (list->count - i) * sizeof(uint32_t));

				list->items[i] = value;
				list->count++;
				return;
			}

	if(!list->duplicates_allowed) // unordered and duplicates disallowed
		for (uint32_t i = 0; i < list->count; i++)
			if(list->items[i] == value)
				return;

	// Unordered, not a duplicate or largest value
	list->items[list->count] = value;
	list->count++;
}

void ui32_list_remove(ui32_list_t *list, uint32_t index)
{
	if (index >= list->count)
		ERRORF("\nui32_list_t REMOVE: index overflow! i=%u cap=%u", index, list->count)
	if(list->count == 0)
		ERROR("\nui32_list_t REMOVE: list is empty!")


	if(list->count == 1 || index == list->count - 1) // Last element or only element
	{
		list->count--;
		return;
	}

	// Move all items from and including items[i] one step backward
	memmove(&list->items[index], &list->items[index + 1], (list->count - index) * sizeof(uint32_t));

	list->count--;
	return;
}

pos8_t pos8_list_get(pos8_list_t list, uint16_t index)
{
	if(index < list.count)
		return list.items[index];
	else
		ERRORF("\npos8_list_t GET index overflow! i=%u cap=%u", index, list.count)
}

void pos8_list_add(pos8_list_t *list, pos8_t value)
{
	if (list->capacity <= list->count)
	{
		list->capacity = list->capacity * LIST_REALLOC_INCREASE + 1;
		list->items = (pos8_t *)realloc(list->items, (list->capacity * sizeof(pos8_t)));
	}

	if(list->ordered) // ordered and maybe duplicates disallowed
		for (uint16_t i = 0; i < list->count; i++)
			if(pos8_to_ui16(list->items[i]) >= pos8_to_ui16(value))
			{
				if(!list->duplicates_allowed)
					return;
				// Move all items from and including items[i] one step forward
				memmove(&list->items[i + 1], &list->items[i], (list->count - i) * sizeof(pos8_t));

				list->items[i] = value;
				list->count++;
				return;
			}

	if(!list->duplicates_allowed) // unordered and duplicates disallowed
		for (uint32_t i = 0; i < list->count; i++)
			if(pos8_to_ui16(list->items[i]) == pos8_to_ui16(value))
				return;

	// Unordered, not a duplicate or largest value
	list->items[list->count] = value;
	list->count++;
}

void pos8_list_remove(pos8_list_t *list, uint16_t index)
{
	if (index >= list->count)
		ERRORF("\npos8_list_t REMOVE: index overflow! i=%u cap=%u", index, list->count)
	if(list->count == 0)
		ERROR("\npos8_list_t REMOVE: list is empty!")


	if(list->count == 1 || index == list->count - 1) // Last element or only element
	{
		list->count--;
		return;
	}

	// Move all items from and including items[i] one step backward
	memmove(&list->items[index], &list->items[index + 1], (list->count - index) * sizeof(pos8_t));

	list->count--;
	return;
}

pos16_t pos16_list_get(pos16_list_t list, uint32_t index)
{
	if(index < list.count)
		return list.items[index];
	else
		ERRORF("\npos16_list_t GET index overflow! i=%u cap=%u", index, list.count)
}
void pos16_list_add(pos16_list_t *list, pos16_t value)
{
	if (list->capacity <= list->count)
	{
		list->capacity = list->capacity * LIST_REALLOC_INCREASE + 1;
		list->items = (pos16_t *)realloc(list->items, (list->capacity * sizeof(pos16_t)));
	}

	if(list->ordered) // ordered and maybe duplicates disallowed
		for (uint32_t i = 0; i < list->count; i++)
			if(pos16_to_ui32(list->items[i]) >= pos16_to_ui32(value))
			{
				if(!list->duplicates_allowed)
					return;
				// Move all items from and including items[i] one step forward
				memmove(&list->items[i + 1], &list->items[i], (list->count - i) * sizeof(pos16_t));

				list->items[i] = value;
				list->count++;
				return;
			}

	if(!list->duplicates_allowed) // unordered and duplicates disallowed
		for (uint32_t i = 0; i < list->count; i++)
			if(pos16_to_ui32(list->items[i]) == pos16_to_ui32(value))
				return;

	// Unordered, not a duplicate or largest value
	list->items[list->count] = value;
	list->count++;
}

void pos16_list_remove(pos16_list_t *list, uint32_t index)
{
	if (index >= list->count)
		ERRORF("\npos16_list_t REMOVE: index overflow! i=%u cap=%u", index, list->count)
	if(list->count == 0)
		ERROR("\npos16_list_t REMOVE: list is empty!")


	if(list->count == 1 || index == list->count - 1) // Last element or only element
	{
		list->count--;
		return;
	}

	// Move all items from and including items[i] one step backward
	memmove(&list->items[index], &list->items[index + 1], (list->count - index) * sizeof(pos16_t));

	list->count--;
}

void *ptr_list_get(ptr_list_t list, uint32_t index)
{
	if(index < list.count)
		return list.items[index];
	else
		ERRORF("\nptr_list_t GET index overflow! i=%u cap=%u", index, list.count)
}

void ptr_list_add(ptr_list_t *list, void *value)
{
	if (list->capacity <= list->count)
	{
		list->capacity = list->capacity * LIST_REALLOC_INCREASE + 1;
		list->items = (void *)realloc(list->items, (list->capacity * sizeof(void *)));
	}

	if(list->ordered) // ordered and maybe duplicates disallowed
		for (uint32_t i = 0; i < list->count; i++)
			if(list->items[i] >= value)
			{
				if(!list->duplicates_allowed)
					return;
				// Move all items from and including items[i] one step forward
				memmove(&list->items[i + 1], &list->items[i], (list->count - i) * sizeof(void *));

				list->items[i] = value;
				list->count++;
				return;
			}

	if(!list->duplicates_allowed) // unordered and duplicates disallowed
		for (uint32_t i = 0; i < list->count; i++)
			if(list->items[i] == value)
				return;

	// Unordered, not a duplicate or largest value
	list->items[list->count] = value;
	list->count++;
}

void ptr_list_remove(ptr_list_t *list, uint32_t index)
{
	if (index >= list->count)
		ERRORF("\nptr_list_t REMOVE: index overflow! i=%u cap=%u", index, list->count)
	if(list->count == 0)
		ERROR("\nptr_list_t REMOVE: list is empty!")


	if(list->count == 1 || index == list->count - 1) // Last element or only element
	{
		list->count--;
		return;
	}

	// Move all items from and including items[i] one step backward
	memmove(&list->items[index], &list->items[index + 1], (list->count - index) * sizeof(void *));

	list->count--;
}

void free_list(void *list)
{
	if(((ui32_list_t *)list)->capacity != 0)
	{
		free(((ui32_list_t *)list)->items);
		((ui32_list_t *)list)->items = NULL;
		((ui32_list_t *)list)->capacity = 0;
		((ui32_list_t *)list)->count = 0;
	}
}

/// STRINGS
uint32_t charr_length(char *charray)
{
	return strlen(charray) + 1;
}

void str_null_end(str_t *str)
{
	(&str->c)[str->length - 1] = 0;
}

char *str_charr(str_t *str)
{
	return &str->c;
}

char str_get(str_t *str, uint32_t i)
{
	if(i > str->length - 1) //	you are allowed to read str[length - 1]
		ERRORF("str_get: index overflow (i=%u > l=%u) in string containing'%s'", i, str->length, str_charr(str))
	return (&str->c)[i];
}

void str_set(str_t *str, uint32_t i, char c)
{
	if(i >= str->length - 1) //	you are NOT allowed to write to str[length - 1]
		ERRORF("str_set: index overflow (i=%u >= l=%u) in string containing'%s'", i, str->length, str_charr(str))
	(&str->c)[i] = c;
}

str_t *str_realloc(str_t **str, uint32_t new_capacity)
{
	(*str)->capacity = new_capacity;
	if((*str)->length > new_capacity)
	{
		(*str)->length = new_capacity;
		str_null_end((*str));
	}
	return realloc((*str), 2 * sizeof(uint32_t) + new_capacity * sizeof(char));
}

void str_append(str_t **str, char c)
{
	if((*str)->length + 1 > (*str)->capacity)
		(*str) = str_realloc(str, (*str)->capacity * LIST_REALLOC_INCREASE + 1);

	(&(*str)->c)[(*str)->length - 1] = c;
	(*str)->length++;
	str_null_end((*str));
}

str_t *str_empty(uint32_t init_capacity)
{
	str_t *new_str = calloc(1, 2 * sizeof(uint32_t) + MAX(init_capacity * sizeof(char), 1));
	new_str->capacity = init_capacity;
	new_str->length = 1; // initial string is "" = {'\0'}

	return new_str;
}

str_t *str_from_charr(char *src, uint32_t length)
{
	str_t *new_str = str_empty(length);
	memcpy(str_charr(new_str), src, length);
	new_str->length = length;
	str_null_end(new_str);

	return new_str;
}

str_t *str_copy(str_t *src)
{
	str_t *new_str = str_empty(src->capacity);
	memcpy(str_charr(new_str), str_charr(src), src->length);
	new_str->length = src->length;

	return new_str;
}

str_t *str_from_buf(char *buf, uint32_t buf_max_size)
{
	uint32_t buf_true_size = charr_length(buf);
	if (buf_true_size > buf_max_size)
		ERRORF("str_from_buf: overflow detected in buffer = '%s'", buf)

	str_t *new_str = str_from_charr(buf, buf_true_size);

	return new_str;
}

void str_write_from_buf(str_t **dest, char *buf, uint32_t buf_max_size)
{
	uint32_t buf_true_size = charr_length(buf);
	if (buf_true_size > buf_max_size)
		ERRORF("str_write_from_buf: overflow detected in buffer = '%s'", buf)

	if((*dest)->capacity < buf_true_size)
		(*dest) = str_realloc(dest, buf_true_size);

	memcpy(str_charr((*dest)), buf, buf_true_size);
	(*dest)->length = buf_true_size;
	str_null_end((*dest));
}

void str_write_from_charr(str_t **dest, char *src, uint32_t length)
{
	if((*dest)->capacity < length)
		(*dest) = str_realloc(dest, length);

	memcpy(str_charr(*dest), src, length);
	(*dest)->length = length;
	str_null_end(*dest);
}

void str_write_from_str(str_t **dest, str_t *src)
{
	str_write_from_charr(dest, str_charr(src), src->length);
}

void **allocate_memory_grid(uint32_t m, uint32_t n, uint32_t elem_size)
{
    // 'm' pointers, each pointing to 'n' elements
    size_t ptrs = sizeof(void *) * m;
    size_t elems = m*n * elem_size;

    void **grid = (void **)calloc(1, ptrs + elems);

    grid[0] = &grid[m]; // First ptr points to space directly after pointers

	size_t column_size = n * elem_size;
	for (uint64_t i = 1; i < m; i++)
		grid[i] = grid[0] + column_size * i;

	return grid;
}

/*
 *
void ***allocate_memory_cube(uint32_t i, uint32_t j, uint32_t k, uint32_t elem_size)
{
    // TODO: MAKE INTO SINGLE ALLOCATION
    void ***cube = calloc(i, sizeof(void *));
	for (uint64_t _i = 0; _i < i; _i++)
        cube[_i] = allocate_memory_grid(j, k, elem_size);
}
*/

reg_t *reg_create(uint16_t elem_size, uint32_t init_cap)
{
	reg_t *reg = calloc(1, sizeof(reg_t));
	reg->elem_size = elem_size;
	reg->data = 		calloc(init_cap, elem_size);
	reg->data_loc = 	calloc(init_cap, sizeof(uint32_t));
	reg->data_id = 	calloc(init_cap, sizeof(uint32_t));

	for (uint32_t i = 0; i < init_cap; i++)
	{
		reg->data_loc[i] = i;
		reg->data_id[i] = i;
	}

	return reg;
}

uint32_t reg_add(reg_t *reg, void *new_data_ptr)
{
	reg->count++;

	if (reg->cap <= reg->count)
	{
		reg->cap = reg->cap * 1.5f + 1;
		reg->data = 		realloc(reg->data, reg->cap * reg->elem_size);
		reg->data_id = 	realloc(reg->data_id, reg->cap * sizeof(uint32_t));
		reg->data_loc = 	realloc(reg->data_loc, reg->cap * sizeof(uint32_t));
	}

	memcpy(reg->data + reg->elem_size * (reg->count - 1), new_data_ptr, reg->elem_size);
	if(reg->id_loc_count < reg->count)
	{
        reg->id_loc_count++;
        reg->data_id[reg->count - 1] =  reg->count - 1;
        reg->data_loc[reg->count - 1] =	reg->count - 1;
	}

	return reg->count - 1;
}

// If smarr contains ptrs, contents this ptr points to must be freed before removal.
void reg_rem(reg_t *reg, uint32_t id)
{
    if (reg->id_loc_count <= id)
        ERRORF("ID overflow: id_loc_count=%u <= id=%u\n", reg->id_loc_count, id);
	if (reg->count <= reg->data_loc[id])
		ERRORF("Item has been removed: count=%u <= data_loc[id=%u]=%u\n", reg->count, id, reg->data_loc[id]);

    // Overwrite element to be removed with the last element
    void *last_elem =       reg->data + (reg->elem_size * (reg->count - 1));
    void *elem_to_remove =  reg->data + reg->elem_size * reg->data_loc[id];
    memcpy(elem_to_remove, last_elem, reg->elem_size);

    // Swap IDs
    uint32_t last_elem_id =         reg->data_id[reg->count - 1];
    uint32_t elem_to_remove_id =    reg->data_id[reg->data_loc[id]];
    reg->data_id[reg->count - 1] =      elem_to_remove_id;
    reg->data_id[reg->data_loc[id]] =   last_elem_id;

    // Swap Locations
    uint32_t elem_to_remove_loc = reg->data_loc[id];
    reg->data_loc[last_elem_id] = elem_to_remove_loc;
    reg->data_loc[id] = last_elem_id;

    reg->count--;
}

void *reg_get(reg_t *reg, uint32_t id)
{
    if (reg->id_loc_count <= id)
        ERRORF("ID overflow: id_loc_count=%u <= id=%u\n", reg->id_loc_count, id);
	if (reg->count <= reg->data_loc[id])
		ERRORF("Item has been removed: count=%u <= data_loc[id=%u]=%u\n", reg->count, id, reg->data_loc[id]);

	return reg->data + reg->elem_size * reg->data_loc[id];
}

void reg_set(reg_t *reg, uint32_t id, void *new_data_ptr)
{
    if (reg->id_loc_count <= id)
        ERRORF("ID overflow: id_loc_count=%u <= id=%u\n", reg->id_loc_count, id);
	if (reg->count <= reg->data_loc[id])
		ERRORF("Item has been removed: count=%u <= data_loc[id=%u]=%u\n", reg->count, id, reg->data_loc[id]);

	memcpy(reg->data + reg->elem_size * reg->data_loc[id], new_data_ptr, reg->elem_size);
}

// Get data in memory-order
void *reg_iter_get(reg_t *reg, uint32_t data_num)
{
    if (reg->count <= data_num)
		ERRORF("data_num overflow: count=%d <= data_num=%d\n", data_num, reg->count);

    return reg->data + reg->elem_size * data_num;
}

// Set data in memory-order
void reg_iter_set(reg_t *reg, uint32_t data_num, void *new_data_ptr)
{
    if (reg->count <= data_num)
		ERRORF("data_num overflow: count=%d <= data_num=%d\n", data_num, reg->count);

    memcpy(reg->data + reg->elem_size * data_num, new_data_ptr, reg->elem_size);
}

// If smarr contains ptrs, contents this ptr points to must be freed before removal.
// Remove data in ID-order
uint32_t reg_iter_rem(reg_t *reg, uint32_t data_num)
{
    if (reg->count <= data_num)
		ERRORF("data_num overflow: count=%d <= data_num=%d\n", data_num, reg->count);

    reg_rem(reg,  reg->data_id[data_num]);
    return reg->data_id[data_num];
}
