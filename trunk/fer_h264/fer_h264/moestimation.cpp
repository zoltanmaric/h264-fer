#include "moestimation.h"
#include "mocomp.h"

#include "headers_and_parameter_sets.h"
#include "quantizationTransform.h"
#include "ref_frames.h"
#include "mode_pred.h"
#include "h264_math.h"
#include "limits.h"

#define P_Skip_Treshold 0.98

int zigZagIdx[16][2] = {{0, 0}, {1, 0}, {0, 1}, {0, 2},
				  {1, 1}, {2, 0}, {3, 0}, {2, 1},
				  {1, 2}, {0, 3}, {1, 3}, {2, 2},
				  {3, 1}, {3, 2}, {2, 3}, {3, 3}};
int tmpVar[16];

frame_type refFrameInterpolated[16];
//frame_type refFrameKar[16];

int **refFrameKar[6][16];
unsigned char * PostLuma[9];

void InitializeInterpolatedRefFrame()
{
	for (int i = 0; i < 9; i++)
		PostLuma[i] = new unsigned char[frame.Lwidth + 5];
	for (int i = 0; i < 16; i++)
	{
		refFrameInterpolated[i].Lheight = frame.Lheight;
		refFrameInterpolated[i].Lwidth = frame.Lwidth;
		refFrameInterpolated[i].Cheight = frame.Cheight;
		refFrameInterpolated[i].Cwidth = frame.Cwidth;

		refFrameInterpolated[i].L = new unsigned char[frame.Lheight*frame.Lwidth];
		for (int kar = 0; kar < 6; kar++) refFrameKar[kar][i] = new int*[frame.Lheight];
		for (int it = 0; it < frame.Lheight; it++)
		{
			//refFrameInterpolated[i].L[it] = new unsigned char[frame.Lwidth];
			for (int kar = 0; kar < 6; kar++) refFrameKar[kar][i][it] = new int[frame.Lwidth];
		}

		refFrameInterpolated[i].C[0] = new unsigned char[frame.Cheight*frame.Cwidth];
		refFrameInterpolated[i].C[1] = new unsigned char[frame.Cheight*frame.Cwidth];

		//for (int it = 0; it < frame.Cheight; it++)
		//{
		//	refFrameInterpolated[i].C[0][it] = new unsigned char[frame.Cwidth];
		//	refFrameInterpolated[i].C[1][it] = new unsigned char[frame.Cwidth];
		//}
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
					refFrameInterpolated[frac].L[(y0+i)*frame.Lwidth+(x0+j)] = predL[i][j];
			x0 /= 2; y0 /= 2;
			for (int i = 0; i < 8; i++)
				for (int j = 0; j < 8; j++)
				{
					refFrameInterpolated[frac].C[0][(y0+i)*frame.Cwidth+(x0+j)] = predCb[i][j];
					refFrameInterpolated[frac].C[1][(y0+i)*frame.Cwidth+(x0+j)] = predCr[i][j];
				}
		}
	}
	for (int i = 0; i < 16; i++)
	{
		for (int tx = frame.Lwidth-1; tx >= 0; tx--)
		{
			for (int ty = frame.Lheight-1; ty >= 0; ty--)
			{
				for (int kar = 0; kar < 4; kar++) refFrameKar[kar][i][ty][tx] = refFrameInterpolated[i].L[ty*frame.Lwidth+tx];
				refFrameKar[1][i][ty][tx] = ABS((int)refFrameKar[1][i][ty][tx]-128);
				refFrameKar[2][i][ty][tx] = transformedLuma(refFrameKar[2][i][ty][tx]);
				if (ty < frame.Lheight-1)
					for (int kar = 0; kar < 4; kar++)
						refFrameKar[kar][i][ty][tx] += refFrameKar[kar][i][ty+1][tx];
			}
			for (int ty = 0; ty < frame.Lheight; ty++)
			{
				refFrameKar[4][i][ty][tx] = refFrameKar[0][i][ty][tx] + (4-frame.Lheight+ty) * refFrameKar[0][i][frame.Lheight-1][tx];
				if (ty < frame.Lheight-4)
					refFrameKar[4][i][ty][tx] = refFrameKar[0][i][ty][tx] - refFrameKar[0][i][ty+4][tx];
				if (ty < frame.Lheight-8)
				{
					refFrameKar[0][i][ty][tx] -= refFrameKar[0][i][ty+8][tx];
					refFrameKar[1][i][ty][tx] -= refFrameKar[1][i][ty+8][tx];
					refFrameKar[2][i][ty][tx] -= refFrameKar[2][i][ty+8][tx];
				}
				else
				{
					refFrameKar[0][i][ty][tx] += (8-frame.Lheight+ty) * refFrameKar[0][i][frame.Lheight-1][tx];
					refFrameKar[1][i][ty][tx] += (8-frame.Lheight+ty) * refFrameKar[1][i][frame.Lheight-1][tx];
					refFrameKar[2][i][ty][tx] += (8-frame.Lheight+ty) * refFrameKar[2][i][frame.Lheight-1][tx];
				}
				if (tx < frame.Lwidth-1)
				{
					refFrameKar[0][i][ty][tx] += refFrameKar[0][i][ty][tx+1];
					refFrameKar[1][i][ty][tx] += refFrameKar[1][i][ty][tx+1];
					refFrameKar[2][i][ty][tx] += refFrameKar[2][i][ty][tx+1];
				}
			}
		}
		for (int tx = 0; tx < frame.Lwidth; tx++)
		{
			for (int ty = 0; ty < frame.Lheight; ty++)
			{
				refFrameKar[3][i][ty][tx] = refFrameKar[0][i][ty][tx] + (4-frame.Lwidth+tx) * refFrameKar[0][i][ty][frame.Lwidth-1];
				if (tx < frame.Lwidth-4)
					refFrameKar[3][i][ty][tx] = refFrameKar[0][i][ty][tx] - refFrameKar[0][i][ty][tx+4];
				if (tx < frame.Lwidth-8)
				{
					refFrameKar[0][i][ty][tx] -= refFrameKar[0][i][ty][tx+8];
					refFrameKar[1][i][ty][tx] -= refFrameKar[1][i][ty][tx+8];
					refFrameKar[2][i][ty][tx] -= refFrameKar[2][i][ty][tx+8];
					refFrameKar[4][i][ty][tx] -= refFrameKar[4][i][ty][tx+8];
				}
				else
				{
					refFrameKar[0][i][ty][tx] += (8-frame.Lwidth+tx) * refFrameKar[0][i][ty][frame.Lwidth-1];
					refFrameKar[1][i][ty][tx] += (8-frame.Lwidth+tx) * refFrameKar[1][i][ty][frame.Lwidth-1];
					refFrameKar[2][i][ty][tx] += (8-frame.Lwidth+tx) * refFrameKar[2][i][ty][frame.Lwidth-1];
					refFrameKar[4][i][ty][tx] += (8-frame.Lwidth+tx) * refFrameKar[4][i][ty][frame.Lwidth-1];
				}
			}
		}
	}
}

