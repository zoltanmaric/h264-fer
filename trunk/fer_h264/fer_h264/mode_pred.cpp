//#include <stdlib.h>
//#include <string.h>
//#ifndef STDIO
//#include <stdio.h>
//#endif

#include "mode_pred.h"
#include "h264_globals.h"
#include "h264_math.h"

#define Max(a,b) ((a)>(b)?(a):(b))
#define Min(a,b) ((a)<(b)?(a):(b))
#define Median(a,b,c) Max(Min(a,b),Min(c,Max(a,b)))

int mvL0x[100000], mvL0y[100000];
int subMvCnt[100000], refIdxL0[100000];

// return false if neighbour MV is not valid (different refIdxL0)
bool get_neighbour_mv(int org_x, int org_y, int neighbourMbAddr, int mbPartIdx, int curr_refIdxL0, int * mvNx, int * mvNy, int * refIdxL0N)
{
	*mvNx = MV_NA; *mvNy = MV_NA;
	if (org_x < 0 || org_y < 0 || org_x >= frame.Lwidth) return true; // not available, but still valid
	if (   mb_type_array[neighbourMbAddr] != P_L0_16x16
		&& mb_type_array[neighbourMbAddr] != P_8x8
		&& mb_type_array[neighbourMbAddr] != P_L0_L0_16x8
		&& mb_type_array[neighbourMbAddr] != P_L0_L0_8x16
		&& mb_type_array[neighbourMbAddr] != P_8x8ref0
		&& mb_type_array[neighbourMbAddr] != P_Skip
		|| MPI_refIdxL0(neighbourMbAddr) != curr_refIdxL0) 
	{
		*mvNx = 0; *mvNy = 0; *refIdxL0N = -1;
		return false;
	} 
	*mvNx = MPI_mvL0x(neighbourMbAddr, mbPartIdx);
	*mvNy = MPI_mvL0y(neighbourMbAddr, mbPartIdx);
	*refIdxL0N = MPI_refIdxL0(neighbourMbAddr);
	return true;
}

