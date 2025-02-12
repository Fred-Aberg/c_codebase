#include "common.h"

uint umin(uint a, uint b)
{
	return (a < b)? a : b;
}

uint umax(uint a, uint b)
{
	return (a < b)? b : a;
}

uint uclamp(uint minv, uint x, uint maxv)
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
