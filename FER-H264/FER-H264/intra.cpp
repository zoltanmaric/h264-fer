#include <stdlib.h>

#include "intra.h"

unsigned char Intra4x4Scan[16][2] = {
  { 0, 0},  { 4, 0},  { 0, 4},  { 4, 4},
  { 8, 0},  {12, 0},  { 8, 4},  {12, 4},
  { 0, 8},  { 4, 8},  { 0,12},  { 4,12},
  { 8, 8},  {12, 8},  { 8,12},  {12,12}
};

// This function is a stub. It simulates the retrieval of the
// prev_intra4x4_pred_mode_flag from the byte stream.
bool get_prev_intra4x4_pred_mode_flag(int luma4x4BlkIdx)
{
	return true;
}

// This function is a stub. It simulates the retrieval of the
// rem_intra4x4_pred_mode from the byte stream.
int get_rem_intra4x4_pred_mode(int luma4x4BlkIdx)
{
	return 5;
}

// This is a helper function used for various purposes
// throughout the decoder. It is defined in subclause
// 5.7 Mathematical functions
int InverseRasterScan(int a, int b, int c, int d, int e)
{
	if (e == 0)
		return (a % (d/b)) * b;
	else
		return (a / (d/b)) * c;
}

// Defined in 5.7 Mathematical functions
int Clip1Y(int x)
{
	// Clip3(0, 255, x); 255 == (1 << BitDepthY) - 1; BitDepthY == 8 when baseline
	if (x < 0)
		return 0;
	if (x > 255)
		return 255;

	return x;
}

// Derivation process for neighbouring locations (6.4.11)
void getNeighbourLocations(int xN, int yN, int *mbAddrN, int CurrMbAddr, int mbWidth, int *xW, int *yW)
{
	// width and height of the macroblock
	int maxW = 16;
	int maxH = 16;
	
	// if xN and yN are within this macroblock
	if ((xN >= 0) && (xN < maxW) &&
		(yN >= 0) && (yN < maxH))
		// this is the current macroblock
		*mbAddrN = CurrMbAddr;
	
	// if (xN,yN) is left from the current macroblock
	else if ((xN < 0) && (yN >= 0) &&
			 (yN < maxH))
	{
		 if ((CurrMbAddr % mbWidth) != 0)
			 // this is macroblack A
			 *mbAddrN = CurrMbAddr - 1;
		 else
			 //this macroblock is not available (out of frame)
			 *mbAddrN = -1;
	}
	
	// if (xN,yN) is above the current macroblock
	else if ((xN >= 0) && (xN < maxW) &&
			 (yN < 0))
	{
		if (CurrMbAddr >= mbWidth)
			// this is macroblock B
			*mbAddrN = CurrMbAddr - mbWidth;
		else
			// this macroblock is not available (out of frame)
			*mbAddrN = -1;
	}

	// if (xN,yN) is above and to the right from the current macroblock
	else if ((xN >= maxW) && (yN < 0))
	{
		if (((CurrMbAddr % mbWidth) != (mbWidth - 1)) &&
			(CurrMbAddr >= mbWidth))
			// this is macroblock C
			*mbAddrN = CurrMbAddr - (mbWidth - 1);
		else
			// this macroblock is not available (out of frame)
			*mbAddrN = -1;
	}

	// if (xN,yN) is above and to the left from the current macroblock
	else if ((xN < 0) && (yN < 0))
	{
		if (((CurrMbAddr % mbWidth) != 0) &&
			(CurrMbAddr >= mbWidth))
			// this is macroblock D
			*mbAddrN = CurrMbAddr - (mbWidth + 1);
		else
			// this macroblock is not available (out of frame)
			*mbAddrN = -1;
	}
	else
		*mbAddrN = -1;

	// position of the edge sample in the adjacent 4x4 block
	// relative to the top left sample in macroblock N
	*xW = (xN + maxW) % maxW;
	*yW = (yN + maxH) % maxH;
}

