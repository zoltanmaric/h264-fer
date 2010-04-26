#include "moestimation.h"
#include "mocomp.h"

#include "headers_and_parameter_sets.h"
#include "quantizationTransform.h"
#include "ref_frames.h"
#include "mode_pred.h"
#include "h264_math.h"
#include "limits.h"

#define P_Skip_Treshold 0.97

int zigZagIdx[16][2] = {{0, 0}, {1, 0}, {0, 1}, {0, 2},
				  {1, 1}, {2, 0}, {3, 0}, {2, 1},
				  {1, 2}, {0, 3}, {1, 3}, {2, 2},
				  {3, 1}, {3, 2}, {2, 3}, {3, 3}};
int tmpVar[16];

frame_type refFrameInterpolated[16];

void InitializeInterpolatedRefFrame()
{
	for (int i = 0; i < 16; i++)
	{
		refFrameInterpolated[i].Lheight=frame.Lheight;
		refFrameInterpolated[i].Lwidth=frame.Lwidth;
		refFrameInterpolated[i].Cheight=frame.Cheight;
		refFrameInterpolated[i].Cwidth=frame.Cwidth;

		refFrameInterpolated[i].L = new unsigned char*[frame.Lheight];
		for (int it = 0; it < frame.Lheight; it++)
		{
			refFrameInterpolated[i].L[it] = new unsigned char[frame.Lwidth];
		}

		refFrameInterpolated[i].C[0] = new unsigned char*[frame.Cheight];
		refFrameInterpolated[i].C[1] = new unsigned char*[frame.Cheight];

		for (int it = 0; it < frame.Cheight; it++)
		{
			refFrameInterpolated[i].C[0][it] = new unsigned char[frame.Cwidth];
			refFrameInterpolated[i].C[1][it] = new unsigned char[frame.Cwidth];
		}
	}
}

void FillInterpolatedRefFrame()
{
	frame_type * refPic = RefPicList0[*(refIdxL0+CurrMbAddr)].frame;
	int predL[16][16], predCr[8][8], predCb[8][8];
	for (int frac = 0; frac < 16; frac++)
	{
		int mvx = frac&3, mvy = (frac&12)/4;
		for (int tmpMbAddr = 0; tmpMbAddr < shd.PicSizeInMbs; tmpMbAddr++)
		{
			for (int i = 0; i < 4; i++)
				for (int j = 0; j < 4; j++)
				{
					MPI_mvSubL0x_byIdx(tmpMbAddr,i,j) = mvx;
					MPI_mvSubL0y_byIdx(tmpMbAddr,i,j) = mvy;
					MotionCompensateSubMBPart(predL, predCr, predCb, refPic, tmpMbAddr, i, j);
				}
			int x0 = InverseRasterScan(tmpMbAddr, 16, 16, frame.Lwidth, 0);
			int y0 = InverseRasterScan(tmpMbAddr, 16, 16, frame.Lwidth, 1);
			for (int i = 0; i < 16; i++)
				for (int j = 0; j < 16; j++)
					refFrameInterpolated[frac].L[y0+i][x0+j] = predL[i][j];
			x0 /= 2; y0 /= 2;
			for (int i = 0; i < 8; i++)
				for (int j = 0; j < 8; j++)
				{
					refFrameInterpolated[frac].C[0][y0+i][x0+j] = predCb[i][j];
					refFrameInterpolated[frac].C[1][y0+i][x0+j] = predCr[i][j];
				}
		}
	}
}

int satdLuma8x8MVs(int mvx, int mvy, int luma8x8BlkIdx)
{
	int satd = 0;

	int xP = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 0);
	int yP = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 1);
	int xPi = xP + mvx/4; if (xPi < 0) xPi = 0; if (xPi >= frame.Lwidth) xPi = frame.Lwidth-1;
	int yPi = yP + mvy/4; if (yPi < 0) yPi = 0; if (yPi >= frame.Lheight) yPi = frame.Lheight-1;
	int frac = (mvx&3) + (mvy&3)*4;
	int diffL4x4[4][4], rLuma[4][4], x0, y0, px, py;
	
	for (int part4x4idx = 0; part4x4idx < 4; part4x4idx++)
	{
		x0 = ((luma8x8BlkIdx%2) << 3) + ((part4x4idx%2) << 2);
		y0 = ((luma8x8BlkIdx&2) << 2) + ((part4x4idx&2) << 1);
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
			{
				px = xPi+x0+i; if (px >= frame.Lwidth) px = frame.Lwidth-1;
				py = yPi+y0+i; if (py >= frame.Lheight) py = frame.Lheight-1;
				diffL4x4[i][j] = frame.L[yP+y0+i][xP+x0+j] - refFrameInterpolated[frac].L[py][px];
			}
		forwardResidual(QPy, diffL4x4, rLuma, true, false);
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				satd += ABS(rLuma[i][j]);
	}

	return satd;
}

int satdLuma8x8(int predL[16][16], int luma8x8BlkIdx)
{
	int satd = 0;

	int xP = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 0);
	int yP = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 1);
	int diffL4x4[4][4], rLuma[4][4], x0, y0;
	
	for (int part4x4idx = 0; part4x4idx < 4; part4x4idx++)
	{
		x0 = ((luma8x8BlkIdx%2) << 3) + ((part4x4idx%2) << 2);
		y0 = ((luma8x8BlkIdx&2) << 2) + ((part4x4idx&2) << 1);
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				diffL4x4[i][j] = frame.L[yP+y0+i][xP+x0+j] - predL[y0+i][x0+j];
		forwardResidual(QPy, diffL4x4, rLuma, true, false);
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				satd += ABS(rLuma[i][j]);
	}

	return satd;
}