void Pogorsaj(int mvx,  int mvy, int luma8x8BlkIdx)
{
	int xP = ((CurrMbAddr%PicWidthInMbs)<<4);
	int yP = ((CurrMbAddr/PicWidthInMbs)<<4);
	int xPi = xP + (mvx>>2); if (xPi < 0) xPi = 0; if (xPi >= frame.Lwidth) xPi = frame.Lwidth-1;
	int yPi = yP + (mvy>>2); if (yPi < 0) yPi = 0; if (yPi >= frame.Lheight) yPi = frame.Lheight-1;
	int frac = (mvx&3) + (mvy&3)*4;
	int x0, y0, px, py;
	
	for (int part4x4idx = 0; part4x4idx < 4; part4x4idx++)
	{
		x0 = ((luma8x8BlkIdx%2) << 3) + ((part4x4idx%2) << 2);
		y0 = ((luma8x8BlkIdx&2) << 2) + ((part4x4idx&2) << 1);
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
			{
				px = xPi+x0+j; if (px >= frame.Lwidth) px = frame.Lwidth-1;
				py = yPi+y0+i; if (py >= frame.Lheight) py = frame.Lheight-1;
				frame.L[(yP+y0+i)*frame.Lwidth+(xP+x0+j)] = refFrameInterpolated[frac].L[py*frame.Lwidth+px];
			}
		for (int boja = 0; boja < 2; boja++)
			for (int i = 0; i < 2; i++)
				for (int j = 0; j < 2; j++)
				{
					px = (xPi+x0)/2 + j; if (px >= frame.Cwidth) px = frame.Cwidth-1;
					py = (yPi+y0)/2 + i; if (py >= frame.Cheight) py = frame.Cheight-1;
					frame.C[boja][((yP+y0)/2+i)*frame.Cwidth+((xP+x0)/2+j)] = refFrameInterpolated[frac].C[boja][py*frame.Cwidth+px];
				}
	}
}