// Derivation process for neighbouring 4x4 luma blocks (6.4.10.4)
// nA - neighbour (A if true, B if false, see figure 6-12)
void getNeighbourAddresses(int CurrMbAddr, int luma4x4BlkIdx, bool nA,
							int mbWidth, int *mbAddrN, int *luma4x4BlkIdxN)
{
	int xD, yD;			// table 6-2
	// Neighbour A
	if (nA == true)
	{
		xD = -1;
		yD = 0;
	}
	// Neighbour B
	else
	{
		xD = 0;
		yD = -1;
	}

	// Luma block scanning process (6.4.3)
	// position of the top left sample in the 4x4 block
	// relative to the top left sample in the current macroblock
	int x = InverseRasterScan(luma4x4BlkIdx / 4, 8, 8, 16, 0) +
			InverseRasterScan(luma4x4BlkIdx % 4, 4, 4, 8, 0);
	int y = InverseRasterScan(luma4x4BlkIdx / 4, 8, 8, 16, 1) +
			InverseRasterScan(luma4x4BlkIdx % 4, 4, 4, 8, 1);

	// position of the edge sample in the adjacent 4x4 block
	// relative to the top left sample in the current macroblock
	int xN = x + xD;
	int yN = y + yD;

	int xW, yW;
	getNeighbourLocations(xN, yN, mbAddrN, CurrMbAddr, mbWidth, &xW, &yW);

	if (*mbAddrN != -1)
		*luma4x4BlkIdxN = 8 * (yW / 8) + 4 * (xW / 8) + 2 * ((yW % 8) / 4) + ((xW % 8) / 4);
	else
		*luma4x4BlkIdxN = -1;
}

// Derivation process for Intra4x4PredMode (8.3.1.1)
void getIntra4x4PredMode(int luma4x4BlkIdx, mode_pred_info &mpi, int CurrMbAddr, int *mbAddrA, int *mbAddrB)
{
	bool prev_intra4x4_pred_mode_flag = get_prev_intra4x4_pred_mode_flag(luma4x4BlkIdx);
	int rem_intra4x4_pred_mode;
	if (prev_intra4x4_pred_mode_flag == false)
		rem_intra4x4_pred_mode = get_rem_intra4x4_pred_mode(luma4x4BlkIdx);

	int luma4x4BlkIdxA, luma4x4BlkIdxB;
	getNeighbourAddresses(CurrMbAddr, luma4x4BlkIdx, true, mpi.MbWidth,
							mbAddrA, &luma4x4BlkIdxA);
	getNeighbourAddresses(CurrMbAddr, luma4x4BlkIdx, false, mpi.MbWidth,
							mbAddrB, &luma4x4BlkIdxB);

	// no checking whether the neighbouring
	// macroblocks are coded as P-macroblocks
	bool dcPredModePredictedFlag = false;
	if ((*mbAddrA == -1) || (*mbAddrB == -1))
		dcPredModePredictedFlag = true;

	int absIdx = CurrMbAddr * 16 + luma4x4BlkIdx;
	if ((dcPredModePredictedFlag == true) ||	// if dc mode is predicted
		(mpi.MbMode[*mbAddrA] != I_NxN) ||	// or the neighbouring
		(mpi.MbMode[*mbAddrB] != I_NxN))		// macroblocks are not using 4x4 prediction
	{
		mpi.Intra4x4PredMode[absIdx] = 2;		// dc prediction mode
	}
	else
	{
		// calculate the absolute addresses of the 4x4 blocks
		int absIdxA = *mbAddrA * 16 + luma4x4BlkIdxA;
		int absIdxB = *mbAddrB * 16 + luma4x4BlkIdxB;
		int intraMxMPredModeA = mpi.Intra4x4PredMode[absIdxA];
		int intraMxMPredModeB = mpi.Intra4x4PredMode[absIdxB];
		// the intra4x4 prediction mode for the current block is the more probable
		// prediction mode of the neighbouring two blocks; the more probable mode
		// has the smaller value
		int predIntra4x4PredMode = (intraMxMPredModeA <= intraMxMPredModeB) ? intraMxMPredModeA : intraMxMPredModeB;
		if (prev_intra4x4_pred_mode_flag == true)
		{
			mpi.Intra4x4PredMode[absIdx] = predIntra4x4PredMode;
		}
		else
		{
			if (rem_intra4x4_pred_mode < predIntra4x4PredMode)
				mpi.Intra4x4PredMode[absIdx] = rem_intra4x4_pred_mode;
			else
				mpi.Intra4x4PredMode[absIdx] = rem_intra4x4_pred_mode + 1;
		}
	}
}

