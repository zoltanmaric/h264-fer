#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "mode_pred.h"
#include "h264_globals.h"

void MPI_Init()
{
	infos->mb_type =  (int *)malloc(FR_numRows*FR_MB_in_row*sizeof(int));
	infos->mvL0x =  (int *)malloc(16*FR_numRows*FR_MB_in_row*sizeof(int)); // max 16 different motion vectors per macroblock
	infos->mvL0y =  (int *)malloc(16*FR_numRows*FR_MB_in_row*sizeof(int));
	//infos->mvCL0x = (int *)malloc(16*FR_numRows*FR_MB_in_row*sizeof(int)); // equal to mvL0x
	//infos->mvCL0y = (int *)malloc(16*FR_numRows*FR_MB_in_row*sizeof(int)); // equal to mvL0y
	infos->subMvCnt = (int *)malloc(FR_numRows*FR_MB_in_row*sizeof(int));
	infos->refIdxL0 = (int *)malloc(FR_numRows*FR_MB_in_row*sizeof(int));
	MPI_Clear();
}

void MPI_Clear()
{
	memset(infos->mb_type, -1, FR_numRows*FR_MB_in_row*sizeof(int));
	memset(infos->mvL0x, 0, 16*FR_numRows*FR_MB_in_row*sizeof(int));
	memset(infos->mvL0y, 0, 16*FR_numRows*FR_MB_in_row*sizeof(int));
	//memset(infos->mvCL0x, 0, 16*FR_numRows*FR_MB_in_row*sizeof(int)); // equal to mvL0x
	//memset(infos->mvCL0y, 0, 16*FR_numRows*FR_MB_in_row*sizeof(int)); // equal to mvL0y
	memset(infos->subMvCnt, 0, FR_numRows*FR_MB_in_row*sizeof(int));
	memset(infos->refIdxL0, -1, FR_numRows*FR_MB_in_row*sizeof(int));
}

void MPI_Free() 
{
	free(infos->mb_type);
	free(infos->mvL0x);
	free(infos->mvL0y);
	//free(infos->mvCL0x); // equal to mvL0x
	//free(infos->mvCL0y); // equal to mvL0y
	free(infos->subMvCnt);
	free(infos->refIdxL0);
}

#define Max(a,b) ((a)>(b)?(a):(b))
#define Min(a,b) ((a)<(b)?(a):(b))
#define Median(a,b,c) Max(Min(a,b),Min(c,Max(a,b)))

// return false if neighbour MV is not valid (different refIdxL0)
bool get_neighbour_mv(int org_x, int org_y, int mbPartIdx, int curr_refIdxL0, int * mvNx, int * mvNy, int * refIdxL0N)
{
	*mvNx = MV_NA; *mvNy = MV_NA;
	if (org_x < 0 || org_y < 0 || org_x >= FRAME_Width) return true; // not available, but still valid
	if (   MPI_mb_type(org_x, org_y) != P_L0_16x16
		&& MPI_mb_type(org_x, org_y) != P_8x8
		&& MPI_mb_type(org_x, org_y) != P_L0_L0_16x8
		&& MPI_mb_type(org_x, org_y) != P_L0_L0_8x16
		&& MPI_mb_type(org_x, org_y) != P_8x8ref0
		&& MPI_mb_type(org_x, org_y) != P_Skip
		|| MPI_refIdxL0(org_x, org_y) != curr_refIdxL0) 
	{
		*mvNx = 0; *mvNy = 0; *refIdxL0N = -1;
		return false;
	} 
	*mvNx = MPI_mvL0x(org_x, org_y, mbPartIdx);
	*mvNy = MPI_mvL0y(org_x, org_y, mbPartIdx);
	*refIdxL0N = MPI_refIdxL0(org_x, org_y);
	return true;
}