int zigZagSADLuma8x8(int predL[16][16], int luma8x8BlkIdx)
{
	int sad = 0;

	int xP = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 0);
	int yP = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 1), x0, y0;

	for (int i = 0; i < 4; i++)
	{
		x0 = ((luma8x8BlkIdx%2) << 3) + ((i%2) << 2);
		y0 = ((luma8x8BlkIdx&2) << 2) + ((i&2) << 1);
		for (int j = 0; j < 16; j++)
		{
			tmpVar[j] = frame.L[yP+y0+zigZagIdx[j][1]][xP+x0+zigZagIdx[j][0]] - predL[y0+zigZagIdx[j][1]][x0+zigZagIdx[j][0]];
			if (j)
				sad += ABS(tmpVar[j]-tmpVar[j-1]);
			else 
				sad += ABS(tmpVar[j]);
		}
	}

	return sad;

}

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
			exactLumaPixels += (ABS(frame.L[yP+i][xP+j]-predL[i][j])<2)?1:0;
		}
	}

	return exactLumaPixels;	
}

int sadLuma(int predL[16][16], int partId)
{
	if (mb_type == P_8x8 || mb_type == P_8x8ref0) return satdLuma8x8(predL, partId);
	if (mb_type == P_L0_L0_16x8) return satdLuma8x8(predL, partId*2) + satdLuma8x8(predL, partId*2+1);
	if (mb_type == P_L0_L0_8x16) return satdLuma8x8(predL, partId) + satdLuma8x8(predL, partId+2);
	if (mb_type == P_L0_16x16 || mb_type == P_Skip) return satdLuma8x8(predL, 0) + satdLuma8x8(predL, 1) + satdLuma8x8(predL, 2) + satdLuma8x8(predL, 3);
}

int sadLumaMVs(int mvx, int mvy, int partId)
{
	if (mb_type == P_8x8 || mb_type == P_8x8ref0) return satdLuma8x8MVs(mvx, mvy, partId);
	if (mb_type == P_L0_L0_16x8) return satdLuma8x8MVs(mvx, mvy, partId*2) + satdLuma8x8MVs(mvx, mvy, partId*2+1);
	if (mb_type == P_L0_L0_8x16) return satdLuma8x8MVs(mvx, mvy, partId) + satdLuma8x8MVs(mvx, mvy, partId+2);
	if (mb_type == P_L0_16x16 || mb_type == P_Skip) return satdLuma8x8MVs(mvx, mvy, 0) + satdLuma8x8MVs(mvx, mvy, 1) + satdLuma8x8MVs(mvx, mvy, 2) + satdLuma8x8MVs(mvx, mvy, 3);
}

void interEncoding(int predL[16][16], int predCr[8][8], int predCb[8][8]) 
{
	int xP = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 0);
	int yP = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 1);

	int minBlock = INT_MAX;
	int bestMbType = P_Skip, mvdL0[4][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
	ClearMVD();
	mb_type = P_Skip;
	mb_type_array[CurrMbAddr] = P_Skip;
	DeriveMVs();
	Decode(predL, predCr, predCb);
	if (ExactPixels(predL) >= (int)(256.0*P_Skip_Treshold))
		return;
	//printf("%d\n", CurrMbAddr);
	for (int curr_mbtype = 0; curr_mbtype < 3; curr_mbtype++)
	{
		mb_type = curr_mbtype;
		mb_type_array[CurrMbAddr] = curr_mbtype;
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
			int genx = MPI_mvL0x(CurrMbAddr, i);
			int geny = MPI_mvL0y(CurrMbAddr, i);
			//Decode(predL, predCr, predCb);
			bmin = sadLumaMVs(0, 0, i);
			for (int tmpx = -128; tmpx <= 128; tmpx+=32)
				for (int tmpy = -128; tmpy <= 128; tmpy+=32)
				{
					int tmpdif = sadLumaMVs(tmpx, tmpy, i);
					if (tmpdif < bmin)
					{
						bmin = tmpdif;
						bx = tmpx; by = tmpy;
					}
				}
			for (int tmpx = -3; tmpx <= 3; tmpx++)
				for (int tmpy = -3; tmpy <= 3; tmpy++)
				{
					int tmpdif = sadLumaMVs(tmpx+bx, tmpy+by, i);
					if (tmpdif < bmin)
					{
						bmin = tmpdif;
						bx += tmpx; by += tmpy;
					}
				}
			bx -= genx;
			by -= geny;\
			Decode(predL, predCr, predCb);
			bmin = sadLuma(predL, i);
			for (int tx = -16; tx <= 16; tx+=4)
				for (int ty = -16; ty <= 16; ty+=4)
				if (tx+ty){
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
			for (int tx = 0; tx <= 1; tx+=1)
				for (int ty = 0; ty <= 1; ty+=1)
				if (tx+ty) {
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
	mb_type_array[CurrMbAddr] = bestMbType;
	for (int i = 0; i < 4; i++)
	{
		mvd_l0[i][0][0] = mvdL0[i][0];
		mvd_l0[i][0][1] = mvdL0[i][1];
	}
	DeriveMVs();
	//printf("Trenutni MB = %d> ", CurrMbAddr);
	for (int i = 0; i < 4; i++)
	{
		//printf("(%d:%d, %d:%d), ", MPI_mvL0x(CurrMbAddr, i), mvd_l0[i][0][0], MPI_mvL0y(CurrMbAddr, i), mvd_l0[i][0][1]);
	}
	//printf("\n");
	Decode(predL, predCr, predCb);
}