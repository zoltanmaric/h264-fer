#include "intra.h"

typedef struct
{
	int mbAddrA, mbAddrB;
	int luma4x4BlkIdxA, luma4x4BlkIdxB;
	int xWA, yWA, xWB, yWB;
} neighbour_info;

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

int InverseRasterScan(int a, int b, int c, int d, int e)
{
	if (e == 0)
		return (a % (d/b)) * b;
	else
		return (a / (d/b)) * c;
}

// nA - neighbour (A if true, B if false, see figure 6-12)
void getNeighbourAddresses(int CurrMbAddr, int luma4x4BlkIdx, bool nA,
							int mbWidth, int *mbAddrN, int *luma4x4BlkIdxN)
{
	int xD, yD;
	// Neighbour A:
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

	// width and height of the macroblock
	int maxW = 16;
	int maxH = 16;	
	
	// if xN and yN are within this macroblock
	if ((xN >= 0) && (xN < maxW) &&
		(yN >= 0) && (yN < maxH))
		*mbAddrN = CurrMbAddr;
	// for macroblock A,
	// if the current macroblock is not on
	// the left edge of the frame
	else if ((nA == true) && ((CurrMbAddr % mbWidth) != 0))
		*mbAddrN = CurrMbAddr - 1;
	// for macroblock B,
	// if the current macroblock is not on
	// the top edge of the frame
	else if ((nA == false) && (CurrMbAddr >= mbWidth))
		*mbAddrN = CurrMbAddr - mbWidth;
	else
		*mbAddrN = -1;	// not available

	// position of the edge sample in the adjacent 4x4 block
	// relative to the top left sample in macroblock A
	int xW = (xN + maxW) % maxW;
	int yW = (yN + maxH) % maxH;

	if (*mbAddrN != -1)
		*luma4x4BlkIdxN = 8 * (yW / 8) + 4 * (xW / 8) + 2 * ((yW % 8) / 4) + ((xW % 8) / 4);
	else
		*luma4x4BlkIdxN = -1;
}

void mb_pred(int luma4x4BlkIdx, mode_pred_info &mpi, int CurrMbAddr, neighbour_info &ni)
{
	bool prev_intra4x4_pred_mode_flag = get_prev_intra4x4_pred_mode_flag(luma4x4BlkIdx);
	int rem_intra4x4_pred_mode;
	if (prev_intra4x4_pred_mode_flag == false)
		rem_intra4x4_pred_mode = get_rem_intra4x4_pred_mode(luma4x4BlkIdx);

	int luma4x4BlkIdxA, luma4x4BlkIdxB;
	getNeighbourAddresses(CurrMbAddr, luma4x4BlkIdx, true, mpi.MbWidth, &ni.mbAddrA, &ni.luma4x4BlkIdxA);
	getNeighbourAddresses(CurrMbAddr, luma4x4BlkIdx, false, mpi.MbWidth, &ni.mbAddrB, &ni.luma4x4BlkIdxB);

	// no checking whether the neighbouring
	// macroblocks are coded as P-macroblocks
	bool dcPredModePredictedFlag = false;
	if ((ni.mbAddrA == -1) || (ni.mbAddrB == -1))
		dcPredModePredictedFlag = true;

	int absIdx = CurrMbAddr * 16 + luma4x4BlkIdx;
	if ((dcPredModePredictedFlag == true) ||
		(mpi.MbMode[ni.mbAddrA] != I_NxN) ||
		(mpi.MbMode[ni.mbAddrB] != I_NxN))
	{
		mpi.Intra4x4PredMode[absIdx] = 2;		// dc prediction mode
	}
	else
	{
		int absIdxA = ni.mbAddrA * 16 + ni.luma4x4BlkIdxA;
		int absIdxB = ni.mbAddrB * 16 + ni.luma4x4BlkIdxB;
		int intraMxMPredModeA = mpi.Intra4x4PredMode[absIdxA];
		int intraMxMPredModeB = mpi.Intra4x4PredMode[absIdxB];
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

void intraPrediction(frame &f, mode_pred_info &mpi, mb_mode mb, int CurrMbAddr)
{

#define p(x,y) p[x+1][y+1];

	bool prev_intra4x4_pred_mode_flag[16];
	int rem_intra4x4_pred_mode[16];
	
	// mb_pred(mb_type) in the standard
	if ((mb.MbPartPredMode[0] == Intra_4x4) ||
		(mb.MbPartPredMode[0] == Intra_16x16))
	{
		if (mb.MbPartPredMode[0] == Intra_4x4)
		{
			for (int luma4x4BlkIdx = 0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
			{
				int mbAddrA, mbAddrB;
				neighbour_info ni;
				mb_pred(luma4x4BlkIdx, mpi, CurrMbAddr, ni);

				int x0 = Intra4x4Scan[luma4x4BlkIdx][0];
				int y0 = Intra4x4Scan[luma4x4BlkIdx][1];

				int x, y, xA, yA, xB, yB;
				for (int i = 0; i < 13; i++)
				{
					// left samples:
					if (i < 4)
					{
						x = -1;
						y = i - 1;

						xA = x0 + x;
						yA = y0 + y;
					}
					// top samples:
					else
					{
						x = i - 4;
						y = -1;

						xB = x0 + x;
						yB = y0 + y;
					}

					int p[9][5];
					if ((ni.mbAddrA == -1) || (ni.mbAddrB == -1) ||
						((x > 3) && ((luma4x4BlkIdx == 3) || (luma4x4BlkIdx == 11))))
					{
						//p(x,y) = -1;
					}
					else
					{
						
					}
					

				}

				int mex = 0;
			}
		}

		// TODO: ChromaArrayType handling (see mb_pred(mb_type))
	}	
}