// 8.4.1.3, subMB not supported, refIdxL0 already assigned in mpi
void PredictMV_Luma(int org_x, int org_y, int mbPartIdx)
{
	int curr_refIdxL0 = MPI_refIdxL0(org_x, org_y);
	int mvAx, mvAy, mvBx, mvBy, mvCx, mvCy, refIdxL0A, refIdxL0B, refIdxL0C;
	bool validA = get_neighbour_mv(org_x-16, org_y, 1, curr_refIdxL0, &mvAx, &mvAy, &refIdxL0A);
	bool validB = get_neighbour_mv(org_x, org_y-16, 1, curr_refIdxL0, &mvBx, &mvBy, &refIdxL0B);
	bool validC = get_neighbour_mv(org_x+16, org_y-16, 1, curr_refIdxL0, &mvCx, &mvCy, &refIdxL0C);
	int curr_mb_type = MPI_mb_type(org_x, org_y);

	if (curr_mb_type == P_L0_L0_16x8 && mbPartIdx == 0 && mvBx != MV_NA && curr_refIdxL0 == refIdxL0B)
	{
		MPI_mvL0x(org_x, org_y, mbPartIdx) = mvBx; MPI_mvL0y(org_x, org_y, mbPartIdx) = mvBy;
		return;
	}
	if (curr_mb_type == P_L0_L0_16x8 && mbPartIdx == 1 && mvAx != MV_NA && curr_refIdxL0 == refIdxL0A)
	{
		MPI_mvL0x(org_x, org_y, mbPartIdx) = mvAx; MPI_mvL0y(org_x, org_y, mbPartIdx) = mvAy;
		return;
	}
	if (curr_mb_type == P_L0_L0_8x16 && mbPartIdx == 0 && mvAx != MV_NA && curr_refIdxL0 == refIdxL0A)
	{
		MPI_mvL0x(org_x, org_y, mbPartIdx) = mvAx; MPI_mvL0y(org_x, org_y, mbPartIdx) = mvAy;
		return;
	}
	if (curr_mb_type == P_L0_L0_8x16 && mbPartIdx == 1 && mvCx != MV_NA && curr_refIdxL0 == refIdxL0C)
	{
		MPI_mvL0x(org_x, org_y, mbPartIdx) = mvCx; MPI_mvL0y(org_x, org_y, mbPartIdx) = mvCy;
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
		MPI_mvL0x(org_x, org_y, mbPartIdx) = mvAx; MPI_mvL0y(org_x, org_y, mbPartIdx) = mvAy;
		return;
	}
	if (refIdxL0A != curr_refIdxL0 && refIdxL0B == curr_refIdxL0 && refIdxL0C != curr_refIdxL0)
	{
		MPI_mvL0x(org_x, org_y, mbPartIdx) = mvBx; MPI_mvL0y(org_x, org_y, mbPartIdx) = mvBy;
		return;
	}
	if (refIdxL0A != curr_refIdxL0 && refIdxL0B != curr_refIdxL0 && refIdxL0C == curr_refIdxL0)
	{
		MPI_mvL0x(org_x, org_y, mbPartIdx) = mvCx; MPI_mvL0y(org_x, org_y, mbPartIdx) = mvCy;
		return;
	}
	// if everything is OK, Median of neighbouring partition is used
	MPI_mvL0x(org_x, org_y, mbPartIdx) = Median(mvAx, mvBx, mvCx);
	MPI_mvL0y(org_x, org_y, mbPartIdx) = Median(mvAy, mvBy, mvCy);
	if (curr_mb_type == P_8x8 || curr_mb_type == P_8x8ref0)
	{
		// Here should be part for generating predicted MV of 4x4 parts (if defined).
		// For now it's equal to MV of current macroblock part (8x8 size).
		for (int i = 1; i < 4; i++)
		{
			MPI_mvSubL0x(org_x, org_y, mbPartIdx, i) = MPI_mvL0x(org_x, org_y, mbPartIdx);
			MPI_mvSubL0x(org_x, org_y, mbPartIdx, i) = MPI_mvL0y(org_x, org_y, mbPartIdx);
		}
	}
}

