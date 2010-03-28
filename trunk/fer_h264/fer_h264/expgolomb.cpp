#include "rawreader.h"
#include "expgolomb.h"

//Coder functions

void golombRice_SC(int codeNum, unsigned int VLCNum)
{
	unsigned int offset;
	unsigned int prefix_length,suffix;
	
	unsigned char buffer[4];
	
	if (codeNum<0)
	{
		offset=(-codeNum-1)*2+1;
	}
	else
	{
		offset=(codeNum-1)*2;
	}


	suffix=offset%(1<<VLCNum);

	prefix_length=offset/(1<<VLCNum);

	writeZeros(prefix_length);
	writeOnes(1);

	if (VLCNum!=0)
	{
		UINT_to_RBSP_size_known(suffix,VLCNum,buffer);
		writeRawBits(VLCNum,buffer);
	}
}

void expGolomb_UC(unsigned int codeNum)
{
	//"prefix_length" is equal to "suffix_length"
	unsigned int prefix_length=1;

	unsigned int upper_boundary=2;
	unsigned int lower_boundary=1;
	unsigned int temp_boundary;

	unsigned int suffix;

	if (codeNum==0)
	{
		writeOnes(1);
	}
	else
	{
		while (codeNum>upper_boundary)
		{
			prefix_length++;
			temp_boundary=upper_boundary;
			upper_boundary=(lower_boundary*2)+1;
			lower_boundary=temp_boundary+1;
		}

		suffix=codeNum-(1<<prefix_length)+1;

		unsigned char suffixRbspValue[4];

		UINT_to_RBSP_size_known(suffix, prefix_length, suffixRbspValue);

		writeZeros(prefix_length);
		writeOnes(1);
		writeRawBits(prefix_length, suffixRbspValue);
	}
}

void expGolomb_SC(unsigned int codeNum)
{
	if (codeNum<=0)
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