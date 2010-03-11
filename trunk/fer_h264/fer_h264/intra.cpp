#include "intra.h"
#include "inttransform.h"
#include "residual.h"
#include "h264_math.h"
#include "h264_globals.h"
#include "headers_and_parameter_sets.h"

// Derivation process for neighbouring locations (6.4.11)
// luma: true if invoked for luma locations, false if invoked for chroma
void getNeighbourLocations(int xN, int yN, int *mbAddrN, int *xW, int *yW, bool luma)
{
	// width and height of the macroblock
	int maxW, maxH;
	if (luma)
	{
		maxW = 16;
		maxH = 16;
	}
	else
	{
		maxW = 8;		// Standard: maxW = MbWidthC;
		maxH = 8;		// Standard: maxH = MbHeightC;
	}
	
	// if xN and yN are within this macroblock
	if ((xN >= 0) && (xN < maxW) &&
		(yN >= 0) && (yN < maxH))
		// this is the current macroblock
		*mbAddrN = CurrMbAddr;
	
	// if (xN,yN) is left from the current macroblock
	else if ((xN < 0) && (yN >= 0) &&
			 (yN < maxH))
	{
		 if ((CurrMbAddr % PicWidthInMbs) != 0)
			 // this is macroblock A
			 *mbAddrN = CurrMbAddr - 1;
		 else
			 //this macroblock is not available (out of frame)
			 *mbAddrN = -1;
	}
	
	// if (xN,yN) is above the current macroblock
	else if ((xN >= 0) && (xN < maxW) &&
			 (yN < 0))
	{
		if (CurrMbAddr >= PicWidthInMbs)
			// this is macroblock B
			*mbAddrN = CurrMbAddr - PicWidthInMbs;
		else
			// this macroblock is not available (out of frame)
			*mbAddrN = -1;
	}

	// if (xN,yN) is above and to the right from the current macroblock
	else if ((xN >= maxW) && (yN < 0))
	{
		if (((CurrMbAddr % PicWidthInMbs) != (PicWidthInMbs - 1)) &&
			(CurrMbAddr >= PicWidthInMbs))
			// this is macroblock C
			*mbAddrN = CurrMbAddr - (PicWidthInMbs - 1);
		else
			// this macroblock is not available (out of frame)
			*mbAddrN = -1;
	}

	// if (xN,yN) is above and to the left from the current macroblock
	else if ((xN < 0) && (yN < 0))
	{
		if (((CurrMbAddr % PicWidthInMbs) != 0) &&
			(CurrMbAddr >= PicWidthInMbs))
			// this is macroblock D
			*mbAddrN = CurrMbAddr - (PicWidthInMbs + 1);
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
void getNeighbourAddresses(int luma4x4BlkIdx, bool nA, int *mbAddrN, int *luma4x4BlkIdxN)
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
	getNeighbourLocations(xN, yN, mbAddrN, &xW, &yW, true);

	if (*mbAddrN != -1)
		*luma4x4BlkIdxN = 8 * (yW / 8) + 4 * (xW / 8) + 2 * ((yW % 8) / 4) + ((xW % 8) / 4);
	else
		*luma4x4BlkIdxN = -1;
}

// Derivation process for Intra4x4PredMode (8.3.1.1)
void getIntra4x4PredMode(int luma4x4BlkIdx, int *mbAddrA, int *mbAddrB)
{
	int luma4x4BlkIdxA, luma4x4BlkIdxB;
	getNeighbourAddresses(luma4x4BlkIdx, true, mbAddrA, &luma4x4BlkIdxA);
	getNeighbourAddresses(luma4x4BlkIdx, false, mbAddrB, &luma4x4BlkIdxB);

	// no checking whether the neighbouring
	// macroblocks are coded as P-macroblocks
	bool dcPredModePredictedFlag = false;
	if ((*mbAddrA == -1) || (*mbAddrB == -1))
		dcPredModePredictedFlag = true;

	int absIdx = CurrMbAddr * 16 + luma4x4BlkIdx;
	int intraMxMPredModeA, intraMxMPredModeB;
	if (dcPredModePredictedFlag == true)	// if dc mode is predicted
	{
		intraMxMPredModeA = 2;
		intraMxMPredModeB = 2;
	}
	else
	{
		if (mb_type_array[*mbAddrA] != I_4x4)		// if the macroblock of the neighbouring
		{											// submacroblock is not using 4x4 prediction
			intraMxMPredModeA = 2;					// dc prediction mode
		}
		else
		{
			int absIdxA = *mbAddrA * 16 + luma4x4BlkIdxA;
			intraMxMPredModeA = Intra4x4PredMode[absIdxA];
		}
		if (mb_type_array[*mbAddrB] != I_4x4)		// if the macroblock of the neighbouring
		{											// submacroblock is not using 4x4 prediction
			intraMxMPredModeB = 2;					// dc prediction mode
		}
		else
		{
			int absIdxB = *mbAddrB * 16 + luma4x4BlkIdxB;
			intraMxMPredModeB = Intra4x4PredMode[absIdxB];
		}
	}
	
	// the intra4x4 prediction mode for the current block is the more probable
	// prediction mode of the neighbouring two blocks; the more probable mode
	// has the smaller value
	int predIntra4x4PredMode = (intraMxMPredModeA <= intraMxMPredModeB) ? intraMxMPredModeA : intraMxMPredModeB;
	if (prev_intra4x4_pred_mode_flag[luma4x4BlkIdx] == true)
	{
		Intra4x4PredMode[absIdx] = predIntra4x4PredMode;
	}
	else
	{
		if (rem_intra4x4_pred_mode[luma4x4BlkIdx] < predIntra4x4PredMode)
			Intra4x4PredMode[absIdx] = rem_intra4x4_pred_mode[luma4x4BlkIdx];
		else
			Intra4x4PredMode[absIdx] = rem_intra4x4_pred_mode[luma4x4BlkIdx] + 1;
	}
}

#define p(x,y) (((x) == -1) ? p[(y) + 1] : p[(x) + 5])

// (8.3.1.2.1)
void Intra_4x4_Vertical(int *p, int pred4x4L[4][4])
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			pred4x4L[y][x] = p(x,-1);
		}
	}
}

