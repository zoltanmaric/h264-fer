//#include <stdlib.h>
//#include <string.h>
//#ifndef STDIO
//#include <stdio.h>
//#endif

#include "mode_pred.h"
#include "h264_globals.h"
#include "residual.h"
#include "h264_math.h"

#define Max(a,b) ((a)>(b)?(a):(b))
#define Min(a,b) ((a)<(b)?(a):(b))
#define Median(a,b,c) Max(Min(a,b),Min(c,Max(a,b)))

int mvL0x[100000], mvL0y[100000];
int subMvCnt[100000], refIdxL0[100000];

int pomocna;


// (6.4.2.1)
void InverseMacroblockPartitionScan(int mbPartIdx, int *x, int *y)
{
	*x = InverseRasterScan(mbPartIdx, P_and_SP_macroblock_modes[mb_type][5], P_and_SP_macroblock_modes[mb_type][6], 16, 0);
	*y = InverseRasterScan(mbPartIdx, P_and_SP_macroblock_modes[mb_type][5], P_and_SP_macroblock_modes[mb_type][6], 16, 1);
}

void get_neighbour_mv(int mbAddrN, int mbPartIdx, int * mvNx, int * mvNy, int * refIdxL0N)
{
	if (P_and_SP_macroblock_modes[ mb_type_array[mbAddrN] ][2] == NA || P_and_SP_macroblock_modes[ mb_type_array[mbAddrN] ][2] == 0) 
	{
		*mvNx = 0; *mvNy = 0; *refIdxL0N = -1;
		return;
	} 
	*mvNx = MPI_mvL0x(mbAddrN, mbPartIdx);
	*mvNy = MPI_mvL0y(mbAddrN, mbPartIdx);
	*refIdxL0N = MPI_refIdxL0(mbAddrN);
}

// (6.4.11)
void DeriveNeighbourLocation(int xN, int yN, int * mbAddrN, int * xW, int * yW, bool * validN)
{
	*xW = xN; *yW = yN;
	*validN = false;
	if (*xW > 15 && *yW >= 0) return;
	if (*yW > 15) return;
	*validN = true;
	*mbAddrN = CurrMbAddr;
	if (*xW >= 0 && *xW < 16 && *yW >= 0) return;
	*mbAddrN = CurrMbAddr - frame.Lwidth/16;
	if (*xW >= 0 && *xW < 16) 
	{ 
		if (CurrMbAddr < frame.Lwidth/16) *validN = false;
		*yW += 16; 
		return; 
	}
	(*mbAddrN)++;
	if (*xW > 15) 
	{
		if (CurrMbAddr < frame.Lwidth/16) *validN = false;
		*xW -= 16; *yW += 16;
		if ((*mbAddrN)%(frame.Lwidth/16) == 0) *validN = false;
		return;
	}
	*xW += 16;
	*mbAddrN -= 2;
	if (*yW < 0) 
	{ 
		if (CurrMbAddr < frame.Lwidth/16) *validN = false;
		if (CurrMbAddr % (frame.Lwidth/16) == 0) *validN = false;
		*yW += 16; 
		return; 
	}
	if (CurrMbAddr % (frame.Lwidth/16) == 0) *validN = false;
	*mbAddrN = CurrMbAddr - 1;
	return;
}

// (6.4.12.4)
void derivation_process_for_macroblock_and_submb_partition(int xP, int yP, int mbType, int * mbPartIdx, int * subMbPartIdx)
{
	if (P_and_SP_macroblock_modes[mbType][2] == NA)
		*mbPartIdx = 0;
	else
		*mbPartIdx = 2*(yP / P_and_SP_macroblock_modes[mbType][6]) + (xP / P_and_SP_macroblock_modes[mbType][5]);
	if (mbType != P_8x8 && mbType != P_8x8ref0)
		*subMbPartIdx = 0;
	else
		*subMbPartIdx = 2*((yP%8)/4) + ((xP%8)/4); // only P_8x8 and P_8x8ref0 are allowed, and sub_mb_part 4x4
}

