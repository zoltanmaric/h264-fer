#include "moestimation.h"
#include "mocomp.h"

#include "headers_and_parameter_sets.h"
#include "ref_frames.h"
#include "mode_pred.h"
#include "h264_math.h"
#include "limits.h"

#define P_Skip_Treshold 0.9

int sadLuma8x8(int predL[16][16], int luma8x8BlkIdx)
{
	int sad = 0;

	int xP = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 0);
	int yP = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 1);

	int x0 = (luma8x8BlkIdx%2) << 3;
	int y0 = (luma8x8BlkIdx&2) << 2;

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			sad += ABS(frame.L[yP+y0+i][xP+x0+j] - predL[x0+i][y0+j]);
		}
	}

	return sad;
}

int ExactPixels(int predL[16][16])
{
	int exactLumaPixels = 0;

	int xP = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 0);
	int yP = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 1);

	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			exactLumaPixels += (frame.L[yP+i][xP+j] != predL[i][j])?0:1;
		}
	}

	return exactLumaPixels;	
}

int sadLuma(int predL[16][16], int partId)
{
	if (mb_type == P_8x8 || mb_type == P_8x8ref0) return sadLuma8x8(predL, partId);
	if (mb_type == P_L0_L0_16x8) return sadLuma8x8(predL, partId*2) + sadLuma8x8(predL, partId*2+1);
	if (mb_type == P_L0_L0_8x16) return sadLuma8x8(predL, partId) + sadLuma8x8(predL, partId+2);
	if (mb_type == P_L0_16x16 || mb_type == P_Skip) return sadLuma8x8(predL, 0) + sadLuma8x8(predL, 1) + sadLuma8x8(predL, 2) + sadLuma8x8(predL, 3);
}

void interEncoding(int predL[16][16], int predCr[8][8], int predCb[8][8]) 
{
	int xP = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 0);
	int yP = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 1);

	int minBlock = INT_MAX;
	int bestMbType = P_Skip, mvdL0[4][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
	ClearMVD();
	mb_type = P_Skip;
	DeriveMVs();
	Decode(predL, predCr, predCb);
	if (ExactPixels(predL) >= (int)(256.0*P_Skip_Treshold))
		return;
	
	for (int curr_mbtype = 0; curr_mbtype < 3; curr_mbtype++)
	{
		mb_type = curr_mbtype;
		ClearMVD();
		int currMin = 0;
		for (int i = 0; i < NumMbPart(mb_type); i++)
		{
			//int currStepSize = 8, currMbSadMin = INT_MAX;
			//int mvdx = 0, mvdy = 0;

			int bx = 0, by = 0, bmin = INT_MAX;
			mvd_l0[i][0][0] = 0;
			mvd_l0[i][0][1] = 0;
			DeriveMVs();
			Decode(predL, predCr, predCb);
			bmin = sadLuma(predL, i);
			for (int tx = -4; tx <= 4; tx+=4)
				for (int ty = -4; ty <= 4; ty+=4)
				{
					mvd_l0[i][0][0] = tx;
					mvd_l0[i][0][1] = ty;
					DeriveMVs();
					Decode(predL, predCr, predCb);
					int trenSad = sadLuma(predL, i);
					if (trenSad < bmin) 
					{
						bmin = trenSad;
						bx = tx; by = ty;
					}
				}
			mvd_l0[i][0][0] = bx;
			mvd_l0[i][0][1] = by;
			for (int tx = 0; tx <= 2; tx+=2)
				for (int ty = 0; ty <= 2; ty+=2)
				{
					mvd_l0[i][0][0] += tx;
					mvd_l0[i][0][1] += ty;
					DeriveMVs();
					Decode(predL, predCr, predCb);
					int trenSad = sadLuma(predL, i);
					if (trenSad < bmin) 
					{
						bmin = trenSad;
						bx += tx; by += ty;
					}
					mvd_l0[i][0][0] -= tx;
					mvd_l0[i][0][1] -= ty;
				}
			currMin += bmin;
			mvd_l0[i][0][0] = bx;
			mvd_l0[i][0][1] = by;

			//while (currStepSize > 0)
			//{
			//	int bx = 0, by = 0, bmin = INT_MAX;
			//	mvd_l0[i][0][0] = mvdx;
			//	mvd_l0[i][0][1] = mvdy;
			//	DeriveMVs();
			//	Decode(predL, predCr, predCb);
			//	bmin = sadLuma(predL, i);
			//	for (int tx = -1; tx < 2; tx++)
			//		for (int ty = -1; ty < 2; ty++)
			//		{
			//			mvd_l0[i][0][0] = mvdx + tx*currStepSize;
			//			mvd_l0[i][0][1] = mvdy + ty*currStepSize;
			//			DeriveMVs();
			//			Decode(predL, predCr, predCb);
			//			int trenSad = sadLuma(predL, i);
			//			if (trenSad < bmin) 
			//			{
			//				bmin = trenSad;
			//				bx = tx; by = ty;
			//			}
			//		}
			//	mvdx = mvdx + bx*currStepSize;
			//	mvdy = mvdy + by*currStepSize;

			//	if (bx == 0 && by == 0) 
			//		currStepSize >>= 1;
			//	currMbSadMin = bmin;
			//}
			//mvd_l0[i][0][0] = mvdx;
			//mvd_l0[i][0][1] = mvdy;
			//currMin += currMbSadMin;
		}
		if (currMin < minBlock)
		{
			minBlock = currMin;
			bestMbType = curr_mbtype;
			for (int i = 0; i < 4; i++)
			{
				mvdL0[i][0] = mvd_l0[i][0][0];
				mvdL0[i][1] = mvd_l0[i][0][1];
			}
		}
	}
	mb_type = bestMbType;
	for (int i = 0; i < 4; i++)
	{
		mvd_l0[i][0][0] = mvdL0[i][0];
		mvd_l0[i][0][1] = mvdL0[i][1];
	}
	DeriveMVs();
	Decode(predL, predCr, predCb);
}