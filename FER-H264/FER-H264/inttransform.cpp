#include "inttransform.h"
#include "h264_math.h"

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

// Haramustek
// (8.5.12.1 & 8.5.12.2)
void inverseResidual(int bitDepth, int qP, int c[4][4], int r[4][4], bool luma)
{

}

// (8.5.12)
// luma: true if process invoked for luma residuals
// QPy_prev: the luma quantization parameter for the
// previously transformed macroblock. At the start
// of each slice, it is initialized to SliceQPY
// derived in Equation 7-29
void scaleAndTransform4x4Residual(int c[4][4], int r[4][4], bool luma, int *QPy_prev)
{
	int qP, QPy, QP_y, QPc, QP_c;

	const int bitDepth = 8;	// Standard: bitDepth = BitDepthY or BitDepthC depending
							// on whether this process is invoked for luma or chroma
							// residuals. In the baseline profile, the value of both
							// of these is always equal to 8.

	const bool sMbFlag = false;	// Standard: sMbFlag is true when mb_type is equal to
								// SI or SP. This is never the case in baseline since
								// only I and P slices are allowed.

	// Standard: because sMbFlag is always false in baseline, qP is never
	// equal to QSy or QSc
	if (luma)
	{
		int QpBdOffsetY = 0;	// Standard: = 6 * bit_depth_luma_minus8; bit_depth_luma_minus_8 == 0 in baseline
		QPy = ((*QPy_prev + mb_qp_delta + 52 + 2*QpBdOffsetY) % (52 + QpBdOffsetY)) - QpBdOffsetY;
		int QP_y = QPy + QpBdOffsetY;

		qP = QP_y;
	}
	else
	{
		int QpBdOffsetC = 0;	// Standard: = 6 * bit_depth_chroma_minus8; bit_depth_chroma_minus_8 == 0 in baseline
		int qPoffset = chroma_qp_index_offset;	// Standard: qPoffset = second_chroma_qp_index_offset,
												// second_chroma_qp_index_offset == chroma_qp_index_offset when not
												// present. It is not present in baseline.
		int qPi = Clip3(-QpBdOffsetC, 51, QPy + qPoffset);
		QPc = qPiToQPc[qPi];
		QP_c = QPc + QpBdOffsetC;

		qP = QP_c;
	}

	// TransformBypassModeFlag == 0 in baseline

	inverseResidual(bitDepth, qP, c, r, luma);

}

// (8.5.1)
void transformDecoding4x4Luma(int LumaLevel[16])
{
	int c[4][4], r[4][4];

	transformInverseScan(LumaLevel, c);

	scaleAndTransform4x4Residual(c, r, true, 0);	
}