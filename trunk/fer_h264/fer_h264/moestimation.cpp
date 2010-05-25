#include "moestimation.h"
#include "mocomp.h"

#include "headers_and_parameter_sets.h"
#include "quantizationTransform.h"
#include "ref_frames.h"
#include "mode_pred.h"
#include "h264_math.h"
#include "limits.h"
#include "openCL_functions.h"

#define P_Skip_Treshold 0.98

int zigZagIdx[16][2] = {{0, 0}, {1, 0}, {0, 1}, {0, 2},
				  {1, 1}, {2, 0}, {3, 0}, {2, 1},
				  {1, 2}, {0, 3}, {1, 3}, {2, 2},
				  {3, 1}, {3, 2}, {2, 3}, {3, 3}};
int tmpVar[16];
bool bio[128][128];
frame_type refFrameInterpolated[16];
//frame_type refFrameKar[16];

int MAXDIFF = 2;
int **refFrameKar[6][16];
int bxs[85], bys[85], bmins[85], bsuma[85], suma[5];

void InitializeInterpolatedRefFrame()
{
	for (int i = 0; i < 16; i++)
	{
		refFrameInterpolated[i].Lheight = frame.Lheight;
		refFrameInterpolated[i].Lwidth = frame.Lwidth;
		refFrameInterpolated[i].Cheight = frame.Cheight;
		refFrameInterpolated[i].Cwidth = frame.Cwidth;

		refFrameInterpolated[i].L = new unsigned char[frame.Lheight*frame.Lwidth];
		for (int kar = 0; kar < 5; kar++) refFrameKar[kar][i] = new int*[frame.Lheight+8];
		for (int it = 0; it < frame.Lheight+8; it++)
		{
			for (int kar = 0; kar < 5; kar++) refFrameKar[kar][i][it] = new int[frame.Lwidth+8];
		}

		refFrameInterpolated[i].C[0] = new unsigned char[frame.Cheight*frame.Cwidth];
		refFrameInterpolated[i].C[1] = new unsigned char[frame.Cheight*frame.Cwidth];
	}
}

int transformedLuma(int a)
{
	if (a < 50) return 1;
	if (a < 110) return 64;
	if (a < 130) return 4096;
	if (a < 160) return 262144;
	return 16777216;
}

int diffTransformedLuma(int a, int b)
{
	return  ABS((a&(0x3f)) - (b&(0x3f)))*90
		  + (ABS((a&(0xfc0)) - (b&(0xfc0)))>>6)*120
		  + (ABS((a&(0x3f000)) - (b&(0x3f000)))>>12)*140
		  + (ABS((a&(0xfc0000)) - (b&(0xfc0000)))>>18)*160
		  + (ABS((a&(0x3f000000)) - (b&(0x3f000000)))>>24)*200;
}