// (6.4.10.7)
void DeriveNeighbourPartitions(int mbPartIdx, int subMbPartIdx,
							int * mbAddrA, int * mbPartIdxA, int * subMbPartIdxA, bool * validA,
							int * mbAddrB, int * mbPartIdxB, int * subMbPartIdxB, bool * validB,
							int * mbAddrC, int * mbPartIdxC, int * subMbPartIdxC, bool * validC,
							int * mbAddrD, int * mbPartIdxD, int * subMbPartIdxD, bool * validD)
{
	int x, y;
	int xS = 0, yS = 0, predPartWidth;
	int xA, xB, xC, xD, yA, yB, yC, yD, xW, yW;

	InverseMacroblockPartitionScan(mbPartIdx, &x, &y);
	/*if (mb_type == P_8x8 || mb_type == P_8x8ref0)
		inverse_4x4_luma_block_scanning_process(subMbPartIdx, &xS, &yS);*/

	predPartWidth = 16;
	if (mb_type == P_8x8 || mb_type == P_8x8ref0)
		predPartWidth = 8;
	if (mb_type == P_L0_L0_8x16)
		predPartWidth = 8;

	xA = x + xS - 1;
	yA = y + yS;
	xB = x + xS;
	yB = y + yS - 1;
	xC = x + xS + predPartWidth;
	yC = y + yS - 1;
	xD = x + xS - 1;
	yD = y + yS - 1;

	DeriveNeighbourLocation(xA, yA, mbAddrA, &xW, &yW, validA);
	if (*validA)
		derivation_process_for_macroblock_and_submb_partition(xW, yW, mb_type_array[*mbAddrA], mbPartIdxA, subMbPartIdxA);
	DeriveNeighbourLocation(xB, yB, mbAddrB, &xW, &yW, validB);
	if (*validB)
		derivation_process_for_macroblock_and_submb_partition(xW, yW, mb_type_array[*mbAddrB], mbPartIdxB, subMbPartIdxB);
	DeriveNeighbourLocation(xC, yC, mbAddrC, &xW, &yW, validC);
	if (*validC)
		derivation_process_for_macroblock_and_submb_partition(xW, yW, mb_type_array[*mbAddrC], mbPartIdxC, subMbPartIdxC);
	DeriveNeighbourLocation(xD, yD, mbAddrD, &xW, &yW, validD);
	if (*validD)
		derivation_process_for_macroblock_and_submb_partition(xW, yW, mb_type_array[*mbAddrD], mbPartIdxD, subMbPartIdxD);
}