void PredictMV(int org_x, int org_y)
{
	if (MPI_mb_type(org_x, org_y) == P_Skip)
	{
		MPI_subMvCnt(org_x, org_y) = 1;
		MPI_refIdxL0(org_x, org_y) = 0;
		if (org_x == 0 || org_y == 0)
		{
			MPI_mvL0x(org_x, org_y, 0) = 0;
			MPI_mvL0y(org_x, org_y, 0) = 0;
		} else {
			if ((MPI_refIdxL0(org_x, org_y-MB_Height) || MPI_mvL0x(org_x, org_y-MB_Height, 0) || MPI_mvL0x(org_x, org_y-MB_Height, 0)) == 0 || 
				(MPI_refIdxL0(org_x-MB_Width, org_y) || MPI_mvL0x(org_x-MB_Width, org_y, 0) || MPI_mvL0x(org_x-MB_Width, org_y, 0)) == 0)
			{
				MPI_mvL0x(org_x, org_y, 0) = 0;
				MPI_mvL0y(org_x, org_y, 0) = 0;
			} else {
				PredictMV_Luma(org_x, org_y, 0);
			}
		}
	} else { // in baseline profile cannot occur B_* MB_TYPE, so, except P_SKIP, normal derivation for luma vector prediction is used
		PredictMV_Luma(org_x, org_y, 0);
		if (MPI_subMvCnt(org_x, org_y) > 1)
		{
			PredictMV_Luma(org_x, org_y, 1);
			if (MPI_subMvCnt(org_x, org_y) > 2)
			{
				PredictMV_Luma(org_x, org_y, 2);
				PredictMV_Luma(org_x, org_y, 3);
			}
		}
	}
}

void DeriveMVs(int org_x, int org_y, int mvdx, int mvdy) {
	// Prediction
	PredictMV(org_x,org_y);
	// Populate every submacroblocks (for example, in 16x8 partition, motion vectors for two more subMBhas to be assigned).
	if (MPI_subMvCnt(org_x, org_y) == 1)
	{
			MPI_mvL0x(org_x, org_y, 1) = MPI_mvL0x(org_x, org_y, 0);
			MPI_mvL0y(org_x, org_y, 1) = MPI_mvL0y(org_x, org_y, 0);
			MPI_mvL0x(org_x, org_y, 2) = MPI_mvL0x(org_x, org_y, 0);
			MPI_mvL0y(org_x, org_y, 2) = MPI_mvL0y(org_x, org_y, 0);
			MPI_mvL0x(org_x, org_y, 3) = MPI_mvL0x(org_x, org_y, 0);
			MPI_mvL0y(org_x, org_y, 3) = MPI_mvL0y(org_x, org_y, 0);
	}
	if (MPI_subMvCnt(org_x, org_y) == 2)
	{
		if (MPI_mb_type(org_x,org_y) == P_L0_L0_16x8) // P_16x8
		{
			MPI_mvL0x(org_x, org_y, 2) = MPI_mvL0x(org_x, org_y, 1);
			MPI_mvL0y(org_x, org_y, 2) = MPI_mvL0y(org_x, org_y, 1);
			MPI_mvL0x(org_x, org_y, 1) = MPI_mvL0x(org_x, org_y, 0);
			MPI_mvL0y(org_x, org_y, 1) = MPI_mvL0y(org_x, org_y, 0);
			MPI_mvL0x(org_x, org_y, 3) = MPI_mvL0x(org_x, org_y, 2);
			MPI_mvL0y(org_x, org_y, 3) = MPI_mvL0y(org_x, org_y, 2);
		} else { // P_8x16
			MPI_mvL0x(org_x, org_y, 2) = MPI_mvL0x(org_x, org_y, 0);
			MPI_mvL0y(org_x, org_y, 2) = MPI_mvL0y(org_x, org_y, 0);
			MPI_mvL0x(org_x, org_y, 3) = MPI_mvL0x(org_x, org_y, 1);
			MPI_mvL0y(org_x, org_y, 3) = MPI_mvL0y(org_x, org_y, 1);
		}
	}
	// Adding given difference
	for (int i = 0; i < 4; i++)
	{
		MPI_mvL0x(org_x, org_y, i) += mvdx;
		MPI_mvL0y(org_x, org_y, i) += mvdy;
		if (MPI_subMvCnt(org_x, org_y) != 4)
		{ // If current macroblock isn't 8x8 partitioned, then every 4x4 subpart in submacroblocks has the same MV.
			for (int j = 0; j < 4; j++)
			{
				MPI_mvSubL0x(org_x, org_y, i, j) = MPI_mvL0x(org_x, org_y, i);
				MPI_mvSubL0y(org_x, org_y, i, j) = MPI_mvL0y(org_x, org_y, i);
			}
		} else {
			// If current macroblock is 8x8 partitioned, then only mvdx, mvdy is added to predicted MV.
			for (int j = 0; j < 4; j++)
			{
				MPI_mvSubL0x(org_x, org_y, i, j) += mvdx;
				MPI_mvSubL0y(org_x, org_y, i, j) += mvdy;
			}
		}
	}
}