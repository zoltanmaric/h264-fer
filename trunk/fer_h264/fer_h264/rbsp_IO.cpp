#include "rbsp_IO.h"
#include <stdio.h>

//Decoder variables
unsigned int RBSP_current_byte;
unsigned int RBSP_current_bit;
unsigned int bitcount;

unsigned int RBSP_bit;

unsigned int one_bit_masks[8]=
{
	0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

unsigned int RBSP_high_masks[8]=
{
	0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE
};

unsigned int RBSP_masks[33]=
{
	0xFFFFFFFF, 
	0x7FFFFFFF, 0x3FFFFFFF, 0x1FFFFFFF, 0x00FFFFFF, 0x07FFFFFF, 0x03FFFFFF, 0x01FFFFFF, 0x00FFFFFF,
	0x7FFFFF, 0x3FFFFF, 0x1FFFFF, 0x00FFFF, 0x07FFFF, 0x03FFFF, 0x01FFFF, 0x00FFFF, 
	0x7FFF, 0x3FFF, 0x1FFF, 0x0FFF, 0x07FF, 0x03FF, 0x01FF, 0x00FF,
	0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01, 0x00
};

unsigned int RBSP_levels[33]=
{
	0, 8, 16, 16, 16, 16, 16, 16, 16, 16,
	24, 24, 24, 24, 24, 24, 24, 24, 32, 32,
	32, 32, 32, 32, 32, 32, 40, 40, 40, 40,
	40, 40, 40
};

unsigned int RBSP_total_size;
unsigned char *RBSP_data;

//Write buffer
unsigned long long int RBSP_write_buffer;
unsigned int RBSP_write_buffer_bit;
unsigned int aligned_block;

//Coder variables
unsigned int RBSP_write_current_byte;
unsigned int RBSP_write_current_bit;
unsigned int RBSP_write_total_size;
unsigned char *RBSP_write_data;

unsigned long long int RBSP_buffer;

void initRawReader(unsigned char *RBSP,unsigned int size)
{
	RBSP_current_byte=0;
	RBSP_current_bit=0;
	RBSP_total_size=size;
	RBSP_data=RBSP;

	RBSP_bit=7;
}

void initRawWriter(unsigned char *RBSP_write, unsigned int size)
{
	bitcount=0;

	RBSP_write_buffer=0;
	RBSP_write_buffer_bit=0;

	RBSP_write_current_byte=0;
	RBSP_write_current_bit=0;
	RBSP_write_total_size=size;
	RBSP_write_data=RBSP_write;
}

void writeFlag(int flag)
{
	if (flag==1)
	{
		writeOnes(1);
	}
	else
	{
		writeZeros(1);
	}
}

void writeOnesDirect(int N)
{
	unsigned int data = (1 << N) - 1;
	writeRawBitsDirect(N, data);
}

void writeZerosDirect(int N)
{
	unsigned int data = 0;
	writeRawBitsDirect(N, data);
}

void writeOnes(int N)
{
	unsigned int data = (1 << N) - 1;
	writeRawBits(N, data);
}

void writeZeros(int N)
{
	unsigned int data = 0;
	writeRawBits(N, data);
}

void flushWriteBuffer()
{
	if (RBSP_write_buffer_bit!=0)
	{
		writeRawBitsDirect(RBSP_write_buffer_bit, (unsigned int)RBSP_write_buffer);
		RBSP_write_buffer_bit=0;
		RBSP_write_buffer=0;
	}
}

void writeRawBits(int N, unsigned int data)
{
	RBSP_write_buffer = RBSP_write_buffer << N;
	RBSP_write_buffer_bit += N;
	RBSP_write_buffer += data;

	if (RBSP_write_buffer_bit>=32)
	{
		aligned_block=(unsigned int)(RBSP_write_buffer>>(RBSP_write_buffer_bit-32));
		if (RBSP_write_current_bit == 0)
		{
			//Perfect 32-bit align
			RBSP_write_data[RBSP_write_current_byte]=(aligned_block>>24)&0xFF;
			RBSP_write_data[RBSP_write_current_byte+1]=(aligned_block>>16)&0xFF;
			RBSP_write_data[RBSP_write_current_byte+2]=(aligned_block>>8)&0xFF;
			RBSP_write_data[RBSP_write_current_byte+3]=aligned_block&0xFF;
		}
		else
		{
			//RBSP_current_bit is not aligned for a perfect write
			RBSP_write_data[RBSP_write_current_byte] &= RBSP_high_masks[RBSP_write_current_bit];
			RBSP_write_data[RBSP_write_current_byte] |= (aligned_block >> (32+RBSP_write_current_bit-8)) & 0xFF;
			RBSP_write_data[RBSP_write_current_byte+1] = (aligned_block >> (32+RBSP_write_current_bit-16)) & 0xFF;
			RBSP_write_data[RBSP_write_current_byte+2] = (aligned_block >> (32+RBSP_write_current_bit-24)) & 0xFF;
			RBSP_write_data[RBSP_write_current_byte+3] = (aligned_block >> (32+RBSP_write_current_bit-32)) & 0xFF;
			RBSP_write_data[RBSP_write_current_byte+4] = (aligned_block & 0xFF) << (8-RBSP_write_current_bit);
		}
		RBSP_write_current_byte += 4;
		RBSP_write_buffer_bit -= 32;
	}
}

// Write up to 32 bits of data
bool writeRawBitsDirect(int N, unsigned int data)
{
	bitcount+=N;
	int count = 0;
	while (count < N)
	{
		if (RBSP_write_current_bit == 0)
		{
			// Clear the current byte.
			RBSP_write_data[RBSP_write_current_byte] = 0;
		}

		int difference = N - count;
		int bitsLeft = 8 - RBSP_write_current_bit;
		if (bitsLeft >= difference)
		{
			RBSP_write_data[RBSP_write_current_byte] |= data << (bitsLeft - difference);
			count += difference;
			RBSP_write_current_bit += difference;
		}
		else
		{
			RBSP_write_data[RBSP_write_current_byte] |= (data >> (difference - bitsLeft)) & 0xff;
			count += bitsLeft;
			RBSP_write_current_bit = 8;
		}
		
		
		RBSP_write_current_byte += RBSP_write_current_bit >> 3; // RBSP_write_current_bit / 8
		RBSP_write_current_bit &= 7;	// RBSP_write_current_bit %= 8;

	}

		bitcount=RBSP_write_current_byte*8+RBSP_write_current_bit;
	return true;
}

bool more_rbsp_data()
{
	return (RBSP_current_byte < RBSP_total_size - 1);// || ((RBSP_current_byte;
}

void skipRawBits(int N)
{
	RBSP_current_byte+=(RBSP_current_bit+N)/8;
	RBSP_current_bit=(RBSP_current_bit+N)%8;
	RBSP_bit=7-RBSP_current_bit;
}


/*
	IMPORTANT:	This function (currently) always receives requests with N=24.
	Therefore I added the initial "if" optimization, which should be removed in the later, differing, implementations.
*/
unsigned int peekRawBits(int N)
{
	RBSP_buffer=0;

	if (N==24)
	{
		RBSP_buffer=(RBSP_data[RBSP_current_byte]<<24)
		| (RBSP_data[RBSP_current_byte+1]<<16)
		| (RBSP_data[RBSP_current_byte+2]<<8)
		| (RBSP_data[RBSP_current_byte+3]);
		return (unsigned int)(RBSP_buffer>>(8-RBSP_current_bit))& 0x00FFFFFF;
	}

	if (N<=9)
	{
		RBSP_buffer=(RBSP_data[RBSP_current_byte]<<8)
		| (RBSP_data[RBSP_current_byte+1]);
	}
	else if (N<=17)
	{
		RBSP_buffer=(RBSP_data[RBSP_current_byte]<<16)
		| (RBSP_data[RBSP_current_byte+1]<<8)
		| (RBSP_data[RBSP_current_byte+2]);
	}
	else if (N<=25)
	{
		RBSP_buffer=(RBSP_data[RBSP_current_byte]<<24)
		| (RBSP_data[RBSP_current_byte+1]<<16)
		| (RBSP_data[RBSP_current_byte+2]<<8)
		| (RBSP_data[RBSP_current_byte+3]);
	}
	else
	{
		RBSP_buffer=((unsigned long)RBSP_data[RBSP_current_byte]<<32)
		| ((unsigned long)RBSP_data[RBSP_current_byte+1]<<24)
		| ((unsigned long)RBSP_data[RBSP_current_byte+2]<<16)
		| ((unsigned long)RBSP_data[RBSP_current_byte+3]<<8)
		| ((unsigned long)RBSP_data[RBSP_current_byte+4]);
	}

	return (unsigned int)(RBSP_buffer>>(RBSP_levels[N]-(N+RBSP_current_bit)))& RBSP_masks[32-N];	
}


unsigned int getRawBit()
{
	unsigned int bitValue=RBSP_data[RBSP_current_byte]& one_bit_masks[RBSP_current_bit];

	RBSP_current_bit++;
	RBSP_current_byte+=(RBSP_current_bit>>3);
	RBSP_current_bit&=0x07;

	return (unsigned int)(bitValue>0);
}



/*
	IMPORTANT: This function usually gets requests up to N=12 (longest residual suffix, prefix is read bit-for-bit).
	When reading data larger then unsigned integer (currently 32 bits) it's necessary to adjust/expand RBSP_levels
	and RBSP_masks.
*/ 

unsigned int getRawBits(int N)
{
	if (N==0)
	{
		return 0;
	}

	RBSP_bit=7-RBSP_current_bit;

	RBSP_buffer=0;
	if (N<=9)
	{
		RBSP_buffer=(RBSP_data[RBSP_current_byte]<<8)
		| (RBSP_data[RBSP_current_byte+1]);
	}
	else if (N<=17)
	{
		RBSP_buffer=(RBSP_data[RBSP_current_byte]<<16)
		| (RBSP_data[RBSP_current_byte+1]<<8)
		| (RBSP_data[RBSP_current_byte+2]);
	}
	else if (N<=25)
	{
		RBSP_buffer=(RBSP_data[RBSP_current_byte]<<24)
		| (RBSP_data[RBSP_current_byte+1]<<16)
		| (RBSP_data[RBSP_current_byte+2]<<8)
		| (RBSP_data[RBSP_current_byte+3]);
	}
	else
	{
		RBSP_buffer=((unsigned long)RBSP_data[RBSP_current_byte]<<32)
		| ((unsigned long)RBSP_data[RBSP_current_byte+1]<<24)
		| ((unsigned long)RBSP_data[RBSP_current_byte+2]<<16)
		| ((unsigned long)RBSP_data[RBSP_current_byte+3]<<8)
		| ((unsigned long)RBSP_data[RBSP_current_byte+4]);
	}

	//Didn't read into next the RBSP byte, easier housekeeping
	if ((int)(N-RBSP_bit-1)<=0)
	{
		RBSP_buffer=(RBSP_data[RBSP_current_byte]>>(1+RBSP_bit-N))&RBSP_masks[32-N];
		RBSP_current_bit+=N;
		RBSP_current_byte+=(RBSP_current_bit>>3);
		RBSP_current_bit=RBSP_current_bit&0x7;
	}
	//Read into unknown number of next RBSP bytes, longer housekeeping
	else
	{
		RBSP_buffer=(RBSP_buffer>>(RBSP_levels[N]-(N+RBSP_current_bit)))& RBSP_masks[32-N];
		RBSP_current_byte+=1+((N-RBSP_bit-1)>>3);
		RBSP_current_bit=(N-RBSP_bit-1)&0x7;
	}
	return (unsigned int)RBSP_buffer;
}