// 8.4.1.3, subMB not supported, refIdxL0 already assigned in mpi
void PredictMV_Luma(int mbPartIdx)
{
	int curr_refIdxL0 = MPI_refIdxL0(CurrMbAddr);
	int mvAx, mvAy, mvBx, mvBy, mvCx, mvCy, refIdxL0A, refIdxL0B, refIdxL0C, validA, validB, validC;
	
	// inverse macroblock scanning process (6.4.1)
	int org_x = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 0);
	int org_y = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 1);
	if (mbPartIdx == 0)
	{
		validA = get_neighbour_mv(org_x-8, org_y, CurrMbAddr-1, 1, curr_refIdxL0, &mvAx, &mvAy, &refIdxL0A);
		validB = get_neighbour_mv(org_x, org_y-8, CurrMbAddr-sps.PicWidthInMbs, 2, curr_refIdxL0, &mvBx, &mvBy, &refIdxL0B);
		validC = get_neighbour_mv(org_x+8, org_y-8, CurrMbAddr-sps.PicWidthInMbs+1, 2, curr_refIdxL0, &mvCx, &mvCy, &refIdxL0C);
	}
	if (mbPartIdx == 1 && mb_type != P_L0_L0_16x8)
	{
		validA = get_neighbour_mv(org_x-8, org_y, CurrMbAddr-1, 1, curr_refIdxL0, &mvAx, &mvAy, &refIdxL0A);
		validB = get_neighbour_mv(org_x, org_y-8, CurrMbAddr-sps.PicWidthInMbs, 3, curr_refIdxL0, &mvBx, &mvBy, &refIdxL0B);
		validC = get_neighbour_mv(org_x+8, org_y-8, CurrMbAddr-sps.PicWidthInMbs+1, 2, curr_refIdxL0, &mvCx, &mvCy, &refIdxL0C);
	}
	if (mbPartIdx == 1 && mb_type == P_L0_L0_16x8)
	{
		validA = get_neighbour_mv(org_x-8, org_y, CurrMbAddr-1, 3, curr_refIdxL0, &mvAx, &mvAy, &refIdxL0A);
		validB = get_neighbour_mv(org_x, org_y-8, CurrMbAddr, 0, curr_refIdxL0, &mvBx, &mvBy, &refIdxL0B);
		validC = get_neighbour_mv(org_x+8, org_y-8, CurrMbAddr-sps.PicWidthInMbs+1, 2, curr_refIdxL0, &mvCx, &mvCy, &refIdxL0C);
	}
	if (mbPartIdx == 2)
	{
		validA = get_neighbour_mv(org_x-8, org_y, CurrMbAddr-1, 3, curr_refIdxL0, &mvAx, &mvAy, &refIdxL0A);
		validB = get_neighbour_mv(org_x, org_y-8, CurrMbAddr-sps.PicWidthInMbs, 2, curr_refIdxL0, &mvBx, &mvBy, &refIdxL0B);
		validC = get_neighbour_mv(org_x+8, org_y-8, CurrMbAddr-sps.PicWidthInMbs+1, 2, curr_refIdxL0, &mvCx, &mvCy, &refIdxL0C);
	}
	if (mbPartIdx == 3)
	{
		validA = get_neighbour_mv(org_x-8, org_y, CurrMbAddr-1, 3, curr_refIdxL0, &mvAx, &mvAy, &refIdxL0A);
		validB = get_neighbour_mv(org_x, org_y-8, CurrMbAddr-sps.PicWidthInMbs, 3, curr_refIdxL0, &mvBx, &mvBy, &refIdxL0B);
		validC = get_neighbour_mv(org_x+8, org_y-8, CurrMbAddr-sps.PicWidthInMbs+1, 2, curr_refIdxL0, &mvCx, &mvCy, &refIdxL0C);
	}

	if (mb_type == P_L0_L0_16x8 && mbPartIdx == 0 && mvBx != MV_NA && curr_refIdxL0 == refIdxL0B)
	{
		MPI_mvL0x(CurrMbAddr, mbPartIdx) = mvBx; MPI_mvL0y(CurrMbAddr, mbPartIdx) = mvBy;
		return;
	}
	if (mb_type == P_L0_L0_16x8 && mbPartIdx == 1 && mvAx != MV_NA && curr_refIdxL0 == refIdxL0A)
	{
		MPI_mvL0x(CurrMbAddr, mbPartIdx) = mvAx; MPI_mvL0y(CurrMbAddr, mbPartIdx) = mvAy;
		return;
	}
	if (mb_type == P_L0_L0_8x16 && mbPartIdx == 0 && mvAx != MV_NA && curr_refIdxL0 == refIdxL0A)
	{
		MPI_mvL0x(CurrMbAddr, mbPartIdx) = mvAx; MPI_mvL0y(CurrMbAddr, mbPartIdx) = mvAy;
		return;
	}
	if (mb_type == P_L0_L0_8x16 && mbPartIdx == 1 && mvCx != MV_NA && curr_refIdxL0 == refIdxL0C)
	{
		MPI_mvL0x(CurrMbAddr, mbPartIdx) = mvCx; MPI_mvL0y(CurrMbAddr, mbPartIdx) = mvCy;
		return;
	}
	if (mvAx == MV_NA && mvBx == MV_NA)
	{
		mvAx = 0; mvAy = 0; refIdxL0A = curr_refIdxL0;
	}
	if (mvBx == MV_NA)
	{
		mvBx = mvAx; mvBy = mvAy; refIdxL0B = refIdxL0A;
	}
	if (mvCx == MV_NA)
	{
		mvCx = mvAx; mvCy = mvAy; refIdxL0C = refIdxL0A;
	}
	if (refIdxL0A == curr_refIdxL0 && refIdxL0B != curr_refIdxL0 && refIdxL0C != curr_refIdxL0)
	{
		MPI_mvL0x(CurrMbAddr, mbPartIdx) = mvAx; MPI_mvL0y(CurrMbAddr, mbPartIdx) = mvAy;
		return;
	}
	if (refIdxL0A != curr_refIdxL0 && refIdxL0B == curr_refIdxL0 && refIdxL0C != curr_refIdxL0)
	{
		MPI_mvL0x(CurrMbAddr, mbPartIdx) = mvBx; MPI_mvL0y(CurrMbAddr, mbPartIdx) = mvBy;
		return;
	}
	if (refIdxL0A != curr_refIdxL0 && refIdxL0B != curr_refIdxL0 && refIdxL0C == curr_refIdxL0)
	{
		MPI_mvL0x(CurrMbAddr, mbPartIdx) = mvCx; MPI_mvL0y(CurrMbAddr, mbPartIdx) = mvCy;
		return;
	}
	// if everything is OK, Median of neighbouring partition is used
	MPI_mvL0x(CurrMbAddr, mbPartIdx) = Median(mvAx, mvBx, mvCx);
	MPI_mvL0y(CurrMbAddr, mbPartIdx) = Median(mvAy, mvBy, mvCy);
	if (mb_type == P_8x8 || mb_type == P_8x8ref0)
	{
		// Here should be part for generating predicted MV of 4x4 parts (if defined).
		// For now it's equal to MV of current macroblock part (8x8 size).
		for (int i = 1; i < 4; i++)
		{
			MPI_mvSubL0x_byIdx(CurrMbAddr, mbPartIdx, i) = MPI_mvL0x(CurrMbAddr, mbPartIdx);
			MPI_mvSubL0x_byIdx(CurrMbAddr, mbPartIdx, i) = MPI_mvL0y(CurrMbAddr, mbPartIdx);
		}
	}
}

