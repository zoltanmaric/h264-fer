#include "intra.h"
#include "mbMode.h"
#include "mbPredictionMode.h"

typedef struct
{
	unsigned char samples[16][16];
} macroblock;

typedef struct
{
	unsigned char samples[4][4];
} submacroblock4x4;

typedef struct
{
	unsigned char pitch;
} mb_info;

unsigned char Intra4x4Scan[16][2] = {
  { 0, 0},  { 4, 0},  { 0, 4},  { 4, 4},
  { 8, 0},  {12, 0},  { 8, 4},  {12, 4},
  { 0, 8},  { 4, 8},  { 0,12},  { 4,12},
  { 8, 8},  {12, 8},  { 8,12},  {12,12}
};

void intraPrediction(frame *f, mb_pred_info mb)
{
	if (mb.mb_type != I_PCM)
	{
		if (mb.mode == I_4x4)
		{
			Intra_4x4(f, 0, 0);
		}
		else if ((mb.mode >= I_16x16_0_0_0) &&
				 (mb.mode <= I_16x16_3_2_1))
		{
			Intra_16x16(f);
		}
	}
}

void Intra4x4(frame *f, int mb_x, int mb_y, mb_pred_info mb_info)
{
	submacroblock4x4 subMB[16];

	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			// TODO: fill subMB		
		}
	}

	for (int luma4x4BlkIdx = 0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
	{
		char xD = -1;
		char yD = 0;
		unsigned char x = Intra4x4Scan[luma4x4BlkIdx][0];
		unsigned char y = Intra4x4Scan[luma4x4BlkIdx][1];

		char xN = x + xD;
		char yN = y + yD;

		unsigned char maxW = 16;
		unsigned char maxH = 16;

		bool mbAddrA = ((xN < 0) && (yN >= 0) && (yN < maxH - 1)) ? true : false;
		bool mbAddrB = ((xN >= 0) && (xN < maxW - 1) && (yN < 0)) ? true : false;

		char xW = (xN + maxW) % maxW;
		char yW = (yN + maxH) % maxH;
		
		unsigned char luma4x4BlkIdxA;
		if (mbAddrA == true)
		{
			luma4x4BlkIdxA = 8 * (yW / 8) + 4 * (xW / 8) + 2 * ((yW % 8) / 4) + ((xW % 8) / 4);
		}

		unsigned char luma4x4BlkIdxB;
		if (mbAddrB == true)
		{
			luma4x4BlkIdxB = 8 * (yW / 8) + 4 * (xW / 8) + 2 * ((yW % 8) / 4) + ((xW % 8) / 4);
		} 

	}
}