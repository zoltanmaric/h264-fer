#include "rawreader.h"

unsigned int RBSP_current_byte;
unsigned int RBSP_current_bit;
unsigned int RBSP_total_size;
unsigned char *RBSP_data;

void initRawReader(unsigned char *RBSP,unsigned int size)
{
	RBSP_current_byte=0;
	RBSP_current_bit=0;
	RBSP_total_size=size;
	RBSP_data=RBSP;
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