// (8.3.1.2.2)
void Intra_4x4_Horizontal(int *p, int pred4x4L[4][4])
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			pred4x4L[y][x] = p(-1,y);
		}
	}
}

// (8.3.1.2.3)
void Intra_4x4_DC(int *p, int pred4x4L[4][4])
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
				pred4x4L[y][x] = (p(0,-1) + p(1,-1) + p(2,-1) + p(3,-1) +
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
				pred4x4L[y][x] = (p(-1,0) + p(-1,1) + p(-1,2) + p(-1,3) + 2) >> 2;
			}
		}
	}
	else if (topAvailable)
	{
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				pred4x4L[y][x] = (p(0,-1) + p(1,-1) + p(2,-1) + p(3,-1) + 2) >> 2;
			}
		}
	}
	else
	{
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				pred4x4L[y][x] = 128; // = (1 << (BitDepthY - 1); BitDepthY = 8 in baseline
			}
		}
	}
}

// (8.3.1.2.4)
void Intra_4x4_Diagonal_Down_Left(int *p, int pred4x4L[4][4])
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			if ((x == 3) && (y == 3))
				pred4x4L[y][x] = (p(6,-1) + 3*p(7,-1) + 2) >> 2;
			else
				pred4x4L[y][x] = (p(x+y,-1) + 2*p(x+y+1,-1) + p(x+y+2,-1) + 2) >> 2;
		}
	}
}

// (8.3.1.2.5)
void Intra_4x4_Diagonal_Down_Right(int *p, int pred4x4L[4][4])
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			if (x > y)
				pred4x4L[y][x] = (p(x-y-2,-1) + 2*p(x-y-1,-1) + p(x-y,-1) + 2) >> 2;
			else if (x < y)
				pred4x4L[y][x] = (p(-1,y-x-2) + 2*p(-1,y-x-1) + p(-1,y-x) + 2) >> 2;
			else
				pred4x4L[y][x] = (p(0,-1) + 2*p(-1,-1) + p(-1,0) + 2) >> 2;
		}
	}
}

