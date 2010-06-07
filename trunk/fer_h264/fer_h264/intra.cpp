#include "intra.h"
#include "inttransform.h"
#include "scaleTransform.h"
#include "quantizationTransform.h"
#include "residual.h"
#include "h264_math.h"
#include "h264_globals.h"
#include "headers_and_parameter_sets.h"
#include "rbsp_decoding.h"
#include "rbsp_encoding.h"
#include "openCL_functions.h"

// TEST:
#include <time.h>

const int intraToChromaPredMode[4] = {2,1,0,3};

const int subMBNeighbours[16][2] = {
{ 5, 10}, { 0, 11}, { 7,  0}, { 2,  1},
{ 1, 14}, { 4, 15}, { 3,  4}, { 6,  5},
{13,  2}, { 8,  3}, {15,  8}, {10,  9},
{ 9,  6}, {12,  7}, {11, 12}, {14, 13}};

// Derivation process for neighbouring 4x4 luma blocks,
// equivalent to (6.4.10.4)
// nA - neighbour (A if true, B if false, see figure 6-12)
void getNeighbourAddresses(int luma4x4BlkIdx, bool nA, int *mbAddrN, int *luma4x4BlkIdxN)
{
	if (nA == true)
	{
		if ((luma4x4BlkIdx == 0) || (luma4x4BlkIdx == 2) ||
			(luma4x4BlkIdx == 8) || (luma4x4BlkIdx == 10))
		{
			if (CurrMbAddr % PicWidthInMbs == 0)
			{
				*mbAddrN = -1;
				*luma4x4BlkIdxN = -1;
			}
			else
			{
				*mbAddrN = CurrMbAddr - 1;
				*luma4x4BlkIdxN = subMBNeighbours[luma4x4BlkIdx][0];
			}
		}		
		else
		{
			*mbAddrN = CurrMbAddr;
			*luma4x4BlkIdxN = subMBNeighbours[luma4x4BlkIdx][0];
		}
	}
	else
	{
		if ((luma4x4BlkIdx == 0) || (luma4x4BlkIdx == 1) ||
			(luma4x4BlkIdx == 4) || (luma4x4BlkIdx == 5))
		{
			if (CurrMbAddr < PicWidthInMbs)
			{
				*mbAddrN = -1;
				*luma4x4BlkIdxN = -1;
			}
			else
			{
				*mbAddrN = CurrMbAddr - PicWidthInMbs;
				*luma4x4BlkIdxN = subMBNeighbours[luma4x4BlkIdx][1];
			}
		}		
		else
		{
			*mbAddrN = CurrMbAddr;
			*luma4x4BlkIdxN = subMBNeighbours[luma4x4BlkIdx][1];
		}
	}
}


// Derivation process for Intra4x4PredMode (8.3.1.1)
void getIntra4x4PredMode(int luma4x4BlkIdx)
{
	int luma4x4BlkIdxA, luma4x4BlkIdxB;
	int mbAddrA, mbAddrB;
	getNeighbourAddresses(luma4x4BlkIdx, true, &mbAddrA, &luma4x4BlkIdxA);
	getNeighbourAddresses(luma4x4BlkIdx, false, &mbAddrB, &luma4x4BlkIdxB);

	// no checking whether the neighbouring
	// macroblocks are coded as P-macroblocks
	bool dcPredModePredictedFlag = false;
	if ((mbAddrA == -1) || (mbAddrB == -1) ||
		(((mbAddrA != -1) || (mbAddrB != -1)) && (pps.constrained_intra_pred_flag == 1)))
		dcPredModePredictedFlag = true;

	int absIdx = (CurrMbAddr << 4) + luma4x4BlkIdx;
	int intraMxMPredModeA, intraMxMPredModeB;
	if (dcPredModePredictedFlag == true)	// if dc mode is predicted
	{
		intraMxMPredModeA = 2;
		intraMxMPredModeB = 2;
	}
	else
	{
		if (MbPartPredMode(mb_type_array[mbAddrA],0) != Intra_4x4)		// if the macroblock of the neighbouring
		{											// submacroblock is not using 4x4 prediction
			intraMxMPredModeA = 2;					// dc prediction mode
		}
		else
		{
			int absIdxA = (mbAddrA << 4) + luma4x4BlkIdxA;
			intraMxMPredModeA = Intra4x4PredMode[absIdxA];
		}
		if (MbPartPredMode(mb_type_array[mbAddrB],0) != Intra_4x4)		// if the macroblock of the neighbouring
		{											// submacroblock is not using 4x4 prediction
			intraMxMPredModeB = 2;					// dc prediction mode
		}
		else
		{
			int absIdxB = (mbAddrB << 4) + luma4x4BlkIdxB;
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
	int result = 128;
	if (p(-1,-1) != -1)		// if all available
		result = (p(0,-1) + p(1,-1) + p(2,-1) + p(3,-1) +
				  p(-1,0) + p(-1,1) + p(-1,2) + p(-1,3) + 4) >> 3;
	else if (p(-1,0) != -1)	// if left available
		result = (p(-1,0) + p(-1,1) + p(-1,2) + p(-1,3) + 2) >> 2;
	else if (p(0,-1) != -1) // if top available
		result = (p(0,-1) + p(1,-1) + p(2,-1) + p(3,-1) + 2) >> 2;
				  
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			pred4x4L[y][x] = result;
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
			pred4x4L[y][x] = (p(x+y,-1) + (p(x+y+1,-1) << 1) + p(x+y+2,-1) + 2) >> 2;
		}
	}	
	pred4x4L[3][3] = (p(6,-1) + 3*p(7,-1) + 2) >> 2;
}