#define p(x,y) (((x) == -1) ? p[(y) + 1] : p[(x) + 5])

// (8.3.1.2.1)
void Intra_4x4_Vertical(int *p, int *pred4x4L)
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			pred4x4L[y * 4 + x] = p(x,-1);
		}
	}
}

// (8.3.1.2.2)
void Intra_4x4_Horizontal(int *p, int *pred4x4L)
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			pred4x4L[y * 4 + x] = p(-1,y);
		}
	}
}

// (8.3.1.2.3)
void Intra_4x4_DC(int *p, int *pred4x4L)
{
	bool allAvailable = true;
	bool leftAvailable = true;
	bool topAvailable = true;

	for (int i = 0; i < 13; i++)
	{
		if (p[i] == -1)
		{
			allAvailable = false;
			if ((i > 0) && (i < 5))
				leftAvailable = false;
			else if ((i >= 5) && (i < 9))
				topAvailable = false;
		}
	}
	
	if (allAvailable)
	{
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				pred4x4L[y * 4 + x] = (p(0,-1) + p(1,-1) + p(2,-1) + p(3,-1) +
									   p(-1,0) + p(-1,1) + p(-1,2) + p(-1,3) + 4) >> 3;
			}
		}
	}
	else if (leftAvailable)
	{
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				pred4x4L[y * 4 + x] = (p(-1,0) + p(-1,1) + p(-1,2) + p(-1,3) + 2) >> 2;
			}
		}
	}
	else if (topAvailable)
	{
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				pred4x4L[y * 4 + x] = (p(0,-1) + p(1,-1) + p(2,-1) + p(3,-1) + 2) >> 2;
			}
		}
	}
	else
	{
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				pred4x4L[y * 4 + x] = 128; // = (1 << (BitDepthY - 1); BitDepthY = 8 in baseline
			}
		}
	}
}

// (8.3.1.2.4)
void Intra_4x4_Diagonal_Down_Left(int *p, int *pred4x4L)
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			if ((x == 3) && (y == 3))
				pred4x4L[y * 4 + x] = (p(6,-1) + 3*p(7,-1) + 2) >> 2;
			else
				pred4x4L[y * 4 + x] = (p(x+y,-1) + 2*p(x+y+1,-1) + p(x+y+2,-1) + 2) >> 2;
		}
	}
}

// (8.3.1.2.5)
void Intra_4x4_Diagonal_Down_Right(int *p, int *pred4x4L)
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			if (x > y)
				pred4x4L[y * 4 + x] = (p(x-y-2,-1) + 2*p(x-y-1,-1) + p(x-y,-1) + 2) >> 2;
			else if (x < y)
				pred4x4L[y * 4 + x] = (p(-1,y-x-2) + 2*p(-1,y-x-1) + p(-1,y-x) + 2) >> 2;
			else
				pred4x4L[y * 4 + x] = (p(0,-1) + 2*p(-1,-1) + p(-1,0) + 2) >> 2;
		}
	}
}

