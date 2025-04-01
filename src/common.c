#include "common.h"
#include "assert.h"
#include "stdio.h"
#include "string.h"

uint16_t umin(uint16_t a, uint16_t b)
{
	return (a < b)? a : b;
}

uint16_t umax(uint16_t a, uint16_t b)
{
	return (a < b)? b : a;
}

uint16_t uclamp(uint16_t minv, uint16_t x, uint16_t maxv)
{
	return umin(maxv, umax(minv, x));
}

int16_t min(int16_t a, int16_t b)
{
	return (a < b)? a : b;
}

int16_t max(int16_t a, int16_t b)
{
	return (a < b)? b : a;
}

int16_t clamp(int16_t minv, int16_t x, int16_t maxv)
{
	return min(maxv, max(minv, x));
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
	
	if(list->ordered)
	{
		for (uint32_t i = 0; i < list->count; i++)
		{
			if(list->items[i] > value)
			{
				// Move all items from and including items[i] one step forward
				memmove(&list->items[i + 1], &list->items[i], (list->count - i) * sizeof(uint32_t));
				
				list->items[i] = value;
				list->count++;
				return;
			}
		}
	}

	// Unordered
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