int satdLuma8x8MVs(int mvx, int mvy, int luma8x8BlkIdx)
{
	int satd = 0, pretvori = 1;

	int xP = (CurrMbAddr%PicWidthInMbs)<<4;
	int yP = ((CurrMbAddr/PicWidthInMbs)<<4);
	int xPi = xP + (mvx>>2); if (xPi < 0) xPi = 0; if (xPi >= frame.Lwidth) xPi = frame.Lwidth-1;
	int yPi = yP + (mvy>>2); if (yPi < 0) yPi = 0; if (yPi >= frame.Lheight) yPi = frame.Lheight-1;
	int frac = (mvx&3) + (mvy&3)*4;
	//int diffL4x4[4][4], rLuma[4][4];
	int x0, y0, px, py;
	
	for (int part4x4idx = 0; part4x4idx < 4; part4x4idx++)
	{
		x0 = ((luma8x8BlkIdx%2) << 3) + ((part4x4idx%2) << 2);
		y0 = ((luma8x8BlkIdx&2) << 2) + ((part4x4idx&2) << 1);
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
			{
				px = xPi+x0+j; if (px >= frame.Lwidth) px = frame.Lwidth-1;
				py = yPi+y0+i; if (py >= frame.Lheight) py = frame.Lheight-1;
				satd += ABS(frame.L[(yP+y0+i)*frame.Lwidth+(xP+x0+j)] - refFrameInterpolated[frac].L[py*frame.Lwidth+px]);
				if (ABS(frame.L[(yP+y0+i)*frame.Lwidth+(xP+x0+j)] - refFrameInterpolated[frac].L[py*frame.Lwidth+px]) > 2)
					pretvori = 0;
			}
		for (int boja = 0; boja < 2; boja++)
			for (int i = 0; i < 2; i++)
				for (int j = 0; j < 2; j++)
				{
					px = (xPi+x0)/2 + j; if (px >= frame.Cwidth) px = frame.Cwidth-1;
					py = (yPi+y0)/2 + i; if (py >= frame.Cheight) py = frame.Cheight-1;
					satd += ABS(frame.C[boja][((yP+y0)/2+i)*frame.Cwidth+((xP+x0)/2+j)] - refFrameInterpolated[frac].C[boja][py*frame.Cwidth+px]);
					if (ABS((int)(frame.C[((yP+y0)/2+i)*frame.Cwidth+((xP+x0)/2+j)]) - refFrameInterpolated[frac].C[boja][py*frame.Cwidth+px]) > 6)
						pretvori = 0;
				}
		//forwardResidual(QPy, diffL4x4, rLuma, true, false);
		//for (int i = 0; i < 4; i++)
		//	for (int j = 0; j < 4; j++)
		//		satd += ABS(rLuma[i][j]);
	}
	if (pretvori) return 0;
	return satd;
}
//
//int satdLuma8x8(int predL[16][16], int luma8x8BlkIdx)
//{
//	int satd = 0;
//
//	int xP = ((CurrMbAddr%PicWidthInMbs)<<4);
//	int yP = ((CurrMbAddr/PicWidthInMbs)<<4);
//	int diffL4x4[4][4], rLuma[4][4], x0, y0;
//	
//	for (int part4x4idx = 0; part4x4idx < 4; part4x4idx++)
//	{
//		x0 = ((luma8x8BlkIdx%2) << 3) + ((part4x4idx%2) << 2);
//		y0 = ((luma8x8BlkIdx&2) << 2) + ((part4x4idx&2) << 1);
//		for (int i = 0; i < 4; i++)
//			for (int j = 0; j < 4; j++)
//				satd += ABS(frame.L[(yP+y0+i)*frame.Lwidth+(xP+x0+j)] - predL[y0+i][x0+j]);
//		//forwardResidual(QPy, diffL4x4, rLuma, true, false);
//		//for (int i = 0; i < 4; i++)
//		//	for (int j = 0; j < 4; j++)
//		//		satd += ABS(rLuma[i][j]);
//	}
//
//	return satd;
//}
//
//int zigZagSADLuma8x8(int predL[16][16], int luma8x8BlkIdx)
//{
//	int sad = 0;
//
//	int xP = ((CurrMbAddr%PicWidthInMbs)<<4);
//	int yP = ((CurrMbAddr/PicWidthInMbs)<<4), x0, y0;
//
//	for (int i = 0; i < 4; i++)
//	{
//		x0 = ((luma8x8BlkIdx%2) << 3) + ((i%2) << 2);
//		y0 = ((luma8x8BlkIdx&2) << 2) + ((i&2) << 1);
//		for (int j = 0; j < 16; j++)
//		{
//			tmpVar[j] = frame.L[(yP+y0+zigZagIdx[j][1])*frame.Lwidth+(xP+x0+zigZagIdx[j][0])] - predL[y0+zigZagIdx[j][1]][x0+zigZagIdx[j][0]];
//			if (j)
//				sad += ABS(tmpVar[j]-tmpVar[j-1]);
//			else 
//				sad += ABS(tmpVar[j]);
//		}
//	}
//
//	return sad;
//
//}
//
//int sadLuma8x8(int predL[16][16], int luma8x8BlkIdx)
//{
//	int sad = 0;
//
//	int xP = ((CurrMbAddr%PicWidthInMbs)<<4);
//	int yP = ((CurrMbAddr/PicWidthInMbs)<<4);
//
//	int x0 = (luma8x8BlkIdx%2) << 3;
//	int y0 = (luma8x8BlkIdx&2) << 2;
//
//	for (int i = 0; i < 8; i++)
//	{
//		for (int j = 0; j < 8; j++)
//		{
//			sad += ABS(frame.L[(yP+y0+i)*frame.Lwidth+(xP+x0+j)] - predL[x0+i][y0+j]);
//		}
//	}
//
//	return sad;
//}