void PredictMV()
{
	if (mb_type == P_Skip)
	{
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				for (int k = 0; k < 2; k++)
					mvd_l0[i][j][k] = 0;
		MPI_refIdxL0(CurrMbAddr) = 0;
		if (CurrMbAddr < sps.PicWidthInMbs || CurrMbAddr%sps.PicWidthInMbs == 0)
		{
			MPI_mvL0x(CurrMbAddr, 0) = 0;
			MPI_mvL0y(CurrMbAddr, 0) = 0;
		} else {
			int u = MPI_mvL0x(CurrMbAddr-sps.PicWidthInMbs, 0);
			int u1 = *(mvL0x+16*(CurrMbAddr-sps.PicWidthInMbs));
			if ((MPI_refIdxL0(CurrMbAddr-sps.PicWidthInMbs) | MPI_mvL0x(CurrMbAddr-sps.PicWidthInMbs, 0) | MPI_mvL0y(CurrMbAddr-sps.PicWidthInMbs, 0)) == 0 ||
				(MPI_refIdxL0(CurrMbAddr-1) | MPI_mvL0x(CurrMbAddr-1, 0) | MPI_mvL0y(CurrMbAddr-1, 0)) == 0)
			{
				MPI_mvL0x(CurrMbAddr, 0) = 0;
				MPI_mvL0y(CurrMbAddr, 0) = 0;
			} else {
				PredictMV_Luma(0);
			}
		}
	} else { // in baseline profile cannot occur B_* MB_TYPE, so, except P_SKIP, normal derivation for luma vector prediction is used
		PredictMV_Luma(0);
		if (NumMbPart(mb_type) > 1)
		{
			PredictMV_Luma(1);
			if (NumMbPart(mb_type) > 2)
			{
				PredictMV_Luma(2);
				PredictMV_Luma(3);
			}
		}
	}
}

