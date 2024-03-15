#include "common.h"

int min(int a, int b)
{
	return (a < b)? a : b;
}

uint umin(uint a, uint b)
{
	return (a < b)? a : b;
}

int max(int a, int b)
{
	return (a < b)? b : a;
}

uint umax(uint a, uint b)
{
	return (a < b)? b : a;
}

int clamp(int low, int x, int high)
{
	return min(high, max(low, x));
}

uint uclamp(uint low, uint x, uint high)
{
	return umin(high, umax(low, x));
}