int ExactPixels(int predL[16][16])
{
	int exactLumaPixels = 0;

	int xP = ((CurrMbAddr%PicWidthInMbs)<<4);
	int yP = ((CurrMbAddr/PicWidthInMbs)<<4);

	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			exactLumaPixels += (ABS(frame.L[(yP+i)*frame.Lwidth+(xP+j)]-predL[i][j])<=4)?0:1;
		}
	}

	return exactLumaPixels;	
}

//int sadLuma(int predL[16][16], int partId)
//{
//	if (mb_type == P_8x8 || mb_type == P_8x8ref0) return satdLuma8x8(predL, partId);
//	if (mb_type == P_L0_L0_16x8) return satdLuma8x8(predL, partId*2) + satdLuma8x8(predL, partId*2+1);
//	if (mb_type == P_L0_L0_8x16) return satdLuma8x8(predL, partId) + satdLuma8x8(predL, partId+2);
//	if (mb_type == P_L0_16x16 || mb_type == P_Skip) return satdLuma8x8(predL, 0) + satdLuma8x8(predL, 1) + satdLuma8x8(predL, 2) + satdLuma8x8(predL, 3);
//}

int sadLumaMVs(int mvx, int mvy, int partId)
{
	if (mb_type == P_8x8 || mb_type == P_8x8ref0) return satdLuma8x8MVs(mvx, mvy, partId);
	if (mb_type == P_L0_L0_16x8) return satdLuma8x8MVs(mvx, mvy, partId*2) + satdLuma8x8MVs(mvx, mvy, partId*2+1);
	if (mb_type == P_L0_L0_8x16) return satdLuma8x8MVs(mvx, mvy, partId) + satdLuma8x8MVs(mvx, mvy, partId+2);
	return satdLuma8x8MVs(mvx, mvy, 0) + satdLuma8x8MVs(mvx, mvy, 1) + satdLuma8x8MVs(mvx, mvy, 2) + satdLuma8x8MVs(mvx, mvy, 3);
}

