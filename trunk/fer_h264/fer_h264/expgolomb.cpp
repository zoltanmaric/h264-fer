#include "rawreader.h"
#include "expgolomb.h"

//Coder functions

void expGolomb_UC(unsigned int codeNum)
{
	//"prefix_length" is equal to "suffix_length"
	unsigned int prefix_length=0;

	unsigned int upper_boundary=1;

	unsigned int suffix;

	while (codeNum<upper_boundary)
	{
		prefix_length++;
		upper_boundary=(upper_boundary*2)+1;
	}

	suffix=codeNum-(1<<prefix_length)+1;

	unsigned char suffixRbspValue[4];

	UINT_to_RBSP_size_known(suffix, prefix_length, suffixRbspValue);

	writeZeros(prefix_length);
	writeOnes(1);
	writeRawBits(prefix_length, suffixRbspValue);

}

void expGolomb_SC(unsigned int codeNum)
{
	if (codeNum<0)
	{
		codeNum=(-codeNum)*2;
	}
	else
	{
		codeNum=(codeNum*2)-1;
	}

	expGolomb_UC(codeNum);
}

//Decoder functions

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