#include "common.h"

uint umin(uint a, uint b)
{
	return (a < b)? a : b;
}

uint umax(uint a, uint b)
{
	return (a < b)? b : a;
}

uint uclamp(uint min, uint x, uint max)
{
	return umin(max, umax(min, x));
}