// (8.3.1.2.5)
void Intra_4x4_Diagonal_Down_Right(int *p, int pred4x4L[4][4])
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			if (x > y)
				pred4x4L[y][x] = (p(x-y-2,-1) + (p(x-y-1,-1) << 1) + p(x-y,-1) + 2) >> 2;
			else if (x < y)
				pred4x4L[y][x] = (p(-1,y-x-2) + (p(-1,y-x-1) << 1) + p(-1,y-x) + 2) >> 2;
			else
				pred4x4L[y][x] = (p(0,-1) + (p(-1,-1) << 1) + p(-1,0) + 2) >> 2;
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
			int zVR = (x << 1) - y;
			if ((zVR == 0) || (zVR == 2) ||
				(zVR == 4) || (zVR == 6))
				pred4x4L[y][x] = (p(x-(y>>1)-1,-1) + p(x-(y>>1),-1) + 1) >> 1;
			else if ((zVR == 1) ||
				(zVR == 3) || (zVR == 5))
				pred4x4L[y][x] = (p(x-(y>>1)-2,-1) + (p(x-(y>>1)-1,-1) << 1) + p(x-(y>>1),-1) + 2) >> 2;
			else if (zVR == -1)
				pred4x4L[y][x] = (p(-1,0) + (p(-1,-1) << 1) + p(0,-1) + 2) >> 2;
			else
				pred4x4L[y][x] = (p(-1,y-1) + (p(-1,y-2) << 1) + p(-1,y-3) + 2) >> 2;
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
			int zHD = (y << 1) - x;
			if ((zHD == 0) || (zHD == 2) ||
				(zHD == 4) || (zHD == 6))
				pred4x4L[y][x] = (p(-1,y-(x>>1)-1) + p(-1,y-(x>>1)) + 1) >> 1;
			else if ((zHD == 1) ||
				(zHD == 3) || (zHD == 5))
				pred4x4L[y][x] = (p(-1,y-(x>>1)-2) + (p(-1,y-(x>>1)-1) << 1) + p(-1,y-(x>>1)) + 2) >> 2;
			else if (zHD == -1)
				pred4x4L[y][x] = (p(-1,0) + (p(-1,-1) << 1) + p(0,-1) + 2) >> 2;
			else
				pred4x4L[y][x] = (p(x-1,-1) + (p(x-2,-1) << 1) + p(x-3,-1) + 2) >> 2;
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
				pred4x4L[y][x] = (p(x+(y>>1),-1) + (p(x+(y>>1)+1,-1) << 1) + p(x+(y>>1)+2,-1) + 2) >> 2;
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
			int zHU = x + (y << 1);
			if ((zHU == 0) || (zHU == 2) ||
				(zHU == 4))
				pred4x4L[y][x] = (p(-1,y+(x>>1)) + p(-1,y+(x>>1)+1) + 1) >> 1;
			else if ((zHU == 1) || (zHU == 3))
				pred4x4L[y][x] = (p(-1,y+(x>>1)) + (p(-1,y+(x>>1)+1) << 1) + p(-1,y+(x>>1)+2) + 2) >> 2;
			else if (zHU == 5)
				pred4x4L[y][x] = (p(-1,2) + 3*p(-1,3) + 2) >> 2;
			else
				pred4x4L[y][x] = p(-1,3);
		}
	}
}