// (8.3.1.2.6)
void Intra_4x4_Vertical_Right(int *p, int *pred4x4L)
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			int zVR = 2 * x - y;
			if ((zVR == 0) || (zVR == 2) ||
				(zVR == 4) || (zVR == 6))
				pred4x4L[y * 4 + x] = (p(x-(y>>1)-1,-1) + p(x-(y>>1),-1) + 1) >> 1;
			else if ((zVR == 1) ||
				(zVR == 3) || (zVR == 5))
				pred4x4L[y * 4 + x] = (p(x-(y>>1)-2,-1) + 2*p(x-(y>>1)-1,-1) + p(x-(y>>1),-1) + 2) >> 2;
			else if (zVR == -1)
				pred4x4L[y * 4 + x] = (p(-1,0) + 2*p(-1,-1) + p(0,-1) + 2) >> 2;
			else
				pred4x4L[y * 4 + x] = (p(-1,y-1) + 2*p(-1,y-2) + p(-1,y-3) + 2) >> 2;
		}
	}
}

// (8.3.1.2.7)
void Intra_4x4_Horizontal_Down(int *p, int *pred4x4L)
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			int zHD = 2 * y - x;
			if ((zHD == 0) || (zHD == 2) ||
				(zHD == 4) || (zHD == 6))
				pred4x4L[y * 4 + x] = (p(-1,y-(x>>1)-1) + p(-1,y-(x>>1)) + 1) >> 1;
			else if ((zHD == 1) ||
				(zHD == 3) || (zHD == 5))
				pred4x4L[y * 4 + x] = (p(-1,y-(x>>1)-2) + 2*p(-1,y-(x>>1)-1) + p(-1,y-(x>>1)) + 2) >> 2;
			else if (zHD == -1)
				pred4x4L[y * 4 + x] = (p(-1,0) + 2*p(-1,-1) + p(0,-1) + 2) >> 2;
			else
				pred4x4L[y * 4 + x] = (p(x-1,-1) + 2*p(x-2,-1) + p(x-3,-1) + 2) >> 2;
		}
	}
}

// (8.3.1.2.8)
void Intra_4x4_Vertical_Left(int *p, int *pred4x4L)
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			if ((y == 0) || (y==2))
				pred4x4L[y * 4 + x] = (p(x+(y>>1),-1) + p(x+(y>>1)+1,-1) + 1) >> 1;
			else
				pred4x4L[y * 4 + x] = (p(x-(y>>1),-1) + 2*p(x+(y>>1)+1,-1) + p(x+(y>>1)+2,-1) + 2) >> 2;
		}
	}
}

// (8.3.1.2.9)
void Intra_4x4_Horizontal_Up(int *p, int *pred4x4L)
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			int zHU = x + 2 * y;
			if ((zHU == 0) || (zHU == 2) ||
				(zHU == 4))
				pred4x4L[y * 4 + x] = (p(-1,y+(x>>1)) + p(-1,y+(x>>1)+1) + 1) >> 1;
			else if ((zHU == 1) || (zHU == 3))
				pred4x4L[y * 4 + x] = (p(-1,y+(x>>1)) + 2*p(-1,y+(x>>1)+1) + p(-1,y+(x>>1)+2) + 2) >> 2;
			else if (zHU == 5)
				pred4x4L[y * 4 + x] = (p(-1,2) + 3*p(-1,3) + 2) >> 2;
			else
				pred4x4L[y * 4 + x] = p(-1,3);
		}
	}
}