void FillInterpolatedRefFrame()
{	
	frame_type * refPic = RefPicList0[0].frame;
	int predL[16][16], predCr[8][8], predCb[8][8];
	for (int frac = 0; frac < 16; frac++)
	{
		int mvx = frac&3, mvy = (frac&12)/4;
		for (int tmpMbAddr = 0; tmpMbAddr < shd.PicSizeInMbs; tmpMbAddr++)
		{
			for (int i = 0; i < 4; i++)
				for (int j = 0; j < 4; j++)
				{
					mvL0x[tmpMbAddr][i][j] = mvx;
					mvL0y[tmpMbAddr][i][j] = mvy;
					MotionCompensateSubMBPart(predL, predCr, predCb, refPic, tmpMbAddr, i, j);
				}
			int x0 = ((tmpMbAddr%PicWidthInMbs)<<4);
			int y0 = ((tmpMbAddr/PicWidthInMbs)<<4);
			for (int i = 0; i < 16; i++)
				for (int j = 0; j < 16; j++)
					refFrameInterpolated[frac].L[(y0+i)*frame.Lwidth+x0+j] = predL[i][j];
			x0 /= 2; y0 /= 2;
			for (int i = 0; i < 8; i++)
				for (int j = 0; j < 8; j++)
				{
					refFrameInterpolated[frac].C[0][(y0+i)*frame.Cwidth+x0+j] = predCb[i][j];
					refFrameInterpolated[frac].C[1][(y0+i)*frame.Cwidth+x0+j] = predCr[i][j];
				}
		}
	}
	TestKar(refFrameKar, refFrameInterpolated);
	//for (int i = 0; i < 16; i++)
	//{
	//	for (int tx = frame.Lwidth; tx < frame.Lwidth+8; tx++)
	//		for (int ty = frame.Lheight-1; ty >= 0; ty--)
	//			refFrameKar[0][i][ty][tx] = (int)refFrameInterpolated[i].L[ty*frame.Lwidth+frame.Lwidth-1];
	//	for (int tx = 0; tx < frame.Lwidth; tx++)
	//		for (int ty = frame.Lheight; ty < frame.Lheight+8; ty++)
	//			refFrameKar[0][i][ty][tx] = (int)refFrameInterpolated[i].L[(frame.Lheight-1)*frame.Lwidth+tx];
	//	for (int tx = frame.Lwidth; tx < frame.Lwidth+8; tx++)
	//		for (int ty = frame.Lheight; ty < frame.Lheight+8; ty++)
	//			refFrameKar[0][i][ty][tx] = (int)refFrameInterpolated[i].L[(frame.Lheight-1)*frame.Lwidth+frame.Lwidth-1];
	//	for (int tx = frame.Lwidth+7; tx >= 0; tx--)
	//		for (int ty = frame.Lheight+7; ty >= 0; ty--)
	//		{
	//			if (ty < frame.Lheight && tx < frame.Lwidth)
	//				refFrameKar[0][i][ty][tx] = (int)refFrameInterpolated[i].L[ty*frame.Lwidth+tx];
	//			if (ty < frame.Lheight+7)
	//				refFrameKar[0][i][ty][tx] += refFrameKar[0][i][ty+1][tx];
	//			if (tx < frame.Lwidth+7)
	//				refFrameKar[0][i][ty][tx] += refFrameKar[0][i][ty][tx+1];
	//			if (ty < frame.Lheight+7 && tx < frame.Lwidth+7)
	//				refFrameKar[0][i][ty][tx] -= refFrameKar[0][i][ty+1][tx+1];
	//		}
	//	for (int tx = 0; tx < frame.Lwidth; tx++)
	//		for (int ty = 0; ty < frame.Lheight; ty++)
	//		{
	//			refFrameKar[4][i][ty][tx] = refFrameKar[0][i][ty][tx] - refFrameKar[0][i][ty][tx+2] - refFrameKar[0][i][ty+8][tx] + refFrameKar[0][i][ty+8][tx+2]
	//									  + refFrameKar[0][i][ty][tx+4] - refFrameKar[0][i][ty][tx+6] - refFrameKar[0][i][ty+8][tx+4] + refFrameKar[0][i][ty+8][tx+6];
	//			refFrameKar[3][i][ty][tx] = refFrameKar[0][i][ty][tx] - refFrameKar[0][i][ty+2][tx] - refFrameKar[0][i][ty][tx+8] + refFrameKar[0][i][ty+2][tx+8]
	//									  + refFrameKar[0][i][ty+4][tx] - refFrameKar[0][i][ty+6][tx] - refFrameKar[0][i][ty+4][tx+8] + refFrameKar[0][i][ty+6][tx+8];
	//			refFrameKar[2][i][ty][tx] = refFrameKar[0][i][ty][tx] - refFrameKar[0][i][ty+8][tx] - refFrameKar[0][i][ty][tx+4] + refFrameKar[0][i][ty+8][tx+4];
	//			refFrameKar[1][i][ty][tx] = refFrameKar[0][i][ty][tx] - refFrameKar[0][i][ty+4][tx] - refFrameKar[0][i][ty][tx+8] + refFrameKar[0][i][ty+4][tx+8];
	//			refFrameKar[0][i][ty][tx] -= refFrameKar[0][i][ty+8][tx] + refFrameKar[0][i][ty][tx+8] - refFrameKar[0][i][ty+8][tx+8];
	//		}
	//}
}

