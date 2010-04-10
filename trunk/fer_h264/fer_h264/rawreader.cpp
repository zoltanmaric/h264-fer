#include "rawreader.h"
#include <stdio.h>

//Decoder variables
unsigned int RBSP_current_byte;
unsigned int RBSP_current_bit;
unsigned int RBSP_total_size;
unsigned char *RBSP_data;

//Coder variables
unsigned int RBSP_write_current_byte;
unsigned int RBSP_write_current_bit;
unsigned int RBSP_write_total_size;
unsigned char *RBSP_write_data;

void initRawReader(unsigned char *RBSP,unsigned int size)
{
	RBSP_current_byte=0;
	RBSP_current_bit=0;
	RBSP_total_size=size;
	RBSP_data=RBSP;
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
	unsigned int count=0;
	while(count<N)
	{
		//8-byte fast forward 
		if ((N-count)>7 && RBSP_write_current_bit==0)
		{
			RBSP_write_data[RBSP_write_current_byte]=0xFF;
			count+=8;
			RBSP_write_current_byte++;
			continue;
		}

		//Classic bit by bit loading
		RBSP_write_data[RBSP_write_current_byte]=	(RBSP_write_data[RBSP_write_current_byte]<<1)+1;
		RBSP_write_current_bit++;
		if (RBSP_write_current_bit==8)
		{
			RBSP_write_current_bit=0;
			RBSP_write_current_byte++;
		}
		count++;
	}
}

void writeZeros(int N)
{
	unsigned int count=0;
	while(count<N)
	{
		//8-byte fast forward 
		if ((N-count)>7 && RBSP_write_current_bit==0)
		{
			RBSP_write_data[RBSP_write_current_byte]=0;
			count+=8;
			RBSP_write_current_byte++;
			continue;
		}

		//Classic bit by bit loading
		RBSP_write_data[RBSP_write_current_byte]=	(RBSP_write_data[RBSP_write_current_byte]<<1);
		RBSP_write_current_bit++;
		if (RBSP_write_current_bit==8)
		{
			RBSP_write_current_bit=0;
			RBSP_write_current_byte++;
		}
		count++;
	}
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
		//8-byte fast forward 
		if ((N-count)>7 && RBSP_write_current_bit==0)
		{
			RBSP_write_data[RBSP_write_current_byte]=data_to_write[count/8];
			count+=8;
			RBSP_write_current_byte++;
			continue;
		}

		//Classic bit by bit loading
		if (CAVLC_table_mode==1)
		{
			offset = 8 - (count%8) - 1;
		}
		else
		{
			offset = N - count - 1;
		}

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

bool more_rbsp_data()
{
	return (RBSP_current_byte < RBSP_total_size - 1);// || ((RBSP_current_byte;
}

void skipRawBits(int N)
{
	RBSP_current_byte+=(RBSP_current_bit+N)/8;
	RBSP_current_bit=(RBSP_current_bit+N)%8;
}

unsigned int peekRawBits(int N)
{
	unsigned int returnValue=0;
	unsigned int count=0;
	//Local copies of index variables, only difference between "peek" and "get".
	int local_RBSP_current_byte=RBSP_current_byte;
	int local_RBSP_current_bit=RBSP_current_bit;

	while(count<N)
	{
		//8-byte fast forward 
		if ((N-count)>7 && local_RBSP_current_bit==0)
		{
			count+=8;
			returnValue=(returnValue<<8) + RBSP_data[local_RBSP_current_byte];
			local_RBSP_current_byte++;
			continue;
		}

		//Classic bit by bit loading
		returnValue=(returnValue*2) + ((RBSP_data[local_RBSP_current_byte]>>(7-local_RBSP_current_bit))&1);
		local_RBSP_current_bit++;
		if (local_RBSP_current_bit==8)
		{
			local_RBSP_current_bit=0;
			local_RBSP_current_byte++;
		}
		count++;
	}

	return returnValue;
}

unsigned int getRawBits(int N)
{
	unsigned int returnValue=0;
	unsigned int count=0;
	while(count<N)
	{
		//8-byte fast forward 
		if ((N-count)>7 && RBSP_current_bit==0)
		{
			count+=8;
			returnValue=(returnValue<<8) + RBSP_data[RBSP_current_byte];
			RBSP_current_byte++;
			continue;
		}

		//Classic bit by bit loading
		returnValue=(returnValue<<1) + ((RBSP_data[RBSP_current_byte]>>(7-RBSP_current_bit))&1);
		RBSP_current_bit++;
		if (RBSP_current_bit==8)
		{
			RBSP_current_bit=0;
			RBSP_current_byte++;
		}
		count++;
	}

	return returnValue;
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