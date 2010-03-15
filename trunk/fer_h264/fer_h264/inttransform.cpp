#include "inttransform.h"
#include "h264_math.h"
#include "scaleTransform.h"
#include "h264_globals.h"
#include "headers_and_parameter_sets.h"

// Specification of QPc as a function of qPi (Table 8-15)
int qPiToQPc[52] = { 0,  1,  2,  3,  4,  5,  6,  7, 
					 8,  9, 10, 11,	12, 13, 14, 15,
					16, 17, 18, 19, 20, 21, 22, 23,
					24, 25, 26, 27, 28, 29, 29, 30,
					31, 32, 32, 33, 34, 34, 35, 35,
					36, 36, 37, 37, 37, 38, 38, 38,
					39, 39, 39, 39};

// (Figure 8-9a)
//int ZigZagReordering[16][2] = 
//{
//	{0,0}, {0,1}, {1,0}, {2,0}, {1,1}, {0,2}, {0,3}, {1,2},
//	{2,1}, {3,0}, {3,1}, {2,2}, {1,3}, {2,3}, {3,2}, {3,3}
//};

// Haramustek
// (8.5.12.1 & 8.5.12.2)
//void inverseResidual(int bitDepth, int qP, int c[4][4], int r[4][4], bool luma)
//{
//
//}

// (8.5.12)
// luma: true if process invoked for luma residuals
// QPy_prev: the luma quantization parameter for the
// previously transformed macroblock. At the start
// of each slice, it is initialized to SliceQPY
// derived in Equation 7-29
void scaleAndTransform4x4Residual(int c[4][4], int r[4][4], bool intra16x16OrChroma, int QPy, bool luma)
{
	int qP;

	const int bitDepth = 8;	// Norm: bitDepth = BitDepthY or BitDepthC depending
							// on whether this process is invoked for luma or chroma
							// residuals. In the baseline profile, the value of both
							// of these is always equal to 8.

	// Norm: sMbFlag is true when mb_type is equal to
	// SI or SP. This is never the case in baseline since
	// only I and P slices are allowed.

	// Norm: because sMbFlag is always false in baseline, qP is never
	// equal to QSy or QSc
	if (luma)
	{
		// Norm: QpBdOffsetY == 6 * bit_depth_luma_minus8; bit_depth_luma_minus_8 == 0 in baseline
		int QP_y = QPy;
		qP = QP_y;
	}
	else
	{
		int QpBdOffsetC = 0;	// Norm: = 6 * bit_depth_chroma_minus8; bit_depth_chroma_minus_8 == 0 in baseline
		int qPoffset = pps.chroma_qp_index_offset;	// Norm: qPoffset = second_chroma_qp_index_offset,
												// second_chroma_qp_index_offset == chroma_qp_index_offset when not
												// present. It is not present in baseline.
		int qPi = Clip3(-QpBdOffsetC, 51, QPy + qPoffset);
		int QPc = qPiToQPc[qPi];
		int QP_c = QPc + QpBdOffsetC;

		qP = QP_c;
	}

	// TransformBypassModeFlag == 0 in baseline

	inverseResidual(bitDepth, qP, c, r, intra16x16OrChroma);

	// MEGATEST:
	//if (intra16x16OrChroma == false)
	//{
	//	for (int i = 0; i < 4; i++)
	//	{
	//		for (int j = i + 1; j < 4; j++)
	//		{
	//			int temp = r[i][j];
	//			r[i][j] = r[j][i];
	//			r[j][i] = temp;
	//		}
	//	}
	//}

	int test = 0;
}

// (8.5.14) partial
void pictureConstruction4x4Luma(int u[4][4], int luma4x4BlkIdx)
{
	int xP = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 0);
	int yP = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 1);

	int x0 = InverseRasterScan(luma4x4BlkIdx / 4, 8, 8, 16, 0) + InverseRasterScan(luma4x4BlkIdx % 4, 4, 4, 8, 0);
	int y0 = InverseRasterScan(luma4x4BlkIdx / 4, 8, 8, 16, 1) + InverseRasterScan(luma4x4BlkIdx % 4, 4, 4, 8, 1);

	// Norm: MbAffFrameFlag == 0 in baseline

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			frame.L[(yP+y0+i)*frame.Lwidth + (xP+x0+j)] = u[i][j];
		}
	}
}

// (8.5.14) partial
void pictureConstructionIntra_16x16Luma(int u[16][16])
{
	int xP = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 0);
	int yP = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 1);

	int x0 = 0;
	int y0 = 0;

	// Norm: MbAffFrameFlag == 0 in baseline

	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			frame.L[(yP+y0+i)*frame.Lwidth + (xP+x0+j)] = u[i][j];
		}
	}
}