int satdLuma8x8MVs(int mvx, int mvy, int luma8x8BlkIdx)
{
	int satd = 0, pretvori = 1;

	int xP = ((CurrMbAddr%PicWidthInMbs)<<4) + (luma8x8BlkIdx%2) * 8;
	int yP = ((CurrMbAddr/PicWidthInMbs)<<4) + (luma8x8BlkIdx/2) * 8;
	int xPi = xP + (mvx>>2); if (xPi < 0) xPi = 0; if (xPi >= frame.Lwidth) xPi = frame.Lwidth-1;
	int yPi = yP + (mvy>>2); if (yPi < 0) yPi = 0; if (yPi >= frame.Lheight) yPi = frame.Lheight-1;
	int frac = (mvx&3) + (mvy&3)*4;

	int x0, y0, px, py;
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
		{
			px = xPi+j; if (px >= frame.Lwidth) px = frame.Lwidth-1;
			py = yPi+i; if (py >= frame.Lheight) py = frame.Lheight-1;
			satd += ABS(frame.L[(yP+i)*frame.Lwidth+xP+j] - (int)(refFrameInterpolated[frac].L[py*frame.Lwidth+px]));
		}
	int fracC = (mvx&2)/2 +((xPi+x0)&1)*2 + (mvy&2)*2 + ((yPi+y0)&1)*8;
	for (int boja = 0; boja < 2; boja++)
		for (int i = 0; i < 2; i++)
			for (int j = 0; j < 2; j++)
			{
				px = xPi/2 + j; if (px >= frame.Cwidth) px = frame.Cwidth-1;
				py = yPi/2 + i; if (py >= frame.Cheight) py = frame.Cheight-1;
				satd += ABS(frame.C[boja][(yP/2+i)*frame.Cwidth+xP/2+j] - refFrameInterpolated[fracC].C[boja][py*frame.Cwidth+px]);
			}

	return satd + ABS(mvx) + ABS(mvy);
}

void PopraviPSkip(int predL[16][16])
{
	int xP = ((CurrMbAddr%PicWidthInMbs)<<4);
	int yP = ((CurrMbAddr/PicWidthInMbs)<<4);

	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			frame.L[(yP+i)*frame.Lwidth+xP+j] = predL[i][j];
		}
	}
}

int ExactPixels(int predL[16][16])
{
	int exactLumaPixels = 0;

	int xP = ((CurrMbAddr%PicWidthInMbs)<<4);
	int yP = ((CurrMbAddr/PicWidthInMbs)<<4);

	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			exactLumaPixels += (ABS(frame.L[(yP+i)*frame.Lwidth+xP+j]-predL[i][j])<=MAXDIFF)?1:0;
		}
	}

	return exactLumaPixels;	
}

int sadLumaMVs(int mvx, int mvy, int partId)
{
	if (mb_type == P_8x8 || mb_type == P_8x8ref0) return satdLuma8x8MVs(mvx, mvy, partId);
	if (mb_type == P_L0_L0_16x8) return satdLuma8x8MVs(mvx, mvy, partId*2) + satdLuma8x8MVs(mvx, mvy, partId*2+1);
	if (mb_type == P_L0_L0_8x16) return satdLuma8x8MVs(mvx, mvy, partId) + satdLuma8x8MVs(mvx, mvy, partId+2);
	return satdLuma8x8MVs(mvx, mvy, 0) + satdLuma8x8MVs(mvx, mvy, 1) + satdLuma8x8MVs(mvx, mvy, 2) + satdLuma8x8MVs(mvx, mvy, 3);
}

void MEstimation(int sx, int sy, int granica, int stepMV, int stepFrac, int genx, int geny, int dokle)
{
	int refx, refy, trenRazlika;
	for (int tmpx = genx-granica; tmpx <= genx+granica; tmpx+=stepMV)
	{
		for (int tmpy = geny-granica; tmpy <= geny+granica; tmpy+=stepMV)
		{
			for (int frac = 0; frac < 16; frac+=stepFrac)
			{
				refx = sx+tmpx;
				refy = sy+tmpy;
				if (refy >= 0 && refy < frame.Lheight && refx>=0 && refx<frame.Lwidth) 
				{
					trenRazlika =   (ABS(tmpx-genx)+ABS(tmpy-geny)+4) * ( ABS(suma[0]-refFrameKar[0][frac][refy][refx])
									+ ABS(suma[1]-refFrameKar[1][frac][refy][refx])
									+ ABS(suma[0]-suma[1]-refFrameKar[0][frac][refy][refx]+refFrameKar[1][frac][refy][refx])
									+ ABS(suma[2]-refFrameKar[2][frac][refy][refx])
									+ ABS(suma[0]-suma[2]-refFrameKar[0][frac][refy][refx]+refFrameKar[2][frac][refy][refx])
									+ ABS(suma[3]-refFrameKar[3][frac][refy][refx])
									+ ABS(suma[0]-suma[3]-refFrameKar[0][frac][refy][refx]+refFrameKar[3][frac][refy][refx])
									+ ABS(suma[4]-refFrameKar[4][frac][refy][refx])
									+ ABS(suma[0]-suma[4]-refFrameKar[0][frac][refy][refx]+refFrameKar[4][frac][refy][refx])
									);
					if (bmins[64] > trenRazlika)
					bmins[64] = trenRazlika;
					bsuma[64] = refFrameKar[0][frac][refy][refx];
					bxs[64] = (tmpx<<2) | (frac&3);
					bys[64] = (tmpy<<2) | ((frac>>2)&3);
					for (int j = 64; j > dokle; j--)
					{
						if (bmins[j] < bmins[j-1]) 
						{
							int t1 = bmins[j]; bmins[j] = bmins[j-1]; bmins[j-1] = t1;
							t1 = bsuma[j]; bsuma[j] = bsuma[j-1]; bsuma[j-1] = t1;
							t1 = bxs[j]; bxs[j] = bxs[j-1]; bxs[j-1] = t1;
							t1 = bys[j]; bys[j] = bys[j-1]; bys[j-1] = t1;
						} else break;
					}
				}
			}
		}
	}
}