// (8.3.1.2.6)
void Intra_4x4_Vertical_Right(int *p, int pred4x4L[4][4])
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			int zVR = 2 * x - y;
			if ((zVR == 0) || (zVR == 2) ||
				(zVR == 4) || (zVR == 6))
				pred4x4L[y][x] = (p(x-(y>>1)-1,-1) + p(x-(y>>1),-1) + 1) >> 1;
			else if ((zVR == 1) ||
				(zVR == 3) || (zVR == 5))
				pred4x4L[y][x] = (p(x-(y>>1)-2,-1) + 2*p(x-(y>>1)-1,-1) + p(x-(y>>1),-1) + 2) >> 2;
			else if (zVR == -1)
				pred4x4L[y][x] = (p(-1,0) + 2*p(-1,-1) + p(0,-1) + 2) >> 2;
			else
				pred4x4L[y][x] = (p(-1,y-1) + 2*p(-1,y-2) + p(-1,y-3) + 2) >> 2;
		}
	}
}

// (8.3.1.2.7)
void Intra_4x4_Horizontal_Down(int *p, int pred4x4L[4][4])
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			int zHD = 2 * y - x;
			if ((zHD == 0) || (zHD == 2) ||
				(zHD == 4) || (zHD == 6))
				pred4x4L[y][x] = (p(-1,y-(x>>1)-1) + p(-1,y-(x>>1)) + 1) >> 1;
			else if ((zHD == 1) ||
				(zHD == 3) || (zHD == 5))
				pred4x4L[y][x] = (p(-1,y-(x>>1)-2) + 2*p(-1,y-(x>>1)-1) + p(-1,y-(x>>1)) + 2) >> 2;
			else if (zHD == -1)
				pred4x4L[y][x] = (p(-1,0) + 2*p(-1,-1) + p(0,-1) + 2) >> 2;
			else
				pred4x4L[y][x] = (p(x-1,-1) + 2*p(x-2,-1) + p(x-3,-1) + 2) >> 2;
		}
	}
}

// (8.3.1.2.8)
void Intra_4x4_Vertical_Left(int *p, int pred4x4L[4][4])
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			if ((y == 0) || (y==2))
				pred4x4L[y][x] = (p(x+(y>>1),-1) + p(x+(y>>1)+1,-1) + 1) >> 1;
			else
				pred4x4L[y][x] = (p(x+(y>>1),-1) + 2*p(x+(y>>1)+1,-1) + p(x+(y>>1)+2,-1) + 2) >> 2;
		}
	}
}

// (8.3.1.2.9)
void Intra_4x4_Horizontal_Up(int *p, int pred4x4L[4][4])
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			int zHU = x + 2 * y;
			if ((zHU == 0) || (zHU == 2) ||
				(zHU == 4))
				pred4x4L[y][x] = (p(-1,y+(x>>1)) + p(-1,y+(x>>1)+1) + 1) >> 1;
			else if ((zHU == 1) || (zHU == 3))
				pred4x4L[y][x] = (p(-1,y+(x>>1)) + 2*p(-1,y+(x>>1)+1) + p(-1,y+(x>>1)+2) + 2) >> 2;
			else if (zHU == 5)
				pred4x4L[y][x] = (p(-1,2) + 3*p(-1,3) + 2) >> 2;
			else
				pred4x4L[y][x] = p(-1,3);
		}
	}
}

