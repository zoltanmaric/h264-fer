// fer_h264.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "nal.h"
#include "rbsp_decoding.h"
#include "rawreader.h"


int _tmain(int argc, _TCHAR* argv[])
{
	FILE *stream;
	stream=fopen("bourne.264","rb");

	unsigned long int ptr=0;
	while(1)
	{
		NALunit nu = getNAL(stream, &ptr);

		if (nu.NumBytesInRBSP==0)
		{
			break;
		}

	
		initRawReader(nu.rbsp_byte, nu.NumBytesInRBSP);
		RBSP_decode(nu);
	}		
	
	return 0;
}

