#include "rbsp_IO.h"
#include "expgolomb.h"
#include <stdio.h>

//Didn't encounter codeNums larger than 10000 (actually no codeNums larger than ~100) during testing
unsigned int expgolomb_UC_codes[10000][2];

void init_expgolomb_UC_codes()
{
	for (int codeNum=0;codeNum<10000;codeNum++)
	{
		unsigned int prefix_length=1;
		unsigned int upper_boundary=2;
		unsigned int lower_boundary=1;

		unsigned int suffix;

		if (codeNum==0)
		{
			expgolomb_UC_codes[codeNum][0]=0;
			expgolomb_UC_codes[codeNum][1]=0;
		}
		else
		{
			while (codeNum>upper_boundary)
			{
				prefix_length++;
				lower_boundary = upper_boundary + 1;
				upper_boundary = lower_boundary << 1;
			}

			suffix=codeNum-(1<<prefix_length)+1;
			expgolomb_UC_codes[codeNum][0]=prefix_length;
			expgolomb_UC_codes[codeNum][1]=suffix;
		}
	}
}

//Coder functions

//There are two of these array (one is in rawreader), with different ordering
unsigned int one_bit_masks_large[24]=
{
	0x800000, 0x400000, 0x200000, 0x100000, 0x80000, 0x40000, 0x20000, 0x10000,
	0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100,
	0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1
};

void golombRice_SC(int codeNum, unsigned int VLCNum)
{
	unsigned int offset;
	unsigned int prefix_length,suffix;
	
	if (codeNum<=0)
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
	//for (int i=0;i<prefix_length;i++)
		//printf("0");
	writeOnes(1);
	//printf("1");

	if (VLCNum!=0)
	{
		writeRawBits(VLCNum, suffix);
	}
}

void expGolomb_UC(unsigned int codeNum)
{
	if (codeNum==0)
	{
		writeOnes(1);
	}
	else
	{
		writeZeros(expgolomb_UC_codes[codeNum][0]);
		writeOnes(1);
		writeRawBits(expgolomb_UC_codes[codeNum][0], expgolomb_UC_codes[codeNum][1]);
	}
}

void expGolomb_SC(int codeNum)
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

unsigned int SC_to_UC(int codeNum)
{
	if (codeNum<=0)
	{
		return (unsigned int)((-codeNum)*2);
	}
	else
	{
		return (unsigned int)((codeNum*2)-1);
	}
}

//Decoder functions

unsigned int expGolomb_UD()
{
	unsigned int zeroCount=0;

	//Using fast N=24 peek request
	unsigned int search_buffer=peekRawBits(24);
	int i;
	for (i=0;i<24;i++)
	{
		if ((one_bit_masks_large[i]&search_buffer)!=0)
		{
			break;
		}
	}

	skipRawBits(i+1);
	search_buffer=getRawBits(i);
	return (1<<i)-1+search_buffer;
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
	
	while (getRawBit() == 0)
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
		return !getRawBit();
	}
}