// Intra_4x4 sample prediction (8.3.1.2)
void Intra4x4SamplePrediction(int luma4x4BlkIdx, int intra4x4PredMode, int pred4x4L[4][4])
{
	int x0 = Intra4x4ScanOrder[luma4x4BlkIdx][0];
	int y0 = Intra4x4ScanOrder[luma4x4BlkIdx][1];

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
		getNeighbourLocations(xN, yN, &mbAddrN, &xW, &yW, true);

		if ((mbAddrN == -1) ||
			((x > 3) && ((luma4x4BlkIdx == 3) || (luma4x4BlkIdx == 11))))
		{
			// if this is an edge macroblock, replicate
			// the edge sample p[3,-1] into the samples p[4..7,-1]
			if ((i > 8) && (p[8] != -1))
				p[i] = p[8];
			else
				p[i] = -1;	// not available for intra prediction

			int test = 0;
		}
		else
		{
			// the abosulte position of the upper-left sample in macroblock N (6.4.1)
			int xM = InverseRasterScan(mbAddrN, 16, 16, frame.Lwidth, 0);
			int yM = InverseRasterScan(mbAddrN, 16, 16, frame.Lwidth, 1);

			p[i] = frame.L[(yM + yW) * frame.Lwidth + (xM + xW)];
			int test = 0;
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

#undef p

#define p(x,y) (((x) == -1) ? p[(y) + 1] : p[(x) + 17])
// (8.3.3.1)
void Intra_16x16_Vertical(int *p, int predL[16][16])
{
	for (int y = 0; y < 16; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			predL[y][x] = p(x,-1);
		}
	}
}

// (8.3.3.2)
void Intra_16x16_Horizontal(int *p, int predL[16][16])
{
	for (int y = 0; y < 16; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			predL[y][x] = p(-1,y);
		}
	}
}

// (8.3.3.3)
void Intra_16x16_DC(int *p, int predL[16][16])
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

	for (int y = 0; y < 16; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			if (allAvailable)
			{
				predL[y][x] = (sumXi + sumYi + 16) >> 5;
			}
			else if (leftAvailable)
			{
				predL[y][x] = (sumYi + 8) >> 4;
			}
			else if (topAvailable)
			{
				predL[y][x] = (sumXi + 8) >> 4;
			}
			else
			{
				predL[y][x] = 1 << 7;		// == 1 << (BitDepthY-1) (BitDepthY is always equal to 8 in the baseline profile)
			}
		}
	}
}

// (8.3.3.4)
void Intra_16x16_Plane(int *p, int predL[16][16])
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

	for (int y = 0; y < 16; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			predL[y][x] = Clip1Y((a + b*(x-7) + c*(y-7) + 16) >> 5);
		}
	}
}

// (8.3.3)
void Intra16x16SamplePrediction(int predL[16][16])
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
		getNeighbourLocations(x, y, &mbAddrN, &xW, &yW, true);

		if (mbAddrN == -1)
		{
			p(x,y) = -1;	// not available for Intra_16x16 prediction
		}
		else
		{
			// inverse macroblock scanning process (6.4.1)
			int xM = InverseRasterScan(mbAddrN, 16, 16, frame.Lwidth, 0);
			int yM = InverseRasterScan(mbAddrN, 16, 16, frame.Lwidth, 1);

			p(x,y) = frame.L[(yM+yW)*frame.Lwidth + (xM+xW)];
		}
	}

	switch(Intra16x16PredMode)
	{
		case 0:
			Intra_16x16_Vertical(p, predL);
			break;
		case 1:
			Intra_16x16_Horizontal(p, predL);
			break;
		case 2:
			Intra_16x16_DC(p, predL);
			break;
		case 3:
			Intra_16x16_Plane(p, predL);
			break;				
	}
}

#undef p

