// fer_h264.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "nal.h"
#include "fileIO.h"
#include "rbsp_decoding.h"
#include "rawreader.h"
#include "h264_globals.h"

void decode()
{
	FILE *stream;
	stream=fopen("bourne.264","rb");
	yuvoutput = fopen("Bourne.yuv","wb");

	NALunit nu;
	nu.rbsp_byte = new unsigned char[500000];

	unsigned long int ptr=0;
	while(1)
	{
		getNAL(stream, &ptr, nu);

		if (nu.NumBytesInRBSP==0)
		{
			break;
		}

		RBSP_decode(nu);
	}		

	fclose(stream);
	fclose(yuvoutput);
}

void encode()
{
	FILE *stream;
	stream = fopen("big_buck_bunny.264", "wb");
	yuvinput = fopen("big_buck_bunny.y4m", "rb");
	yuvoutput = fopen("Bourne.yuv","wb");

	frameCount = 0;
	while (readFromY4M() != -1)
	{		
		frameCount++;
		writeToY4M();
	}
}


int _tmain(int argc, _TCHAR* argv[])
{
	decode();
	//encode();

	return 0;
}