// Intra_4x4 sample prediction (8.3.1.2)
void Intra4x4SamplePrediction(int luma4x4BlkIdx, int CurrMbAddr, int mbWidth, frame &f, int intra4x4PredMode, int *pred4x4L)
{
	int x0 = Intra4x4Scan[luma4x4BlkIdx][0];
	int y0 = Intra4x4Scan[luma4x4BlkIdx][1];

	int p[13];
	for (int i = 0; i < 13; i++)
	{
		int x, y;
		if (i < 5)
		{
			x = -1;
			y = i - 1;
		}
		else
		{
			x = i - 5;
			y = -1;
		}

		int xN = x0 + x;
		int yN = y0 + y;

		int mbAddrN, xW, yW;
		getNeighbourLocations(xN, yN, &mbAddrN, CurrMbAddr, mbWidth, &xW, &yW);

		if ((mbAddrN == -1) ||
			((x > 3) && ((luma4x4BlkIdx == 3) || (luma4x4BlkIdx == 1))))
		{
			// if this is an edge macroblock, replicate
			// the edge sample p[3,-1] into the samples p[4..7,-1]
			if ((i > 8) && (p[8] != -1))
				p[i] = p[8];
			else
				p[i] = -1;	// not available for intra prediction
		}
		else
		{
			// the abosulte position of the upper-left sample in macroblock N (6.4.1)
			int xM = InverseRasterScan(mbAddrN, 16, 16, f.Lwidth, 0);
			int yM = InverseRasterScan(mbAddrN, 16, 16, f.Lwidth, 1);

			p[i] = f.L[(yM + yW) * f.Lwidth + (xM + xW)];
		}
	}

	switch (intra4x4PredMode)
	{
		case 0:
			Intra_4x4_Vertical(p, pred4x4L);
			break;
		case 1:
			Intra_4x4_Horizontal(p, pred4x4L);
			break;
		case 2:
			Intra_4x4_DC(p, pred4x4L);
			break;
		case 3:
			Intra_4x4_Diagonal_Down_Left(p, pred4x4L);
			break;
		case 4:
			Intra_4x4_Diagonal_Down_Right(p, pred4x4L);
			break;
		case 5:
			Intra_4x4_Vertical_Right(p, pred4x4L);
			break;
		case 6:
			Intra_4x4_Horizontal_Down(p, pred4x4L);
			break;
		case 7:
			Intra_4x4_Vertical_Left(p, pred4x4L);
			break;
		case 8:
			Intra_4x4_Horizontal_Up(p, pred4x4L);
			break;
		default:
			break;
	}
}

#undef p(x,y)

#define p(x,y) (((x) == -1) ? p[(y) + 1] : p[(x) + 17])
// (8.3.3.1)
void Intra_16x16_Vertical(int *p, int *predL, int x, int y)
{
	predL[y*16 + x] = p(x,-1);
}

// (8.3.3.2)
void Intra_16x16_Horizontal(int *p, int *predL, int x, int y)
{
	predL[y*16 + x] = p(-1,y);
}

// (8.3.3.3)
void Intra_16x16_DC(int *p, int *predL, int x, int y)
{
	int sumXi = 0;		// = sum(p[x',-1]) | x € (0..15)
	int sumYi = 0;		// = sum(p[-1,y']) | y € (0..15)
	for (int i = 0; i < 16; i++)
	{
		sumXi += p(i,-1);
		sumYi += p(-1,i);
	}

	// check availability of neighbouring samples:
	bool allAvailable = true;
	bool leftAvailable = true;
	bool topAvailable = true;
	for (int i = 0; i < 33; i++)
	{
		if (p[i] == -1)
		{
			allAvailable = false;
			if ((i > 0) && (i < 17))
				leftAvailable = false;
			else if ((i >= 17) && (i < 33))
				topAvailable = false;
		}
	}

	if (allAvailable)
	{
		predL[y*16+x] = (sumXi + sumYi + 16) >> 5;
	}
	else if (topAvailable)
	{
		predL[y*16+x] = (sumYi + 8) >> 4;
	}
	else if (leftAvailable)
	{
		predL[y*16+x] = (sumXi + 8) >> 4;
	}
	else
	{
		predL[y*16+x] = 1 << 7;		// == 1 << (BitDepthY-1) (BitDepthY is always equal to 8 in the baseline profile)
	}
}

