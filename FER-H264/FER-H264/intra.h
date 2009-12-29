#pragma once

// this single structure contains data
// for each macroblock in a frame/slice
typedef struct _mode_pred_info {
	int slice_type;
	// per-macroblock information     (16x16)
	int MbWidth, MbHeight, MbPitch;	// frame dimensions in number of macroblocks
	int *MbMode;	// mb_type field
	// per-chroma block information    (8x8)
	int CbWidth, CbHeight, CbPitch;
	int *TotalCoeffC[2];
	// per-transform block information (4x4)
	int TbWidth, TbHeight, TbPitch;	// frame dimensions in number of 4x4 blocks
	int *TotalCoeffL;
	int *Intra4x4PredMode;
	int *MVx,*MVy;
} mode_pred_info;

typedef struct _mb_mode {
  int mb_type;
  int NumMbPart;
  int MbPartPredMode[2];
  int Intra16x16PredMode;
  int MbPartWidth;
  int MbPartHeight;
  int CodedBlockPatternChroma;
  int CodedBlockPatternLuma;
} mb_mode;

typedef struct __frame {
  int Lwidth,Lheight,Lpitch;
  int Cwidth,Cheight,Cpitch;
  unsigned char *L, *C[2];
} frame;

void intraPrediction(frame &f, mode_pred_info &mpi, mb_mode mb, int CurrMbAddr);

// macroblock types for I-slices
#define I_NxN 0			// MbPartPredMode( mb_type, 0 ) equal to Intra_4x4 or Intra_8x8
#define I_16x16_0_0_0 1
#define I_16x16_1_0_0 2
#define I_16x16_2_0_0 3
#define I_16x16_3_0_0 4
#define I_16x16_0_1_0 5
#define I_16x16_1_1_0 6
#define I_16x16_2_1_0 7
#define I_16x16_3_1_0 8
#define I_16x16_0_2_0 9
#define I_16x16_1_2_0 10
#define I_16x16_2_2_0 11
#define I_16x16_3_2_0 12
#define I_16x16_0_0_1 13
#define I_16x16_1_0_1 14
#define I_16x16_2_0_1 15
#define I_16x16_3_0_1 16
#define I_16x16_0_1_1 17
#define I_16x16_1_1_1 18
#define I_16x16_2_1_1 19
#define I_16x16_3_1_1 20
#define I_16x16_0_2_1 21
#define I_16x16_1_2_1 22
#define I_16x16_2_2_1 23
#define I_16x16_3_2_1 24
#define I_PCM 25

// macroblock type prediction modes:
#define Intra_4x4 0
#define Intra_16x16 1