// (8.5.14) partial
void pictureConstructionChroma(int u[8][8], bool Cb)
{
	int xP = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 0);
	int yP = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 1);
	
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			int nW = 8, nH = 8;	// Norm: MbWidthC == MbHeightC == 8 in baseline.
			int x0 = 0, y0 = 0;

			// Norm: MbAffFrameFlag == 0 in baseline
			// Norm: subWidthC == SubHeightC == 2 in baseline
			if (Cb)
			{
				frame.C[0][((yP/2)+y0+i)*frame.Cwidth + (xP/2)+x0+j] = u[i][j];
			}
			else
			{
				frame.C[1][((yP/2)+y0+i)*frame.Cwidth + (xP/2)+x0+j] = u[i][j];
			}
		}
	}
}

// (8.5.1)
// QPy_prev: the luma quantization parameter for the
// previously transformed macroblock. At the start
// of each slice, it is initialized to SliceQPY
// derived in Equation 7-29
void transformDecoding4x4LumaResidual(int LumaLevel[16][16], int predL[16][16], int luma4x4BlkIdx, int QPy)
{
	int c[4][4], r[4][4], u[4][4];

	transformInverseScan(LumaLevel[luma4x4BlkIdx], c);
	scaleAndTransform4x4Residual(c, r, false, QPy, true);

	int x0 = InverseRasterScan(luma4x4BlkIdx / 4, 8, 8, 16, 0) + InverseRasterScan(luma4x4BlkIdx % 4, 4, 4, 8, 0);
	int y0 = InverseRasterScan(luma4x4BlkIdx / 4, 8, 8, 16, 1) + InverseRasterScan(luma4x4BlkIdx % 4, 4, 4, 8, 1);

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			u[i][j] = Clip1Y(predL[y0+i][x0+j] + r[i][j]);
		}
	}

	// Norm: TransformBypassModeFlag == 0 in baseline

	pictureConstruction4x4Luma(u, luma4x4BlkIdx);
}

// (8.5.2)
void transformDecodingIntra_16x16Luma(int Intra16x16DCLevel[16], int Intra16x16ACLevel[16][16],int predL[16][16], int QPy)
{
	int c[4][4], dcY[4][4], rMb[16][16], r[4][4], u[16][16];

	transformInverseScan(Intra16x16DCLevel, c);
	int QP_y = QPy;

	InverseDCLumaIntra(8, QP_y, c, dcY);

	int lumaList[16];
	for (int luma4x4BlkIdx = 0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
	{
		int x = InverseRasterScan(luma4x4BlkIdx / 4, 2, 2, 4, 0) +
				InverseRasterScan(luma4x4BlkIdx % 4, 1, 1, 2, 0);
		int y = InverseRasterScan(luma4x4BlkIdx / 4, 2, 2, 4, 1) +
				InverseRasterScan(luma4x4BlkIdx % 4, 1, 1, 2, 1);
		lumaList[0] = dcY[y][x];
		for (int k = 1; k < 16; k++)
		{
			lumaList[k] = Intra16x16ACLevel[luma4x4BlkIdx][k-1];
		}

		transformInverseScan(lumaList, c);
		scaleAndTransform4x4Residual(c, r, true, QPy, true);
		int x0 = InverseRasterScan(luma4x4BlkIdx / 4, 8, 8, 16, 0) + InverseRasterScan(luma4x4BlkIdx % 4, 4, 4, 8, 0);
		int y0 = InverseRasterScan(luma4x4BlkIdx / 4, 8, 8, 16, 1) + InverseRasterScan(luma4x4BlkIdx % 4, 4, 4, 8, 1);

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				rMb[y0+i][x0+j] = r[i][j];
			}
		}
	}

	// Norm: TransformBypassModeFlag == 0 in baseline

	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			// Norm:
			// u(i,j) = Clip1Y(predL[j][i] + rMb[j][i])
			// this is inverted because the first index
			// corresponds to the x coordinate in the
			// standard.
			u[i][j] = Clip1Y(predL[i][j] + rMb[i][j]);
		}
	}

	pictureConstructionIntra_16x16Luma(u);
}

