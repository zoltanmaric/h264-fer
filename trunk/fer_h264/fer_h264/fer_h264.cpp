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
	stream=fopen("big_buck_bunny.264","rb");
	yuvoutput = fopen("Bourne.yuv","wb");

	NALunit nu;
	nu.rbsp_byte = new unsigned char[500000];

	unsigned long int ptr=0;
	while(1)
	{
		getNAL(&ptr, nu);

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
	stream = fopen("big_buck_bunny.264", "wb");
	yuvinput = fopen("big_buck_bunny.y4m", "rb");
	yuvoutput = fopen("Bourne.yuv","wb");

	frameCount = 0;
	NALunit nu;
	nu.rbsp_byte = new unsigned char[500000];


	nu.forbidden_zero_bit = 0;

	loadY4MHeader();

	// write sequence parameter set:
	nu.nal_ref_idc = 1;			// non-zero for sps
	nu.nal_unit_type = NAL_UNIT_TYPE_SPS;
	RBSP_encode(nu);

	writeNAL(nu);

	// write picture paramater set:
	nu.nal_ref_idc = 1;			// non-zero for pps
	nu.nal_unit_type = NAL_UNIT_TYPE_PPS;
	RBSP_encode(nu);

	writeNAL(nu);
	
	nu.nal_ref_idc = 1;		// non-zero for reference
	while (readFromY4M() != -1)
	{		
		frameCount++;

		printf("Frame #%d\n", frameCount);

		if(frameCount % 30 == 1)
		{
			nu.nal_unit_type = NAL_UNIT_TYPE_IDR;
		}
		else
		{
			nu.nal_unit_type = NAL_UNIT_TYPE_NOT_IDR;
		}
		RBSP_encode(nu);

		writeNAL(nu);

		if (frameCount == 5) break;

		//writeToY4M();
	}

	fclose(stream);
	fclose(yuvinput);
	fclose(yuvoutput);
}

int _tmain(int argc, _TCHAR* argv[])
{
	decode();
	//encode();

	return 0;
}