// (8.3.3.4)
void Intra_16x16_Plane(int *p, int *predL, int x, int y)
{
	int H = 0, V = 0;
	for (int i = 0; i < 7; i++)
	{
		H += (i+1)*(p(8+i,-1) - p(6-i,-1));
		V += (i+1)*(p(-1,8+1) - p(-1,6-i));
	}

	int a = 16 * (p(-1,15) + p(15,-1));
	int b = (5*H + 32) >> 6;
	int c = (5*V + 32) >> 6;

	predL[y*16+x] = Clip1Y((a + b*(x-7) + c*(y-7) + 16) >> 5);
}

// (8.3.3)
void Intra16x16SamplePrediction(int CurrMbAddr, int mbWidth, frame &f, int *predL, int Intra16x16PredMode)
{
	int p[33];
	for (int i = 0; i < 33; i++)
	{
		int x, y;
		if (i < 17)
		{
			x = -1;
			y = i - 1;
		}
		else
		{
			x = i - 17;
			y = -1;
		}

		int xW, yW, mbAddrN;
		getNeighbourLocations(x, y, &mbAddrN, CurrMbAddr, mbWidth, &xW, &yW);

		if (mbAddrN == -1)
			p(x,y) = -1;	// not available for Intra_16x16 prediction

		// inverse macroblock scanning process (6.4.1)
		int xM = InverseRasterScan(mbAddrN, 16, 16, f.Lwidth, 0);
		int yM = InverseRasterScan(mbAddrN, 16, 16, f.Lwidth, 1);

		p(x,y) = f.L[xM+xW, yM+yW];

		switch(Intra16x16PredMode)
		{
			case 0:
				Intra_16x16_Vertical(p, predL, x, y);
				break;
			case 1:
				Intra_16x16_Horizontal(p, predL, x, y);
				break;
			case 2:
				Intra_16x16_DC(p, predL, x, y);
				break;
			case 3:
				Intra_16x16_Plane(p, predL, x, y);
				break;				
		}
	}
}

#undef p(x,y)

// predL, predCr and predCb are the output prediction samples
// with dimensions 16x16, 8x8 and 8x8 respectively
void intraPrediction(frame &f, mode_pred_info &mpi, mb_mode mb, int CurrMbAddr, int *predL, int *predCr, int *predCb)
{
	bool prev_intra4x4_pred_mode_flag[16];
	int rem_intra4x4_pred_mode[16];

	// mb_pred(mb_type) in the standard
	if ((mb.MbPartPredMode[0] == Intra_4x4) ||
		(mb.MbPartPredMode[0] == Intra_16x16))
	{
		// LUMA:

		if (mb.MbPartPredMode[0] == Intra_4x4)
		{
			for (int luma4x4BlkIdx = 0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
			{
				int mbAddrA, mbAddrB;
				getIntra4x4PredMode(luma4x4BlkIdx, mpi, CurrMbAddr, &mbAddrA, &mbAddrB);

				// the luma prediction samples
				int *pred4x4L;
				pred4x4L = (int *)malloc(16 * sizeof(int));
				int absIdx = CurrMbAddr * 16 + luma4x4BlkIdx;
				Intra4x4SamplePrediction(luma4x4BlkIdx, CurrMbAddr, mpi.MbWidth, f, mpi.Intra4x4PredMode[absIdx], pred4x4L);

				int x0 = Intra4x4Scan[luma4x4BlkIdx][0];
				int y0 = Intra4x4Scan[luma4x4BlkIdx][1];

				for (int y = 0; y < 4; y++)
				{
					for (int x = 0; x < 4; x++)
					{
						predL[(y0 + y)*16 + (x0 + x)] = pred4x4L[y*4 + x];
					}
				}

				int test = 0;
			}

			// the transform coefficient decoding part comes here (8.5)
		}
		else
		{
			Intra16x16SamplePrediction(CurrMbAddr, mpi.MbWidth, f, predL, mb.Intra16x16PredMode);
		}

		// CHROMA:
		IntraChromaSamplePrediction();




		// TODO: ChromaArrayType handling (see mb_pred(mb_type))
	}	
}