// This function is not explicitly defined in the norm.
// It is a wrapper for the invocation of 4x4 luma residual
// decoding process which is to be invoked for P_Skip
// macroblocks with LumaLevel, ChromACLevel and ChromaDCLevel
// equal to 0, as defined by the norm.
void transformDecodingP_Skip(int predL[16][16], int predCb[8][8], int predCr[8][8], int QPy)
{
	// Norm: When the current macroblock is coded as P_Skip or B_Skip, all values of LumaLevel, LumaLevel8x8, CbLevel,
	// CbLevel8x8, CrLevel, CrLevel8x8, ChromaDCLevel, ChromaACLevel are set equal to 0 for the current macroblock.
	int LumaLevel[16][16] = {0};
	for(int luma4x4BlkIdx = 0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
	{
		transformDecoding4x4LumaResidual(LumaLevel, predL, luma4x4BlkIdx, QPy);
	}

	int ChromaDCLevel[4] = {0};
	int ChromaACLevel[4][16] = {0};
	transformDecodingChroma(ChromaDCLevel, ChromaACLevel, predCb, QPy, true);
	transformDecodingChroma(ChromaDCLevel, ChromaACLevel, predCr, QPy, false);
}

// (8.5.4)
// ChromaDCLevel corresponds to either ChromaDCLevel[0] or
// ChromaDCLevel[1] depending on whether the process is
// invoked for Cr or Cb. The same applies for ChromaACLevel.
// This implies that this process is invoked once for each
// chroma component.
void transformDecodingChroma(int ChromaDCLevel[4], int ChromaACLevel[4][16], int predC[8][8], int QPy, bool Cb)
{
	int numChroma4x4Blks = 4;	// Norm: = (MbWidthC/4) * (MbHeightC/4);
								// MbWidthC == MbHeightC == 8 in baseline.

	// Norm: When the current macroblock is coded as P_Skip or B_Skip, all values of LumaLevel, LumaLevel8x8, CbLevel,
	// CbLevel8x8, CrLevel, CrLevel8x8, ChromaDCLevel, ChromaACLevel are set equal to 0 for the current macroblock.
	if (mb_type == P_Skip)
	{
		for (int i = 0; i < 4; i++)
		{
			ChromaDCLevel[i] = 0;
			for (int j = 0; j < 16; j++)
			{
				ChromaACLevel[i][j] = 0;
			}
		}
	}

	// Norm: ChromaArrayType == 1 in baseline.
	int c[2][2];
	for (int i = 0; i < 4; i++)
	{
		c[i/2][i%2] = ChromaDCLevel[i];
	}

	int QpBdOffsetC = 0;	// Norm: = 6 * bit_depth_chroma_minus8; bit_depth_chroma_minus_8 == 0 in baseline
	int qPoffset = pps.chroma_qp_index_offset;	// Norm: qPoffset = second_chroma_qp_index_offset,
												// second_chroma_qp_index_offset == chroma_qp_index_offset when not
												// present. It is not present in baseline.
	int qPi = Clip3(-QpBdOffsetC, 51, QPy + qPoffset);
	int QPc = qPiToQPc[qPi];
	int QP_c = QPc + QpBdOffsetC;

	int qP = QP_c;

	int dcC[2][2];
	InverseDCChroma(8, qP, c, dcC);

	int rMb[8][8];	// Norm: MbWidthC == MbHeightC == 8;
	for (int chroma4x4BlkIdx = 0; chroma4x4BlkIdx < numChroma4x4Blks; chroma4x4BlkIdx++)
	{
		int chromaList[16];
		chromaList[0] = dcC[chroma4x4BlkIdx/2][chroma4x4BlkIdx%2];
		for (int k = 1; k < 16; k++)
		{
			chromaList[k] = ChromaACLevel[chroma4x4BlkIdx][k-1];
		}

		int c2[4][4];
		transformInverseScan(chromaList, c2);

		int r[4][4];
		scaleAndTransform4x4Residual(c2, r, true, QPy, false);

		int x0 = InverseRasterScan(chroma4x4BlkIdx, 4, 4, 8, 0);
		int y0 = InverseRasterScan(chroma4x4BlkIdx, 4, 4, 8, 1);

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				rMb[y0+i][x0+j] = r[i][j];
			}
		}
	}
	
	// Norm: TransformBypassModeFlag == 0 in baseline.

	int u[8][8];	// Norm: MbWidthC == MbHeightC == 8 in baseline.
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			// Norm:
			// u(i,j) = Clip1C(predC[j][i] + rMb[j][i])
			// this is inverted because the first index
			// corresponds to the x coordinate in the
			// standard.
			u[i][j] = Clip1C(predC[i][j] + rMb[i][j]);
		}
	}

	pictureConstructionChroma(u, Cb);
}