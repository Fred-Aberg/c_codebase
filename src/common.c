#include "common.h"

uint_t umin(uint_t a, uint_t b)
{
	return (a < b)? a : b;
}

uint_t umax(uint_t a, uint_t b)
{
	return (a < b)? b : a;
}

uint_t uclamp(uint_t minv, uint_t x, uint_t maxv)
{
	return umin(maxv, umax(minv, x));
}

int min(int a, int b)
{
	return (a < b)? a : b;
}

int max(int a, int b)
{
	return (a < b)? b : a;
}

int clamp(int minv, int x, int maxv)
{
	return min(maxv, max(minv, x));
}