// 8.4.1.3, subMB not supported, refIdxL0 already assigned in mpi
void PredictMV_Luma(int mbPartIdx)
{
	int curr_refIdxL0 = MPI_refIdxL0(CurrMbAddr);
	int mvNx[4], mvNy[4], refIdxL0N[3];
	int mbAddrN[4], mbPartIdxN[4], subMbPartIdxN[4];
	bool validN[4];
	for (int i = 0; i < 3; i++) { mvNx[i] = mvNy[i] = MV_NA; refIdxL0N[i] = -1; }
	DeriveNeighbourPartitions(mbPartIdx, 0, 
							&mbAddrN[0], &mbPartIdxN[0], &subMbPartIdxN[0], &validN[0],
							&mbAddrN[1], &mbPartIdxN[1], &subMbPartIdxN[1], &validN[1],
							&mbAddrN[2], &mbPartIdxN[2], &subMbPartIdxN[2], &validN[2],
							&mbAddrN[3], &mbPartIdxN[3], &subMbPartIdxN[3], &validN[3]);
	if (!validN[2])
	{
		validN[2] = validN[3];
		mbAddrN[2] = mbAddrN[3];
		mbPartIdxN[2] = mbPartIdxN[3];
		subMbPartIdxN[2] = subMbPartIdxN[3];
	}

	for (int i = 0; i < 3; i++)
		if (validN[i]) 
		{
			//if (mb_type_array[mbAddrN[i]] == P_L0_L0_16x8) mbPartIdxN[i] *= 2;
			get_neighbour_mv(mbAddrN[i], mbPartIdxN[i], &mvNx[i], &mvNy[i], &refIdxL0N[i]);
		}

	if (mb_type == P_L0_L0_16x8 && mbPartIdx == 0 && mvNx[1] != MV_NA && curr_refIdxL0 == refIdxL0N[1])
	{
		MPI_mvL0x(CurrMbAddr, mbPartIdx) = mvNx[1]; MPI_mvL0y(CurrMbAddr, mbPartIdx) = mvNy[1];
		return;
	}
	if (mb_type == P_L0_L0_16x8 && mbPartIdx == 1 && mvNx[0] != MV_NA && curr_refIdxL0 == refIdxL0N[0])
	{
		MPI_mvL0x(CurrMbAddr, mbPartIdx) = mvNx[0]; MPI_mvL0y(CurrMbAddr, mbPartIdx) = mvNy[0];
		return;
	}
	if (mb_type == P_L0_L0_8x16 && mbPartIdx == 0 && mvNx[0] != MV_NA && curr_refIdxL0 == refIdxL0N[0])
	{
		MPI_mvL0x(CurrMbAddr, mbPartIdx) = mvNx[0]; MPI_mvL0y(CurrMbAddr, mbPartIdx) = mvNy[0];
		return;
	}
	if (mb_type == P_L0_L0_8x16 && mbPartIdx == 1 && mvNx[2] != MV_NA && curr_refIdxL0 == refIdxL0N[2])
	{
		MPI_mvL0x(CurrMbAddr, mbPartIdx) = mvNx[2]; MPI_mvL0y(CurrMbAddr, mbPartIdx) = mvNy[2];
		return;
	}
	if (mvNx[0] == MV_NA && mvNx[1] == MV_NA)
	{
		mvNx[0] = 0; mvNy[0] = 0; refIdxL0N[0] = curr_refIdxL0;
	}
	if (mvNx[0] == MV_NA && mvNx[1] != MV_NA)
	{
		mvNx[0] = mvNx[1]; mvNy[0] = mvNy[1]; refIdxL0N[0] = refIdxL0N[1];
	}
	if (mvNx[1] == MV_NA)
	{
		mvNx[1] = mvNx[0]; mvNy[1] = mvNy[0]; refIdxL0N[1] = refIdxL0N[0];
	}
	if (mvNx[2] == MV_NA)
	{
		mvNx[2] = mvNx[0]; mvNy[2] = mvNy[0]; refIdxL0N[2] = refIdxL0N[0];
	}
	if (refIdxL0N[0] == curr_refIdxL0 && refIdxL0N[1] != curr_refIdxL0 && refIdxL0N[2] != curr_refIdxL0)
	{
		MPI_mvL0x(CurrMbAddr, mbPartIdx) = mvNx[0]; MPI_mvL0y(CurrMbAddr, mbPartIdx) = mvNy[0];
		return;
	}
	if (refIdxL0N[0] != curr_refIdxL0 && refIdxL0N[1] == curr_refIdxL0 && refIdxL0N[2] != curr_refIdxL0)
	{
		MPI_mvL0x(CurrMbAddr, mbPartIdx) = mvNx[1]; MPI_mvL0y(CurrMbAddr, mbPartIdx) = mvNy[1];
		return;
	}
	if (refIdxL0N[0] != curr_refIdxL0 && refIdxL0N[1] != curr_refIdxL0 && refIdxL0N[2] == curr_refIdxL0)
	{
		MPI_mvL0x(CurrMbAddr, mbPartIdx) = mvNx[2]; MPI_mvL0y(CurrMbAddr, mbPartIdx) = mvNy[2];
		return;
	}
	// if everything is OK, Median of neighbouring partition is used
	MPI_mvL0x(CurrMbAddr, mbPartIdx) = Median(mvNx[0], mvNx[1], mvNx[2]);
	MPI_mvL0y(CurrMbAddr, mbPartIdx) = Median(mvNy[0], mvNy[1], mvNy[2]);
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

void ClearMVD()
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			for (int k = 0; k < 2; k++)
				mvd_l0[i][j][k] = 0;
}

