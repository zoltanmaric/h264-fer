#include "rawreader.h"
#include <stdio.h>

//Decoder variables
unsigned int RBSP_current_byte;
unsigned int RBSP_current_bit;

unsigned int RBSP_bit;

unsigned int one_bit_masks[8]=
{
	0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
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
	RBSP_write_current_byte=0;
	RBSP_write_current_bit=0;
	RBSP_write_total_size=size;
	RBSP_write_data=RBSP_write;
}

// No return value, since the size of the "uint_number" (in bits) is known to caller
// rbsp_result byte 0 is the LSB and byte 3 is the MSB
void UINT_to_RBSP_size_known(unsigned long int uint_number, unsigned int size, unsigned char rbsp_result[4])
{
	rbsp_result[3]=(uint_number>>24)&0xFF;
	rbsp_result[2]=(uint_number>>16)&0xFF;
	rbsp_result[1]=(uint_number>>8)&0xFF;
	rbsp_result[0]=(uint_number)&0xFF;
}

// Return value is the size of "uint_number" in bits
// rbsp_result byte 0 is the LSB and byte 3 is the MSB
unsigned int UINT_to_RBSP_size_unknown(unsigned long int uint_number, unsigned char rbsp_result[4])
{
	int rbsp_length=0;
	unsigned char *new_rbsp_result=new unsigned char[4];
	new_rbsp_result[0]=0;
	rbsp_result=new_rbsp_result;
	if (uint_number==0)
	{
		return 0;
	}
	else
	{
		new_rbsp_result[3]=(uint_number>>24)&0xFF;
		new_rbsp_result[2]=(uint_number>>16)&0xFF;
		new_rbsp_result[1]=(uint_number>>8)&0xFF;
		new_rbsp_result[0]=(uint_number)&0xFF;


		//Let's count the number of bits that make this integer (important only for return value)
		int length=0;

		while (uint_number>0)
		{
			if ((uint_number>>8)==0)
			{
				int bit_mask=128;
				int bit_counter=8;
				while ((bit_mask & uint_number) == 0)
				{
					bit_counter--;
					bit_mask=bit_mask>>1;
				}
				length+=bit_counter;
			}
			else
			{
				length+=8;
			}
			
			uint_number>>8;
		}

		return length;
	}
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

void dumpWriteBuffer()
{
	for (int i=0;i<=RBSP_write_current_byte;i++)
	{
		printf("%d%d%d%d%d%d%d%d - ",(RBSP_write_data[i]&128)>>7,(RBSP_write_data[i]&64)>>6,(RBSP_write_data[i]&32)>>5,
			(RBSP_write_data[i]&16)>>4,(RBSP_write_data[i]&8)>>3,(RBSP_write_data[i]&4)>>2,(RBSP_write_data[i]&2)>>1,
			(RBSP_write_data[i]&1));
		printf("%x\n",RBSP_write_data[i]);
	}

	printf("Ending bit: %d\n",RBSP_write_current_bit);
}

bool writeRawBits(int N, unsigned char *data_to_write, int CAVLC_table_mode)
{
	unsigned int count=0, offset;
	while(count<N)
	{
		if (CAVLC_table_mode==1)
		{
			//offset = 8 - (count%8) - 1;
			// TEST:
			offset = (8 - (N & 7)) & 7;
			unsigned int data = 0;
			while (count < N)
			{
				data |= data_to_write[count/8];
				count += 8;
				if (count < N)
				{
					data <<= 8;
				}
			}
			// The data in the CAVLC tables is aligned to
			// the left, so it has to be moved to
			// correspond to the right value.
			data >>= offset;
			return writeRawBits(N, data);
		}
		else
		{
			offset = N - count - 1;
		}

		//8-byte fast forward 
		if ((N-count)>7 && RBSP_write_current_bit==0 && (N-count)&7 == 0)
		{
			//int shift = offset + 1;
			RBSP_write_data[RBSP_write_current_byte]=data_to_write[offset>>3];
			count+=8;
			RBSP_write_current_byte++;
			continue;
		}

		//Classic bit by bit loading
		RBSP_write_data[RBSP_write_current_byte]=	(RBSP_write_data[RBSP_write_current_byte]<<1) + ((data_to_write[offset/8]>>(offset%8))&1);
		RBSP_write_current_bit++;
		if (RBSP_write_current_bit==8)
		{
			RBSP_write_current_bit=0;
			RBSP_write_current_byte++;
		}
		count++;
	}

	return true;
}

// Write up to 32 bits of data
bool writeRawBits(int N, unsigned int data)
{
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
	IMPORTANT:	This functions (currently) always receives requests with N=24.
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
		RBSP_buffer=(RBSP_data[RBSP_current_byte]<<32)
		| (RBSP_data[RBSP_current_byte+1]<<24)
		| (RBSP_data[RBSP_current_byte+2]<<16)
		| (RBSP_data[RBSP_current_byte+3]<<8)
		| (RBSP_data[RBSP_current_byte+4]);
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

	int predict_bytes;
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
		RBSP_buffer=(RBSP_data[RBSP_current_byte]<<32)
		| (RBSP_data[RBSP_current_byte+1]<<24)
		| (RBSP_data[RBSP_current_byte+2]<<16)
		| (RBSP_data[RBSP_current_byte+3]<<8)
		| (RBSP_data[RBSP_current_byte+4]);
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

unsigned int RBSPtoUINT(unsigned char *rbsp, int N)
{
	unsigned int returnValue=0;

	if (N<9)
	{
		return *rbsp&((1<<N)-1);
	}
	else if (N<17)
	{
		return *(rbsp)&(((1<<(N-8))-1)<<8) + *(rbsp+1);
	}
	else if (N<25)
	{
		return *(rbsp)&(((1<<(N-16))-1)<<16) + (*(rbsp+1)<<8) + *(rbsp+2);
	}
	else
	{
		return *(rbsp)&(((1<<(N-24))-1)<<24) + (*(rbsp+1)<<16) + (*(rbsp+2)<<8) + *(rbsp+3);
	}
}