void interEncoding(int predL[16][16], int predCr[8][8], int predCb[8][8]) 
{
	int xp = ((CurrMbAddr%PicWidthInMbs)<<4);
	int yp = ((CurrMbAddr/PicWidthInMbs)<<4);
	MAXDIFF = 2;
	int minBlock = INT_MAX;
	int bestMbType = P_Skip, mvdL0[4][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, mvx[4], mvy[4];
	ClearMVD();
	mb_type = P_Skip;
	mb_type_array[CurrMbAddr] = P_Skip;
	DeriveMVs();
	Decode(predL, predCr, predCb);
	int bla = 0;
	for (int tx = 0; tx < 16; tx++)
		for (int ty = 0; ty < 16; ty++)
			bla += (int)(frame.L[(ty+yp)*frame.Lwidth+tx+xp]);
	MAXDIFF = bla/256;  bla = 0;
	for (int tx = 0; tx < 16; tx++)
		for (int ty = 0; ty < 16; ty++)
			bla += ABS((int)(frame.L[(ty+yp)*frame.Lwidth+tx+xp]) - MAXDIFF);
	MAXDIFF = bla/256;
	if (MAXDIFF < 3) MAXDIFF = 3;
	if (ExactPixels(predL) == 256) 
	{
		PopraviPSkip(predL);
		return;
	}
		mb_type = P_8x8ref0;
		mb_type_array[CurrMbAddr] = P_8x8ref0;
		ClearMVD();
		int prazanResidual[4] = {0, 0, 0, 0};
		for (int i = 0; i < NumMbPart(mb_type); i++)
		{
			mvd_l0[i][0][0] = mvd_l0[i][0][1] = 0;
			DeriveMVs();
			int genx = mvL0x[CurrMbAddr][i][0] >> 2;
			int geny = mvL0y[CurrMbAddr][i][0] >> 2;
			int relx = (i%2) * 8, rely = (i/2) * 8;
			int u = 1;
			for (int te = 0; te < 5; te++) suma[te] = 0;
			for (int tx = 0; tx < 8; tx++)
			{
				for (int ty = 0; ty < 8; ty++)
				{
					suma[0] += frame.L[(ty+rely+yp)*frame.Lwidth+tx+relx+xp];
					suma[1] += (ty>3)?0:frame.L[(ty+rely+yp)*frame.Lwidth+tx+relx+xp];
					suma[2] += (tx>3)?0:frame.L[(ty+rely+yp)*frame.Lwidth+tx+relx+xp];
					suma[3] += ((ty%4)>1)?0:frame.L[(ty+rely+yp)*frame.Lwidth+tx+relx+xp];
					suma[4] += ((tx%4)>1)?0:frame.L[(ty+rely+yp)*frame.Lwidth+tx+relx+xp];
				}
			}
			int sumaAbsRazlika = 0, srednja = (suma[0]>>6);
			for (int tx = 0; tx < 8; tx++)
			{
				for (int ty = 0; ty < 8; ty++)
				{
					sumaAbsRazlika += ABS(frame.L[(ty+rely+yp)*frame.Lwidth+tx+relx+xp] - srednja);
				}
			}
			MAXDIFF = 2;
			int bx = 0, by = 0, bmin = 1000000000, refx, refy, trenRazlika;
			for (int j = 0; j < 85; j++)
			{
				bmins[j] = 1000000000;
				bxs[j] = bys[j] = 100000000;
			}
			int tux[32], tuy[32];
			MEstimation(relx+xp, rely+yp, 1, 1, 1, genx, geny, 0);
			bmin = 2000000000;
			for (int j = 0; j <= 64; j++)
			if (bxs[j] < 100000000 && bys[j] < 100000000) {
				bmins[j] = sadLumaMVs(bxs[j], bys[j], i);
				if (bmins[j] < bmin)
				{
					bmin = bmins[j];
					bx = bxs[j];
					by = bys[j];
				}
			}
			for (int j = 0; j < 85; j++) bmins[j] = 1000000000;
			MEstimation(relx+xp, rely+yp, 4, 1, 1, 0, 0, 0);
			MEstimation(relx+xp, rely+yp, 16, 2, 2, 0, 0, 32);
			MEstimation(relx+xp, rely+yp, 256, 16, 16, 0, 0, 32);
			for (int j = 0; j <= 64; j++)
			if (bxs[j] < 100000000 && bys[j] < 100000000) {
				bmins[j] = sadLumaMVs(bxs[j], bys[j], i);
				if (bmins[j] < bmin)
				{
					bmin = bmins[j];
					bx = bxs[j];
					by = bys[j];
				}
			}
			if (bmin == 0) prazanResidual[i] = 1;
			mvx[i] = bx; mvy[i] = by;
			bx -= mvL0x[CurrMbAddr][i][0];
			by -= mvL0y[CurrMbAddr][i][0];
			mvd_l0[i][0][0] = mvdL0[i][0] = bx;
			mvd_l0[i][0][1] = mvdL0[i][1] = by;
		}
		if (mvx[0] == mvx[1] && mvx[0] == mvx[2] && mvx[0] == mvx[3] &&
			mvy[0] == mvy[1] && mvy[0] == mvy[2] && mvy[0] == mvy[3] )
		{
			mb_type = mb_type_array[CurrMbAddr] = P_L0_16x16;
		} else {
			if (mvx[0] == mvx[1] && mvx[2] == mvx[3] &&
				mvy[0] == mvy[1] && mvy[2] == mvy[3] )
			{
				mb_type = mb_type_array[CurrMbAddr] = P_L0_L0_16x8;
				mvx[1] = mvx[2];
				mvy[1] = mvy[2];
			} else {
				if (mvx[0] == mvx[2] && mvx[1] == mvx[3] &&
					mvy[0] == mvy[2] && mvy[1] == mvy[3] )
				{
					mb_type = mb_type_array[CurrMbAddr] = P_L0_L0_8x16;
				}
			}
		}
		for (int i = 0; i < NumMbPart(mb_type); i++)
		{
			mvd_l0[i][0][0] = mvd_l0[i][0][1] = 0;
			DeriveMVs();
			if (i == 1 && mb_type == P_L0_L0_16x8)
			{
				mvd_l0[i][0][0] = mvx[i] - mvL0x[CurrMbAddr][2][0];
				mvd_l0[i][0][1] = mvy[i] - mvL0y[CurrMbAddr][2][0];
			} else {
				mvd_l0[i][0][0] = mvx[i] - mvL0x[CurrMbAddr][i][0];
				mvd_l0[i][0][1] = mvy[i] - mvL0y[CurrMbAddr][i][0];
			}
		}
		DeriveMVs();
		for (int i = 0; i < 4; i++){
			mvx[i] = mvL0x[CurrMbAddr][i][0];
			mvy[i] = mvL0y[CurrMbAddr][i][0];
		}
		Decode(predL, predCr, predCb);
		for (int i = 0; i < 4; i++){
				for (int tx = (i&1)*8; tx < 8+(i&1)*8; tx++)
					for (int ty = (i&2)*4; ty < 8+(i&2)*4; ty++)
						if (ABS(frame.L[(yp+ty)*frame.Lwidth+xp+tx] - predL[ty][tx]) < 3)
							frame.L[(yp+ty)*frame.Lwidth+xp+tx] = predL[ty][tx];
				for (int tx = (i&1)*4; tx < 8-(i&1)*4; tx++)
					for (int ty = (i&2)*2; ty < 8-(i&2)*2; ty++)
					{
						if (ABS(frame.C[0][(yp/2+ty)*frame.Cwidth+xp/2+tx]-predCb[ty][tx]) <= 3) 
							frame.C[0][(yp/2+ty)*frame.Cwidth+xp/2+tx] = predCb[ty][tx];
						if (ABS(frame.C[1][(yp/2+ty)*frame.Cwidth+xp/2+tx]-predCr[ty][tx]) <= 3) 
							frame.C[1][(yp/2+ty)*frame.Cwidth+xp/2+tx] = predCr[ty][tx];
					}
		}
}