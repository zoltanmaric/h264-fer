// fer_h264.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "nal.h"
#include "fileIO.h"
#include "rbsp_decoding.h"
#include "rbsp_IO.h"
#include "h264_globals.h"
#include "residual_tables.h"
#include "ref_frames.h"
#include "expgolomb.h"
#include "rbsp_encoding.h"
#include "moestimation.h"

void decode()
{
	stream=fopen("big_buck_bunny.264","rb");
	yuvoutput = fopen("Bourne.y4m","wb");

	generate_residual_level_tables();

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
	yuvoutput = fopen("reference.yuv","wb");

	generate_residual_level_tables();
	init_expgolomb_UC_codes();

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
	InitializeInterpolatedRefFrame();
	while (readFromY4M() != -1)
	{		
		frameCount++;
		if (frameCount < 1) continue;

		printf("Frame #%d\n", frameCount);
		writeToYUV();

		nu.nal_unit_type = selectNALUnitType();
		RBSP_encode(nu);

		writeNAL(nu);
		//FillInterpolatedRefFrame();
		//for (int frac = 0; frac < 16; frac++)
		//{
		//	for (int i = 0; i < frame.Lheight; i++)
		//		for (int j = 0; j < frame.Lwidth; j++)
		//		{
		//			int tmp = frame.L[i][j]; frame.L[i][j] = refFrameInterpolated[frac].L[i][j]; refFrameInterpolated[frac].L[i][j] = tmp;
		//		}
		//	for (int i = 0; i < frame.Cheight; i++)
		//		for (int j = 0; j <  frame.Cwidth; j++)
		//		{
		//			int tmp = frame.C[0][i][j]; frame.C[0][i][j] = refFrameInterpolated[frac].C[0][i][j]; refFrameInterpolated[frac].C[0][i][j] = tmp;
		//				tmp = frame.C[1][i][j]; frame.C[1][i][j] = refFrameInterpolated[frac].C[1][i][j]; refFrameInterpolated[frac].C[1][i][j] = tmp;
		//		}
		//	char a = 'A' + frac;
		//	char name[20] = "interpolated_fracA";
		//	name[17] += frac;
		//	writeToPPM(name);
		//	for (int i = 0; i < frame.Lheight; i++)
		//		for (int j = 0; j <  frame.Lwidth; j++)
		//		{
		//			int tmp = frame.L[i][j]; frame.L[i][j] = refFrameInterpolated[frac].L[i][j]; refFrameInterpolated[frac].L[i][j] = tmp;
		//		}
		//	for (int i = 0; i < frame.Cheight; i++)
		//		for (int j = 0; j <  frame.Cwidth; j++)
		//		{
		//			int tmp = frame.C[0][i][j]; frame.C[0][i][j] = refFrameInterpolated[frac].C[0][i][j]; refFrameInterpolated[frac].C[0][i][j] = tmp;
		//				tmp = frame.C[1][i][j]; frame.C[1][i][j] = refFrameInterpolated[frac].C[1][i][j]; refFrameInterpolated[frac].C[1][i][j] = tmp;
		//		}
		//}
		//writeToPPM("reconstruct");

		if (frameCount == 1) break;
	}

	fclose(stream);
	fclose(yuvinput);
	fclose(yuvoutput);
}

int _tmain(int argc, _TCHAR* argv[])
{
	//decode();
	encode();

	return 0;
}