void DeriveMVs() {

	// Prediction
	PredictMV();
	// Populate every submacroblocks (for example, in 16x8 partition, motion vectors for two more subMBhas to be assigned).
	if (NumMbPart(mb_type) == 1)
	{
			MPI_mvL0x(CurrMbAddr, 1) = MPI_mvL0x(CurrMbAddr, 0);
			MPI_mvL0y(CurrMbAddr, 1) = MPI_mvL0y(CurrMbAddr, 0);
			MPI_mvL0x(CurrMbAddr, 2) = MPI_mvL0x(CurrMbAddr, 0);
			MPI_mvL0y(CurrMbAddr, 2) = MPI_mvL0y(CurrMbAddr, 0);
			MPI_mvL0x(CurrMbAddr, 3) = MPI_mvL0x(CurrMbAddr, 0);
			MPI_mvL0y(CurrMbAddr, 3) = MPI_mvL0y(CurrMbAddr, 0);
	}
	if (NumMbPart(mb_type) == 2)
	{
		if (mb_type == P_L0_L0_16x8) // P_16x8
		{
			MPI_mvL0x(CurrMbAddr, 2) = MPI_mvL0x(CurrMbAddr, 1);
			MPI_mvL0y(CurrMbAddr, 2) = MPI_mvL0y(CurrMbAddr, 1);
			MPI_mvL0x(CurrMbAddr, 1) = MPI_mvL0x(CurrMbAddr, 0);
			MPI_mvL0y(CurrMbAddr, 1) = MPI_mvL0y(CurrMbAddr, 0);
			MPI_mvL0x(CurrMbAddr, 3) = MPI_mvL0x(CurrMbAddr, 2);
			MPI_mvL0y(CurrMbAddr, 3) = MPI_mvL0y(CurrMbAddr, 2);
		} else { // P_8x16
			MPI_mvL0x(CurrMbAddr, 2) = MPI_mvL0x(CurrMbAddr, 0);
			MPI_mvL0y(CurrMbAddr, 2) = MPI_mvL0y(CurrMbAddr, 0);
			MPI_mvL0x(CurrMbAddr, 3) = MPI_mvL0x(CurrMbAddr, 1);
			MPI_mvL0y(CurrMbAddr, 3) = MPI_mvL0y(CurrMbAddr, 1);
		}
	}

	// k is the divisor for the subMbPart index of mvd_l0
	int k;
	if (NumMbPart(mb_type) == 1)
	{
		k = 4;
	}
	else if (NumMbPart(mb_type) == 2)
	{
		k = 2;
	}
	else
	{
		k = 1;
	}
	int bla[4][4][2];
	// Adding given difference
	for (int i = 0; i < 4; i++)
	{	
		if (mb_type_array[CurrMbAddr] == P_L0_L0_8x16)
		{
			MPI_mvL0x(CurrMbAddr, i) += mvd_l0[i%2][0][0];
			MPI_mvL0y(CurrMbAddr, i) += mvd_l0[i%2][0][1];
		} else {
			MPI_mvL0x(CurrMbAddr, i) += mvd_l0[i/k][0][0];
			MPI_mvL0y(CurrMbAddr, i) += mvd_l0[i/k][0][1];
		}
		bla[i][0][0] = MPI_mvL0x(CurrMbAddr, i);
		bla[i][0][1] = MPI_mvL0y(CurrMbAddr, i);
		if (NumMbPart(mb_type) != 4)
		{ // If current macroblock isn't 8x8 partitioned, then every 4x4 subpart in submacroblocks has the same MV.
			for (int j = 0; j < 4; j++)
			{
				MPI_mvSubL0x_byIdx(CurrMbAddr, i, j) = MPI_mvL0x(CurrMbAddr, i);
				MPI_mvSubL0y_byIdx(CurrMbAddr, i, j) = MPI_mvL0y(CurrMbAddr, i);
				bla[i][j][0] = MPI_mvSubL0x_byIdx(CurrMbAddr, i, j);
				bla[i][j][1] = MPI_mvSubL0y_byIdx(CurrMbAddr, i, j);
				int test = 0;
			}
		} else {
			// If current macroblock is 8x8 partitioned, then only mvdx, mvdy is added to predicted MV.
			for (int j = 0; j < 4; j++)
			{
				if (mb_type == P_L0_L0_8x16)
				{
					MPI_mvSubL0x_byIdx(CurrMbAddr, i, j) += mvd_l0[i%2][j][0];
					MPI_mvSubL0y_byIdx(CurrMbAddr, i, j) += mvd_l0[i%2][j][1];
				} else {
					MPI_mvSubL0x_byIdx(CurrMbAddr, i, j) += mvd_l0[i/k][j][0];
					MPI_mvSubL0y_byIdx(CurrMbAddr, i, j) += mvd_l0[i/k][j][1];
				}
				bla[i][j][0] = MPI_mvSubL0x_byIdx(CurrMbAddr, i, j);
				bla[i][j][1] = MPI_mvSubL0y_byIdx(CurrMbAddr, i, j);
				int test = 0;
			}
		}
	}
	int test2 = 1;
}