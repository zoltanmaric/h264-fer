#include "rawreader.h"
#include "expgolomb.h"

unsigned int expGolomb_UD()
{
	unsigned int zeroCount=0;

	while (!getRawBits(1))
	{
		zeroCount++;
	}

	unsigned int broj=getRawBits(zeroCount);
	return (1<<zeroCount)-1+broj;
}

signed int expGolomb_SD()
{
	int returnValue=expGolomb_UD();
	if (returnValue%2)
	{
		return (returnValue+1)/2;
	}
	else
	{
		return -returnValue/2;
	}
}

// Truncated exp-golomb code
unsigned int expGolomb_TD()
{
	unsigned int zeroCount = 0;
	
	while (getRawBits(1) == 0)
	{
		zeroCount++;
	}

	if (zeroCount == 0)
	{
		return 0;
	}

	unsigned int x = getRawBits(zeroCount);
	if (x > 1)
	{
		return (1 << zeroCount) - 1 + x;
	}
	else
	{
		return !getRawBits(1);
	}
}