void FetchPredictionSamplesIntra4x4(int luma4x4BlkIdx, int p[13])
{
	int xP = ((CurrMbAddr%PicWidthInMbs)<<4);
	int yP = ((CurrMbAddr/PicWidthInMbs)<<4);
	
	int x0 = Intra4x4ScanOrder[luma4x4BlkIdx][0];
	int y0 = Intra4x4ScanOrder[luma4x4BlkIdx][1];
	
	int x = xP + x0;
	int y = yP + y0;
	
	int xF = x - 1;
	int yF = y - 1;
	int frameIdx = yF * frame.Lwidth + xF;
	if ((xF < 0) || (yF < 0))
	{
		p[0] = -1;
	}
	else
	{
		p[0] = frame.L[frameIdx];
	}
	
	xF = x - 1;
	yF = y;	
	if (xF < 0)
	{
		for (int i = 1; i < 5; i++)
		{
			p[i] = -1;		// Unavailable for prediction
		}
	}
	else
	{
		for (int i = 1; i < 5; i++)
		{
			frameIdx = yF * frame.Lwidth + xF;
			p[i] = frame.L[frameIdx];
			yF++;
		}
	}
	
	xF = x;
	yF = y - 1;
	if (yF < 0)
	{
		for (int i = 5; i < 13; i++)
		{
			// Samples above and above-right marked as unavailable for prediction
			p[i] = -1;
		}
	}
	else
	{
		for (int i = 5; i < 9; i++)
		{
			frameIdx = yF * frame.Lwidth + xF;
			p[i] = frame.L[frameIdx];
			xF++;
		}
		
		xF = x + 4;
		bool edgeSubMB = (xF >= frame.Lwidth) || ((x0 == 12) && (y0 > 0));
		if ((edgeSubMB == true) || (luma4x4BlkIdx == 3) || (luma4x4BlkIdx == 11))
		{
			xF = x + 3;
			frameIdx = yF * frame.Lwidth + xF;
			for (int i = 9; i < 13; i++)
			{
				// Copy the rightmost prediction sample to
				// the samples above-right
				p[i] = frame.L[frameIdx];
			}
		}
		else
		{
			for (int i = 9; i < 13; i++)
			{
				frameIdx = yF * frame.Lwidth + xF;
				p[i] = frame.L[frameIdx];
				xF++;
			}
		}
	}
}

void performIntra4x4Prediction(int luma4x4BlkIdx, int intra4x4PredMode, int pred4x4L[4][4], int p[13])
{
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
	}
}

// Intra_4x4 sample prediction (8.3.1.2)
void Intra4x4SamplePrediction(int luma4x4BlkIdx, int intra4x4PredMode, int pred4x4L[4][4])
{
	int p[13];
	FetchPredictionSamplesIntra4x4(luma4x4BlkIdx, p);
	performIntra4x4Prediction(luma4x4BlkIdx,intra4x4PredMode,pred4x4L,p);	
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

	int result = 128;
	if (p[0] != -1)			// if all available
		result = (sumXi + sumYi + 16) >> 5;
	else if (p[1] != -1) 	// if left available
		result = (sumYi + 8) >> 4;
	else if (p[17] != -1)	// if top available
		result = (sumXi + 8) >> 4;
	
	for (int y = 0; y < 16; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			predL[y][x] = result;
		}
	}
}