#define p(x,y) (((x) == -1) ? p[(y) + 1] : p[(x) + 9])
#define pCr(x,y) (((x) == -1) ? pCr[(y) + 1] : pCr[(x) + 9])
#define pCb(x,y) (((x) == -1) ? pCb[(y) + 1] : pCb[(x) + 9])
// (8.3.4.1)
void Intra_Chroma_DC(int *p, int predC[8][8])
{
	// chroma4x4BlkIdx € [0..(1<<ChromaArrayType+1)) - 1]; ChromaArrayType == 1 in baseline
	for (int chroma4x4BlkIdx = 0; chroma4x4BlkIdx < 4; chroma4x4BlkIdx++)
	{
		int x0 = InverseRasterScan(chroma4x4BlkIdx, 4, 4, 8, 0);
		int y0 = InverseRasterScan(chroma4x4BlkIdx, 4, 4, 8, 1);

		int sumXi = 0, sumYi = 0;
		for (int i = 0; i < 4; i++)
		{
			sumXi += p(i+x0,-1);
			sumYi += p(-1,i+y0);
		}

		// check availability of neighbouring samples:
		bool allAvailable = true;
		bool leftAvailable = true;
		bool topAvailable = true;
		for (int i = 0; i < 4; i++)
		{
			if (p(i+x0,-1) == -1)
			{
				allAvailable = false;
				topAvailable = false;
			}
			if (p(-1,i+y0) == -1)
			{
				allAvailable = false;
				leftAvailable = false;
			}
		}

		if ((x0 == 0) && (y0 == 0) ||
			(x0 > 0) && (y0 > 0))
		{
			for (int y = 0; y < 4; y++)
			{
				for (int x = 0; x < 4; x++)
				{
					if (allAvailable)
						predC[x+x0][y+y0] = (sumXi + sumYi + 4) >> 3;
					else if (leftAvailable)
						predC[x+x0][y+y0] = (sumYi + 2) >> 2;
					else if (topAvailable)
						predC[x+x0][y+y0] = (sumXi + 2) >> 2;
					else
						predC[x+x0][y+y0] = 1 << 7;	// == 1 << (BitDepthC-1) (BitDepthC is always equal to 8 in the baseline profile)
				}
			}
		}
		else if ((x0 > 0) && (y0 == 0))
		{
			for (int y = 0; y < 4; y++)
			{
				for (int x = 0; x < 4; x++)
				{
					if (topAvailable)
						predC[y+y0][x+x0] = (sumXi + 2) >> 2;
					else if (leftAvailable)
						predC[y+y0][x+x0] = (sumYi + 2) >> 2;
					else
						predC[y+y0][x+x0] = 1 << 7;	// == 1 << (BitDepthC-1) (BitDepthC is always equal to 8 in the baseline profile)
				}
			}
		}
		else
		{
			for (int y = 0; y < 4; y++)
			{
				for (int x = 0; x < 4; x++)
				{
					if (leftAvailable)
						predC[y+y0][x+x0] = (sumYi + 2) >> 2;
					else if (topAvailable)
						predC[y+y0][x+x0] = (sumXi + 2) >> 2;
					else
						predC[y+y0][x+x0] = 1 << 7;	// == 1 << (BitDepthC-1) (BitDepthC is always equal to 8 in the baseline profile)
				}
			}
		}
	}
}

// (8.3.4.2)
void Intra_Chroma_Horizontal(int *p, int predC[8][8])
{
	for (int y = 0; y < MbHeightC; y++)
	{
		for (int x = 0; x < MbWidthC; x++)
		{
			predC[y][x] = p(-1,y);
		}
	}
}

// (8.3.4.3)
void Intra_Chroma_Vertical(int *p, int predC[8][8])
{
	for (int y = 0; y < MbHeightC; y++)
	{
		for (int x = 0; x < MbWidthC; x++)
		{
			predC[y][x] = p(x,-1);
		}
	}
}

// (8.3.4.4)
void Intra_Chroma_Plane(int *p, int predC[8][8])
{
	int xCF = 0;	// xCF = (ChromaArrayType == 3) ? 4 : 0; ChromaArrayType == 1 when baseline
	int yCF = 0;	// yCF = (ChromaArrayType != 1) ? 4 : 0; ChromaArrayType == 1 when baseline

	int H = 0, V = 0;
	for (int i = 0; i <= 3 + xCF; i++)
		H += (i+1)*(p((4+xCF+i),-1) - p((2+xCF-i),-1));
	for (int i = 0; i <= 3 + yCF; i++)
		V += (i+1)*(p(-1,(4+yCF+i)) - p(-1,(2+yCF-i)));

	int a = 16 * (p(-1, (MbHeightC - 1)) + p((MbWidthC - 1), -1));
	int b = (34 * H + 32) >> 6;	// Standard: ChromaArrayType == 0, so there's no 29*(ChromaArrayType == 3) coefficient
	int c = (34 * V + 32) >> 6;	// Standard: ChromaArrayType == 0, so there's no 29*(ChromaArrayType != 1) coefficient

	for (int y = 0; y < MbHeightC; y++)
	{
		for (int x = 0; x < MbWidthC; x++)
		{
			predC[y][x] = Clip1C((a + b*(x-3-xCF) + c*(y-3-yCF) + 16) >> 5);
		}
	}
}

