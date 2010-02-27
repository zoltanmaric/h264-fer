#include "inttransform.h"

// (8.5.6)
void transformInverseScan(int list[16], int c[4][4])
{
	for (int i = 0; i < 16; i++)
	{
		int x = ZigZagReordering[i][0];
		int y = ZigZagReordering[i][1];
		c[x][y] = list[i];
	}
}

// (8.5.12)
void scaleAndTransform4x4Residual(int c[4][4], int r[4][4])
{
	const int bitDepth = 8;	// Standard: bitDepth = BitDepthY or BitDepthC depending
							// on whether this process is invoked for luma or chroma
							// residuals. In the baseline profile, the value of both
							// of these is always equal to 8.

	const bool sMbFlag = false;	// Standard: sMbFlag is 	
}

// (8.5.1)
void transformDecoding4x4Luma(int LumaLevel[16])
{
	int c[4][4];

	transformInverseScan(LumaLevel, c);
	
}