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