// (8.3.4)
void IntraChromaSamplePrediction(int predCr[8][8], int predCb[8][8])
{
	int pCr[17], pCb[17];
	for (int i = 0; i < MbWidthC + MbHeightC + 1; i++)
	{
		int x, y;
		if (i < MbHeightC + 1)
		{
			x = -1;
			y = i - 1;
		}
		else
		{
			x = i - (MbHeightC + 1);
			y = -1;
		}

		int xW, yW, mbAddrN;
		getNeighbourLocations(x, y, &mbAddrN, &xW, &yW, false);

		if (mbAddrN == -1)
		{
			pCr(x,y) = -1;	// not available for Intra prediction
			pCb(x,y) = -1;
		}
		else
		{
			// inverse macroblock scanning process (6.4.1)
			int xL = InverseRasterScan(mbAddrN, 16, 16, frame.Lwidth, 0);
			int yL = InverseRasterScan(mbAddrN, 16, 16, frame.Lwidth, 1);

			int xM = (xL >> 4)*MbWidthC;
			int yM = (yL >> 4)*MbHeightC + (yL % 2);

			// Chrominance blue:
			pCb(x,y) = (frame.C[0])[(yM+yW)*frame.Cwidth + (xM+xW)];
			// Chrominance red:
			pCr(x,y) = (frame.C[1])[(yM+yW)*frame.Cwidth + (xM+xW)];
		}
	}

	switch(intra_chroma_pred_mode)
	{
		case 0:
			Intra_Chroma_DC(pCb, predCb);
			Intra_Chroma_DC(pCr, predCr);
			break;
		case 1:
			Intra_Chroma_Horizontal(pCb, predCb);
			Intra_Chroma_Horizontal(pCr, predCr);
			break;
		case 2:
			Intra_Chroma_Vertical(pCb, predCb);
			Intra_Chroma_Vertical(pCr, predCr);
			break;
		case 3:
			Intra_Chroma_Plane(pCb, predCb);
			Intra_Chroma_Plane(pCr, predCr);
			break;
	}		
}

// predL, predCr and predCb are the output prediction samples
// with dimensions 16x16, 8x8 and 8x8 respectively
void intraPrediction(int predL[16][16], int predCr[8][8], int predCb[8][8])
{
	// mb_pred(mb_type) in the standard
	if ((MbPartPredMode(mb_type , 0) == Intra_4x4) || (MbPartPredMode(mb_type , 0) == Intra_16x16))
	{
		// LUMA:

		if (MbPartPredMode(mb_type , 0) == Intra_4x4)
		{
			for (int luma4x4BlkIdx = 0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
			{
				int mbAddrA, mbAddrB;
				getIntra4x4PredMode(luma4x4BlkIdx, &mbAddrA, &mbAddrB);

				// the luma prediction samples
				int pred4x4L[4][4];
				int absIdx = CurrMbAddr * 16 + luma4x4BlkIdx;
				Intra4x4SamplePrediction(luma4x4BlkIdx, Intra4x4PredMode[absIdx], pred4x4L);

				int x0 = Intra4x4ScanOrder[luma4x4BlkIdx][0];
				int y0 = Intra4x4ScanOrder[luma4x4BlkIdx][1];

				for (int y = 0; y < 4; y++)
				{
					for (int x = 0; x < 4; x++)
					{
						predL[y0+y][x0+x] = pred4x4L[y][x];
					}
				}

				transformDecoding4x4LumaResidual(LumaLevel, predL, luma4x4BlkIdx, QPy);
			}

			int test = 0;
		}
		else
		{
			Intra16x16SamplePrediction(predL);
		}

		// CHROMA:
		IntraChromaSamplePrediction(predCr, predCb);
	}	
}