void PredictMV()
{
	if (mb_type == P_Skip)
	{
		ClearMVD();
		MPI_refIdxL0(CurrMbAddr) = 0;

		if (CurrMbAddr < sps.PicWidthInMbs || CurrMbAddr%sps.PicWidthInMbs == 0)
		{
			MPI_mvL0x(CurrMbAddr, 0) = 0;
			MPI_mvL0y(CurrMbAddr, 0) = 0;
		} else {
			//int u = MPI_mvL0x(CurrMbAddr-sps.PicWidthInMbs, 2);
			//int u1 = MPI_mvL0y(CurrMbAddr-sps.PicWidthInMbs, 2);
			//int u2 = MPI_mvL0x(CurrMbAddr-1, 1);
			//int u3 = MPI_mvL0y(CurrMbAddr-1, 1);
			if (((P_and_SP_macroblock_modes[mb_type_array[CurrMbAddr-sps.PicWidthInMbs]][2] == 0) | (P_and_SP_macroblock_modes[mb_type_array[CurrMbAddr-sps.PicWidthInMbs]][2] == NA) | MPI_refIdxL0(CurrMbAddr-sps.PicWidthInMbs) | MPI_mvL0x(CurrMbAddr-sps.PicWidthInMbs, 2) | MPI_mvL0y(CurrMbAddr-sps.PicWidthInMbs, 2)) == 0 ||
				((P_and_SP_macroblock_modes[mb_type_array[CurrMbAddr-1]][2] == 0) | (P_and_SP_macroblock_modes[mb_type_array[CurrMbAddr-1]][2] == NA) | MPI_refIdxL0(CurrMbAddr-1) | MPI_mvL0x(CurrMbAddr-1, 1) | MPI_mvL0y(CurrMbAddr-1, 1)) == 0)
			{
				MPI_mvL0x(CurrMbAddr, 0) = 0;
				MPI_mvL0y(CurrMbAddr, 0) = 0;
			} else {
				PredictMV_Luma(0);
				MPI_mvL0x(CurrMbAddr, 0) += mvd_l0[0][0][0];
				MPI_mvL0y(CurrMbAddr, 0) += mvd_l0[0][0][1];
			}
		}
	} else { // in baseline profile cannot occur B_* MB_TYPE, so, except P_SKIP, normal derivation for luma vector prediction is used
		PredictMV_Luma(0);
		//int t_1 = MPI_mvL0x(CurrMbAddr, 0), t_2 = MPI_mvL0y(CurrMbAddr, 0);
		MPI_mvL0x(CurrMbAddr, 0) += mvd_l0[0][0][0];
		MPI_mvL0y(CurrMbAddr, 0) += mvd_l0[0][0][1];
		if (NumMbPart(mb_type) > 1)
		{
			PredictMV_Luma(1);
			MPI_mvL0x(CurrMbAddr, 1) += mvd_l0[1][0][0];
			MPI_mvL0y(CurrMbAddr, 1) += mvd_l0[1][0][1];
			if (NumMbPart(mb_type) > 2)
			{
				PredictMV_Luma(2);
				MPI_mvL0x(CurrMbAddr, 2) += mvd_l0[2][0][0];
				MPI_mvL0y(CurrMbAddr, 2) += mvd_l0[2][0][1];
				PredictMV_Luma(3);
				MPI_mvL0x(CurrMbAddr, 3) += mvd_l0[3][0][0];
				MPI_mvL0y(CurrMbAddr, 3) += mvd_l0[3][0][1];
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
		//if (mb_type_array[CurrMbAddr] == P_L0_L0_8x16)
		//{
		//	MPI_mvL0x(CurrMbAddr, i) += mvd_l0[i%2][0][0];
		//	MPI_mvL0y(CurrMbAddr, i) += mvd_l0[i%2][0][1];
		//} else {
		//	MPI_mvL0x(CurrMbAddr, i) += mvd_l0[i/k][0][0];
		//	MPI_mvL0y(CurrMbAddr, i) += mvd_l0[i/k][0][1];
		//}
		bla[i][0][0] = MPI_mvL0x(CurrMbAddr, i);
		bla[i][0][1] = MPI_mvL0y(CurrMbAddr, i);
		for (int j = 0; j < 4; j++)
		{
			MPI_mvSubL0x_byIdx(CurrMbAddr, i, j) = MPI_mvL0x(CurrMbAddr, i);
			MPI_mvSubL0y_byIdx(CurrMbAddr, i, j) = MPI_mvL0y(CurrMbAddr, i);
			bla[i][j][0] = MPI_mvSubL0x_byIdx(CurrMbAddr, i, j);
			bla[i][j][1] = MPI_mvSubL0y_byIdx(CurrMbAddr, i, j);
			int test = 0;
		}
	}
	if (CurrMbAddr == 2880)
	{
		int test2 = 1;
	}
}