void interEncoding(int predL[16][16], int predCr[8][8], int predCb[8][8]) 
{
	int xp = ((CurrMbAddr%PicWidthInMbs)<<4);
	int yp = ((CurrMbAddr/PicWidthInMbs)<<4);

	int minBlock = INT_MAX;
	int bestMbType = P_Skip, mvdL0[4][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, mvx[4], mvy[4];
	ClearMVD();
	mb_type = P_Skip;
	mb_type_array[CurrMbAddr] = P_Skip;
	DeriveMVs();
	Decode(predL, predCr, predCb);
	if (!ExactPixels(predL)) return;
	//printf("%d\n", CurrMbAddr);
	//for (int curr_mbtype = 0; curr_mbtype < 3; curr_mbtype++)
	//{
		mb_type = P_8x8ref0;
		mb_type_array[CurrMbAddr] = P_8x8ref0;
		ClearMVD();
		//int currMin = 0;
		for (int i = 0; i < NumMbPart(mb_type); i++)
		{
			//int currStepSize = 8, currMbSadMin = INT_MAX;
			//int mvdx = 0, mvdy = 0;
			mvd_l0[i][0][0] = mvd_l0[i][0][1] = 0;
			DeriveMVs();
			int genx = mvL0x[CurrMbAddr][i][0] >> 2;
			int geny = mvL0y[CurrMbAddr][i][0] >> 2;
			int suma[6] = {0, 0, 0, 0, 0, 0}, relx = (i%2) * 8, rely = (i/2) * 8;
			if (CurrMbAddr == 235)
			{
				int u = 1;
			}
			for (int tx = 0; tx < 8; tx++)
			{
				for (int ty = 0; ty < 8; ty++)
				{
					suma[0] += frame.L[(ty+rely+yp)*frame.Lwidth+(tx+relx+xp)];
					suma[1] += ABS(frame.L[(ty+rely+yp)*frame.Lwidth+(tx+relx+xp)]-128);
					suma[2] += transformedLuma(frame.L[(ty+rely+yp)*frame.Lwidth+(tx+relx+xp)]);
					suma[3] += (tx>3)?0:frame.L[(ty+rely+yp)*frame.Lwidth+(tx+relx+xp)];
					suma[4] += (ty>3)?0:frame.L[(ty+rely+yp)*frame.Lwidth+(tx+relx+xp)];
				}
				int u = 1;
			}
			int bx = 0, by = 0, bmin = 1000000000, refx, refy, trenRazlika;
			int bxs[85], bys[85], bmins[85];
			for (int j = 0; j < 85; j++)
			{
				bmins[j] = 1000000000;
				bxs[j] = bys[j] = 100000000;
			}
			for (int tmpx = genx-4; tmpx <= genx+4; tmpx++)
			{
				for (int tmpy = geny-4; tmpy <= geny+4; tmpy++)
				{
					for (int frac = 0; frac < 16; frac++)
					{
						refx = relx+xp+tmpx;
						refy = rely+yp+tmpy;
						if (refy >= 0 && refy < frame.Lheight && refx>=0 && refx<frame.Lwidth) 
						{
							trenRazlika = (8+ABS(frac/4)+ABS(tmpx)+ABS(2*tmpy))*( ABS(suma[0]-refFrameKar[0][frac][refy][refx])
											+ ABS(suma[1]-refFrameKar[1][frac][refy][refx])
											+ diffTransformedLuma(suma[2], refFrameKar[2][frac][refy][refx])
											+ ABS(suma[3]-refFrameKar[3][frac][refy][refx])
											+ ABS(suma[4]-refFrameKar[4][frac][refy][refx])
											+ ABS(suma[0]-suma[3]+refFrameKar[3][frac][refy][refx]-refFrameKar[0][frac][refy][refx])
											+ ABS(suma[0]-suma[4]+refFrameKar[4][frac][refy][refx]-refFrameKar[0][frac][refy][refx])
											);
							if (bmins[51] > trenRazlika)
							bmins[52] = trenRazlika;
							bxs[52] = (tmpx<<2) | (frac&3);
							bys[52] = (tmpy<<2) | ((frac>>2)&3);
							for (int j = 52; j > 0; j--)
							{
								if (bmins[j] < bmins[j-1]) 
								{
									int t1 = bmins[j]; bmins[j] = bmins[j-1]; bmins[j-1] = t1;
									t1 = bxs[j]; bxs[j] = bxs[j-1]; bxs[j-1] = t1;
									t1 = bys[j]; bys[j] = bys[j-1]; bys[j-1] = t1;
								} else break;
							}
						}
					}
				}
			}
			for (int tmpx = genx-128; tmpx <= genx+128; tmpx+=8)
			{
				for (int tmpy = geny-128; tmpy <= geny+128; tmpy+=8)
				{
					for (int frac = 0; frac < 1; frac++)
					{
						refx = relx+xp+tmpx;
						refy = rely+yp+tmpy;
						if (refy >= 0 && refy < frame.Lheight && refx>=0 && refx<frame.Lwidth) 
						{
							trenRazlika = (8+ABS(frac/4)+ABS(tmpx)+ABS(2*tmpy))*( ABS(suma[0]-refFrameKar[0][frac][refy][refx])
											+ ABS(suma[1]-refFrameKar[1][frac][refy][refx])
											//+ ABS(suma[2]-refFrameKar[2][frac][refy][refx])
											+ diffTransformedLuma(suma[3], refFrameKar[3][frac][refy][refx])
											+ ABS(suma[4]-refFrameKar[4][frac][refy][refx])
											+ ABS(suma[5]-refFrameKar[5][frac][refy][refx])
											+ ABS(suma[0]-suma[4]+refFrameKar[4][frac][refy][refx]-refFrameKar[0][frac][refy][refx])
											+ ABS(suma[0]-suma[5]+refFrameKar[5][frac][refy][refx]-refFrameKar[0][frac][refy][refx])
											//+ ABS(suma[0]-suma[1]+refFrameKar[1][frac][refy][refx]-refFrameKar[0][frac][refy][refx])
											//+ ABS(suma[0]-suma[2]+refFrameKar[2][frac][refy][refx]-refFrameKar[0][frac][refy][refx])
											//+ ABS(suma[0]-suma[3]+refFrameKar[3][frac][refy][refx]-refFrameKar[0][frac][refy][refx])
											);
							if (bmins[83] > trenRazlika)
							bmins[84] = trenRazlika;
							bxs[84] = (tmpx<<2) | (frac&3);
							bys[84] = (tmpy<<2) | ((frac>>2)&3);
							for (int j = 84; j > 52; j--)
							{
								if (bmins[j] < bmins[j-1]) 
								{
									int t1 = bmins[j]; bmins[j] = bmins[j-1]; bmins[j-1] = t1;
									t1 = bxs[j]; bxs[j] = bxs[j-1]; bxs[j-1] = t1;
									t1 = bys[j]; bys[j] = bys[j-1]; bys[j-1] = t1;
								} else break;
							}
						}
					}
				}
			}

			bmin = 2000000000;
			//for (int jx = -3; jx <= 3; jx++)
			//	for (int jy = -3; jy <= 3; jy++)
			//	{
			//		int tmpdif = sadLumaMVs(jx, jy, i);
			//		if (tmpdif < bmin)
			//		{
			//			bmin = tmpdif;
			//			bx = jx;
			//			by = jy;
			//		}
			//	}
			//bxs[20] = 0; bys[20] = -3;
			for (int j = 0; j <= 84; j++)
			if (bxs[j] < 100000000 && bys[j] < 100000000){
				int tmpdif = sadLumaMVs(bxs[j], bys[j], i);
				if (tmpdif < bmin)
				{
					bmin = tmpdif;
					bx = bxs[j];
					by = bys[j];
				}
			}
			//printf("%d.%d --> [%d.%d] = %d\n", CurrMbAddr, i, bx, by, bmin);
			//int bx = 0, by = 0, bmin = INT_MAX;
			//mvd_l0[i][0][0] = 0;
			//mvd_l0[i][0][1] = 0;
			//Decode(predL, predCr, predCb);
			//bmin = sadLumaMVs(0, 0, i);
			//for (int tmpx = -64; tmpx <= 64; tmpx+=4)
			//	for (int tmpy = -64; tmpy <= 64; tmpy+=4)
			//	{
			//		int tmpdif = sadLumaMVs(tmpx, tmpy, i);
			//		if (tmpdif < bmin)
			//		{
			//			bmin = tmpdif;
			//			bx = tmpx; by = tmpy;
			//		}
			//	}
			//for (int tmpx = -4; tmpx <= 4; tmpx++)
			//{
			//	for (int tmpy = -4; tmpy <= 4; tmpy++)
			//	{
			//		int tmpdif = sadLumaMVs(tmpx+genx, tmpy+geny, i);
			//		if (tmpdif < bmin)
			//		{
			//			bmin = tmpdif;
			//			bx = tmpx+genx; by = tmpy+geny;
			//		}
			//	}
			//}
			if (bmin == 0)
			{
				Pogorsaj(bx, by, i);
			}
			mvx[i] = bx; mvy[i] = by;
			bx -= mvL0x[CurrMbAddr][i][0];
			by -= mvL0y[CurrMbAddr][i][0];
			mvd_l0[i][0][0] = mvdL0[i][0] = bx;
			mvd_l0[i][0][1] = mvdL0[i][1] = by;
			//bmin = sadLuma(predL, i);
			//for (int tmpx = -2; tmpx <= 2; tmpx+=1)
			//	for (int tmpy = -2; tmpy <= 2; tmpy+=1)
			//	{
			//		int tmpdif = sadLumaMVs(tmpx+genx, tmpy+geny, i);
			//		if (tmpdif < bmin)
			//		{
			//			bmin = tmpdif;
			//			bx = tmpx; by = tmpy;
			//		}
			//	}
			//mvd_l0[i][0][0] = mvdL0[i][0] = bx;
			//mvd_l0[i][0][1] = mvdL0[i][1] = by;
			//DeriveMVs();
			//Decode(predL, predCr, predCb);
			//mvd_l0[i][0][0] = bx;
			//mvd_l0[i][0][1] = by;
			//for (int tx = 0; tx <= 1; tx+=1)
			//	for (int ty = 0; ty <= 1; ty+=1)
			//	if (tx+ty) {
			//		mvd_l0[i][0][0] += tx;
			//		mvd_l0[i][0][1] += ty;
			//		DeriveMVs();
			//		Decode(predL, predCr, predCb);
			//		int trenSad = sadLuma(predL, i);
			//		if (trenSad < bmin) 
			//		{
			//			bmin = trenSad;
			//			bx += tx; by += ty;
			//		}
			//		mvd_l0[i][0][0] -= tx;
			//		mvd_l0[i][0][1] -= ty;
			//	}
			//currMin += bmin;
			//mvd_l0[i][0][0] = bx;
			//mvd_l0[i][0][1] = by;
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
	//	if (currMin < minBlock)
	//	{
	//		minBlock = currMin;
	//		bestMbType = curr_mbtype;
	//		for (int i = 0; i < 4; i++)
	//		{
	//			mvdL0[i][0] = mvd_l0[i][0][0];
	//			mvdL0[i][1] = mvd_l0[i][0][1];
	//		}
	//	}
	//}
}