// (8.3.3.4)
void Intra_16x16_Plane(int *p, int predL[16][16])
{
	int H = 0, V = 0;
	for (int i = 0; i <= 7; i++)
	{
		H += (i+1)*(p(8+i,-1) - p(6-i,-1));
		V += (i+1)*(p(-1,8+i) - p(-1,6-i));
	}

	int a = ((p(-1,15) + p(15,-1)) << 4);
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

void FetchPredictionSamplesIntra16x16(int p[33])
{
	
	int xF, yF, frameIdx;
	
	int xP = ((CurrMbAddr%PicWidthInMbs)<<4);
	int yP = ((CurrMbAddr/PicWidthInMbs)<<4);

	// p[-1,-1]:
	xF = xP - 1;
	yF = yP - 1;
	frameIdx = yF*frame.Lwidth + xF;
	p[0] = ((xF >= 0) && (yF >= 0)) ? frame.L[frameIdx] : -1;

	// p[-1,0]..p[-1,15]:
	xF = xP - 1;
	yF = yP;
	frameIdx = yF*frame.Lwidth + xF;
	for (int i = 1; i < 17; i++)
	{
		p[i] = (xF >= 0) ? frame.L[frameIdx] : -1;
		frameIdx += frame.Lwidth;
	}

	// p[0,-1]..p[15,-1]:
	xF = xP;
	yF = yP - 1;
	frameIdx = yF*frame.Lwidth + xF;
	for (int i = 17; i < 33; i++)
	{
		p[i] = (yF >= 0) ? frame.L[frameIdx] : -1;
		frameIdx++;
	}
}

void inline performIntra16x16Prediction(int *p, int predL[16][16], int Intra16x16PredMode)
{
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

// (8.3.3)
void Intra16x16SamplePrediction(int predL[16][16], int Intra16x16PredMode)
{
	int p[33];
	FetchPredictionSamplesIntra16x16(p);
	performIntra16x16Prediction(p,predL,Intra16x16PredMode);
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
		int x0 = (chroma4x4BlkIdx & 1) << 2;
		int y0 = (chroma4x4BlkIdx >> 1) << 2;

		int sumXi = 0, sumYi = 0;
		for (int i = 0; i < 4; i++)
		{
			sumXi += p(i+x0,-1);
			sumYi += p(-1,i+y0);
		}

		// check availability of neighbouring samples:
		bool leftAvailable = p(-1, y0) != -1;
		bool topAvailable = p(x0, -1) != -1;
		bool allAvailable = topAvailable && leftAvailable;

		int result = 128; // == 1 << (BitDepthC-1) (BitDepthC is always equal to 8 in the baseline profile)
		if ((x0 == 0) && (y0 == 0) ||
			(x0 > 0) && (y0 > 0))
		{
			if (allAvailable)
				result = (sumXi + sumYi + 4) >> 3;
			else if (leftAvailable)
				result = (sumYi + 2) >> 2;
			else if (topAvailable)
				result = (sumXi + 2) >> 2;

			for (int y = 0; y < 4; y++)
			{
				for (int x = 0; x < 4; x++)
				{
					predC[x+x0][y+y0] = result;
				}
			}
		}
		else if ((x0 > 0) && (y0 == 0))
		{
			if (topAvailable)
				result = (sumXi + 2) >> 2;
			else if (leftAvailable)
				result = (sumYi + 2) >> 2;

			for (int y = 0; y < 4; y++)
			{
				for (int x = 0; x < 4; x++)
				{
					predC[y+y0][x+x0] = result;
				}
			}
		}
		else
		{
			if (leftAvailable)
				result = (sumYi + 2) >> 2;
			else if (topAvailable)
				result = (sumXi + 2) >> 2;
			for (int y = 0; y < 4; y++)
			{
				for (int x = 0; x < 4; x++)
				{
					predC[y+y0][x+x0] = result;
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
	// xCF, yCF == 0, because
	// xCF = (ChromaArrayType == 3) ? 4 : 0; ChromaArrayType == 1 when baseline
	// yCF = (ChromaArrayType != 1) ? 4 : 0; ChromaArrayType == 1 when baseline

	int H = 0, V = 0;
	for (int i = 0; i <= 3; i++)
		H += (i+1)*(p((4+i),-1) - p((2-i),-1));
	for (int i = 0; i <= 3; i++)
		V += (i+1)*(p(-1,(4+i)) - p(-1,(2-i)));

	int a = ((p(-1, (MbHeightC - 1)) + p((MbWidthC - 1), -1)) << 4);
	int b = (34 * H + 32) >> 6;	// Norm: ChromaArrayType == 0, so there's no 29*(ChromaArrayType == 3) coefficient
	int c = (34 * V + 32) >> 6;	// Norm: ChromaArrayType == 0, so there's no 29*(ChromaArrayType != 1) coefficient

	for (int y = 0; y < MbHeightC; y++)
	{
		for (int x = 0; x < MbWidthC; x++)
		{
			predC[y][x] = Clip1C((a + b*(x-3) + c*(y-3) + 16) >> 5);
		}
	}
}

// (8.3.4)
void IntraChromaSamplePrediction(int predCr[8][8], int predCb[8][8])
{
	int pCr[17], pCb[17];

	int xM = ((CurrMbAddr%PicWidthInMbs)<<3);
	int yM = ((CurrMbAddr/PicWidthInMbs)<<3);

	// p[-1, -1]:
	int xF = xM - 1;
	int yF = yM - 1;
	if ((xF < 0) || (yF < 0))
	{
		pCb[0] = -1;
		pCr[0] = -1;
	}
	else
	{
		pCb[0] = frame.C[0][yF*frame.Cwidth + xF];
		pCr[0] = frame.C[1][yF*frame.Cwidth + xF];
	}

	// p[-1,0]..p[-1,7]:
	xF = xM - 1;
	yF = yM;
	for (int i = 1; i < 9; i++)
	{
		if (xF < 0)
		{
			pCb[i] = -1;
			pCr[i] = -1;
		}
		else
		{
			pCb[i] = frame.C[0][yF*frame.Cwidth + xF];
			pCr[i] = frame.C[1][yF*frame.Cwidth + xF];
			yF++;
		}
	}

	// p[0,-1]..p[7,-1]:
	xF = xM;
	yF = yM - 1;
	for (int i = 9; i < 17; i++)
	{
		if (yF < 0)
		{
			pCb[i] = -1;
			pCr[i] = -1;
		}
		else
		{
			pCb[i] = frame.C[0][yF*frame.Cwidth + xF];
			pCr[i] = frame.C[1][yF*frame.Cwidth + xF];
			xF++;
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
	if (MbPartPredMode(mb_type , 0) == Intra_4x4)
	{
		for (int luma4x4BlkIdx = 0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
		{
			getIntra4x4PredMode(luma4x4BlkIdx);

			// the luma prediction samples
			int pred4x4L[4][4];
			int absIdx = (CurrMbAddr << 4) + luma4x4BlkIdx;
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
	}
	else
	{
		int Intra16x16PredMode;
		if ((shd.slice_type%5)==P_SLICE)
		{
			Intra16x16PredMode = I_Macroblock_Modes[mb_type-5][4];
		}
		else
		{
			Intra16x16PredMode = I_Macroblock_Modes[mb_type][4];
		}
		Intra16x16SamplePrediction(predL, Intra16x16PredMode);
	}

	// CHROMA:
	IntraChromaSamplePrediction(predCr, predCb);
}

// ENCODING:

// Çalculates the sum of absolute transformed differences
// for a predicted 4x4 luma submacroblock.
int satdLuma4x4(int pred4x4L[4][4], int luma4x4BlkIdx)
{
	int satd = 0;

	int xP = ((CurrMbAddr%PicWidthInMbs)<<4);
	int yP = ((CurrMbAddr/PicWidthInMbs)<<4);

	int x0 = Intra4x4ScanOrder[luma4x4BlkIdx][0];
	int y0 = Intra4x4ScanOrder[luma4x4BlkIdx][1];

	int diffL4x4[4][4];
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			diffL4x4[i][j] = frame.L[(yP+y0+i)*frame.Lwidth+(xP+x0+j)] - pred4x4L[i][j];
		}
	}

	int rLuma[4][4];
	forwardResidual(QPy, diffL4x4, rLuma, true, false);

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			satd += ABS(rLuma[i][j]);
		}
	}

	return satd;
}

int satdLuma16x16(int predL[16][16])
{
	int satd = 0;

	for (int luma4x4BlkIdx = 0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
	{
		int x0 = Intra4x4ScanOrder[luma4x4BlkIdx][0];
		int y0 = Intra4x4ScanOrder[luma4x4BlkIdx][1];

		int pred4x4L[4][4];
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				pred4x4L[y][x] = predL[y0+y][x0+x];
			}
		}

		satd += satdLuma4x4(pred4x4L, luma4x4BlkIdx);		
	}

	return satd;
}

// Sets the global variables prev_intra4x4_pred_mode_flag
// and rem_intra4x4_pred_mode.
void setIntra4x4PredMode(int luma4x4BlkIdx)
{
	int luma4x4BlkIdxA, luma4x4BlkIdxB;
	int mbAddrA, mbAddrB;
	getNeighbourAddresses(luma4x4BlkIdx, true, &mbAddrA, &luma4x4BlkIdxA);
	getNeighbourAddresses(luma4x4BlkIdx, false, &mbAddrB, &luma4x4BlkIdxB);

	// no checking whether the neighbouring
	// macroblocks are coded as P-macroblocks
	bool dcPredModePredictedFlag = false;
	if ((mbAddrA == -1) || (mbAddrB == -1) ||
		(((mbAddrA != -1) || (mbAddrB != -1)) && (pps.constrained_intra_pred_flag == 1)))
		dcPredModePredictedFlag = true;

	int absIdx = (CurrMbAddr << 4) + luma4x4BlkIdx;
	int intraMxMPredModeA, intraMxMPredModeB;
	if (dcPredModePredictedFlag == true)	// if dc mode is predicted
	{
		intraMxMPredModeA = 2;
		intraMxMPredModeB = 2;
	}
	else
	{
		if (MbPartPredMode(mb_type_array[mbAddrA],0) != Intra_4x4)		// if the macroblock of the neighbouring
		{											// submacroblock is not using 4x4 prediction
			intraMxMPredModeA = 2;					// dc prediction mode
		}
		else
		{
			int absIdxA = (mbAddrA << 4) + luma4x4BlkIdxA;
			intraMxMPredModeA = Intra4x4PredMode[absIdxA];
		}
		if (MbPartPredMode(mb_type_array[mbAddrB],0) != Intra_4x4)		// if the macroblock of the neighbouring
		{											// submacroblock is not using 4x4 prediction
			intraMxMPredModeB = 2;					// dc prediction mode
		}
		else
		{
			int absIdxB = (mbAddrB << 4) + luma4x4BlkIdxB;
			intraMxMPredModeB = Intra4x4PredMode[absIdxB];
		}
	}
	
	// the intra4x4 prediction mode for the current block is the more probable
	// prediction mode of the neighbouring two blocks; the more probable mode
	// has the smaller value
	int predIntra4x4PredMode = (intraMxMPredModeA <= intraMxMPredModeB) ? intraMxMPredModeA : intraMxMPredModeB;
	if (Intra4x4PredMode[absIdx] == predIntra4x4PredMode)
	{
		prev_intra4x4_pred_mode_flag[luma4x4BlkIdx] = true;
	}
	else
	{
		prev_intra4x4_pred_mode_flag[luma4x4BlkIdx] = false;

		if (Intra4x4PredMode[absIdx] < predIntra4x4PredMode)
		{
			rem_intra4x4_pred_mode[luma4x4BlkIdx] = Intra4x4PredMode[absIdx];
		}
		else
		{
			rem_intra4x4_pred_mode[luma4x4BlkIdx] = Intra4x4PredMode[absIdx] - 1;
		}
	}
}

// The function returns 0-3 for intra16x16 prediction, the
// values correspond to the intra 16x16 prediction mode.
// If Intra4x4 is chosen, -1 is returned and the prediction
// modes are assigned to the global array Intra4x4PredMode.
// The chroma prediction mode is assigned to intra_chroma_pred_mode.
int intraPredictionEncoding(int predL[16][16], int predCr[8][8], int predCb[8][8])
{
	int Intra16x16PredMode;
	int pred4x4L[4][4];
	int bitLoad;
	int chosenChromaPrediction;

	int min = INT_MAX;

	int p[33];
	FetchPredictionSamplesIntra16x16(p);

	if (OpenCLEnabled == true)
	{
		if (CurrMbAddr == 0)
		{
			WaitIntraCL(true);
		}

		Intra16x16PredMode = predModes16x16[CurrMbAddr];
		performIntra16x16Prediction(p, predL, Intra16x16PredMode);

		// Choose the same prediction mode for chroma:
		intra_chroma_pred_mode = intraToChromaPredMode[Intra16x16PredMode];
		IntraChromaSamplePrediction(predCr, predCb);

		min = coded_mb_size(Intra16x16PredMode, predL, predCb, predCr);
		chosenChromaPrediction = intra_chroma_pred_mode;
	}
	else
	{
		// 16x16 prediction:
		int min16x16 = INT_MAX;
		for (int i = 0; i < 4; i++)
		{
			// Skip prediction if required neighbouring macroblocks are not available
			if (((i == 0) && (p[17] == -1)) ||
				((i == 1) && (p[1] == -1)) ||
				((i == 3) && (p[0] == -1)))
			{
				continue;
			}
			performIntra16x16Prediction(p, predL, i);

			// Choose the same prediction mode for chroma:
			intra_chroma_pred_mode = intraToChromaPredMode[i];
			IntraChromaSamplePrediction(predCr, predCb);

			int satd = satdLuma16x16(predL);
			if (satd < min16x16)
			{
				min16x16 = satd;
				Intra16x16PredMode = i;
				chosenChromaPrediction = intra_chroma_pred_mode;
			}
		}		
		performIntra16x16Prediction(p, predL, Intra16x16PredMode);		

		// Store the chosen chroma prediction:
		intra_chroma_pred_mode = chosenChromaPrediction;
		IntraChromaSamplePrediction(predCr, predCb);
		
		min = coded_mb_size(Intra16x16PredMode, predL, predCb, predCr);


		// 4x4 prediction:
		mb_type_array[CurrMbAddr] = 0;
		for (int luma4x4BlkIdx = 0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
		{
			int min4x4 = INT_MAX;

			int p[13];
			FetchPredictionSamplesIntra4x4(luma4x4BlkIdx, p);
			for(int predMode = 0; predMode < 9; predMode++)
			{
				// Skip prediction if required neighbouring submacroblocks are not available
				if (((predMode == 0) && (p[5] == -1)) ||
					((predMode == 1) && (p[1] == -1)) ||
					((predMode == 3) && (p[5] == -1)) ||
					((predMode == 4) && (p[0] == -1)) ||
					((predMode == 5) && (p[0] == -1)) ||
					((predMode == 6) && (p[0] == -1)) ||
					((predMode == 7) && (p[5] == -1)) ||
					((predMode == 8) && (p[1] == -1)))
				{
					continue;
				}
				
				performIntra4x4Prediction(luma4x4BlkIdx, predMode, pred4x4L, p);

				int satd4x4 = satdLuma4x4(pred4x4L, luma4x4BlkIdx);
				if (satd4x4 < min4x4)
				{
					int absIdx = (CurrMbAddr << 4) + luma4x4BlkIdx;
					Intra4x4PredMode[absIdx] = predMode;
					min4x4 = satd4x4;
					if (min4x4 == 0)
					{
						break;
					}					
				}
			}
		}
	}

	// set the best intra4x4 pred modes:
	int xP = ((CurrMbAddr%PicWidthInMbs)<<4);
	int yP = ((CurrMbAddr/PicWidthInMbs)<<4);

	int originalMB[16][16];
	mb_type_array[CurrMbAddr] = 0;
	if (OpenCLEnabled == true && CurrMbAddr == 0)
	{
		WaitIntraCL(false);
	}
	for (int luma4x4BlkIdx = 0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
	{
		setIntra4x4PredMode(luma4x4BlkIdx);
		
		int absIdx = (CurrMbAddr << 4) + luma4x4BlkIdx;				
		Intra4x4SamplePrediction(luma4x4BlkIdx, Intra4x4PredMode[absIdx], pred4x4L);

		int x0 = Intra4x4ScanOrder[luma4x4BlkIdx][0];
		int y0 = Intra4x4ScanOrder[luma4x4BlkIdx][1];

		int diffL4x4[4][4], rLuma[4][4];
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				predL[y0+y][x0+x] = pred4x4L[y][x];
				originalMB[y0+y][x0+x] = frame.L[(yP + y0 + y)*frame.Lwidth+(xP + x0 + x)];
				diffL4x4[y][x] = frame.L[(yP + y0 + y)*frame.Lwidth+(xP + x0 + x)] - pred4x4L[y][x];
			}
		}

		forwardResidual(QPy, diffL4x4, rLuma, true, false);
		transformScan(rLuma, LumaLevel[luma4x4BlkIdx], false);
		transformDecoding4x4LumaResidual(LumaLevel, predL, luma4x4BlkIdx, QPy);
	}

	bitLoad = coded_mb_size(-1, predL, predCb, predCr);
	if (bitLoad < min)
	{
		// Choose 4x4 prediction mode, the prediction samples
		// and other variables are already set for Intra4x4
		Intra16x16PredMode = -1;	
	}
	else
	{
		// Restore the original frame samples
		for (int i = 0; i < 16; i++)
		{
			for (int j = 0; j < 16; j++)
			{
				frame.L[(yP+i)*frame.Lwidth+(xP+j)] = originalMB[i][j];
			}
		}
		// Reset the best intra16x16 prediction
		Intra16x16SamplePrediction(predL, Intra16x16PredMode);
	}

	return Intra16x16PredMode;
}