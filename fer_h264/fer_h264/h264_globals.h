#pragma once

extern int LumaDCLevel[16];
extern int LumaACLevel[16][16];
extern int ChromaDCLevelX[2][4];
extern int ChromaACLevelX[2][4][16];

//Constants used for "get_nC" function
#define LUMA 0
#define CHROMA 1
#define CB 1
#define CR 2

#define MbWidthC 8
#define MbHeightC 8

#define SubWidthC 2
#define SubHeightC 2

//TODO:

#define NA				0xff

#define P_L0_16x16      0
#define P_L0_L0_16x8    1
#define P_L0_L0_8x16    2
#define P_8x8           3
#define P_8x8ref0       4

// macroblock types for I-slices
#define I_4x4		  0			// MbPartPredMode( mb_type, 0 ) equal to Intra_4x4 or Intra_8x8
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

#define P_Skip		   31

#define P_L0_8x8       0
#define P_L0_8x4       1
#define P_L0_4x8       2
#define P_L0_4x4       3
#define B_Direct_8x8   4



#define Intra_4x4       0
#define Intra_16x16     1
#define Pred_L0         2
#define Pred_L1         3
#define BiPred          4
#define Direct          5
#define B_Direct_8x8 4

#define	P_SLICE 0
#define B_SLICE 1
#define I_SLICE 2
#define SP_SLICE 3
#define SI_SLICE 4
	
#define NAL_UNIT_TYPE_NOT_IDR 1
#define NAL_UNIT_TYPE_IDR 5
#define NAL_UNIT_TYPE_SEI 6
#define NAL_UNIT_TYPE_SPS 7
#define NAL_UNIT_TYPE_PPS 8

extern int number_of_mmc_operations;

extern int P_and_SP_macroblock_modes[32][7];
extern int I_Macroblock_Modes[27][7];
extern int P_sub_macroblock_modes[5][6];
extern int codeNum_to_coded_block_pattern_intra[48];
extern int codeNum_to_coded_block_pattern_inter[48];

extern int coded_block_pattern_to_codeNum_intra[48];
extern int coded_block_pattern_to_codeNum_inter[48];

extern int CodedBlockPatternLuma;
extern int CodedBlockPatternChroma;

extern int *CodedBlockPatternLumaArray;
extern int *CodedBlockPatternChromaArray;

//mb_type for current macroblock
extern int mb_type;

//mb_type values for all the macroblocks in the current frame/slice/NAL unit
extern int mb_type_array[10000];

//Picture/Frame dimensions in macroblocks (divided by 16 in both dimensions)
extern int PicWidthInMbs, PicHeightInMbs;

//Transform block = 4x4 luma block (same as previos, but divided by 4)
extern int PicWidthInTbs, PicHeightInTbs;

//Colour block = Cb/Cr block (same as previous, but divided by 8)
extern int PicWidthInCbs, PicHeightInCbs;

//Macro functions to make the "slice_decode" part of the program look more like the norm :)

//"x" can be only "0" or "1".
#define MbPartPredMode(mb_type, x)				(((shd.slice_type%5)==P_SLICE)?P_and_SP_macroblock_modes[mb_type][3+(x%2)]:I_Macroblock_Modes[mb_type][3])
#define NumMbPart(mb_type)						P_and_SP_macroblock_modes[mb_type][2]
												
//Offset +1 at sub_mb_type because the first line in the table is "special". TODO
#define SubMbPredMode(sub_mb_type)				P_sub_macroblock_modes[sub_mb_type+1][3]
#define NumSubMbPart(sub_mb_type)				P_sub_macroblock_modes[sub_mb_type+1][2]


//Various intra prediction globals
//(Used in decoding the rbsp as well.)
extern int rem_intra4x4_pred_mode[16];
extern bool prev_intra4x4_pred_mode_flag[16];
extern int intra_chroma_pred_mode;

extern int Intra4x4ScanOrder[16][2];

// Inter prediction globals
extern int **ref_idx_l0_array;
extern int mvd_l0[4][4][2];

//Various helping functions

#define ABS(a) (((a) < 0) ? -(a) : (a))

//Initalize per-movie data 
int init_h264_structures();
int init_h264_structures_encoder();

//INIT!
typedef struct {
  int Lwidth,Lheight;
  int Cwidth,Cheight;
  unsigned char *L, *C[2];
} frame_type;

extern frame_type frame;
// The absolute frame count starting with 1
extern unsigned long frameCount;

//frame/4 dimenzije
extern int *Intra4x4PredMode;

extern	int mb_pos_x;
extern	int mb_pos_y;

extern int brojTipova[5];	
extern int vrijeme;
extern int BasicInterEncoding;
extern int WindowSize;
extern int MAXDIFF_SET;
extern int startFrame;
extern int endFrame;
extern int IntraEvery;
extern int currFrameCount;

extern int mb_qp_delta;
extern int QPy;

extern int CurrMbAddr;

extern int to_4x4_luma_block[16];

extern int mb_pos_array[10000];

extern int totalcoeff_array_luma[10000][16];
extern int totalcoeff_array_chroma[2][10000][4];

extern int invoked_for_Intra16x16DCLevel, invoked_for_Intra16x16ACLevel, invoked_for_LumaLevel, invoked_for_ChromaACLevel, invoked_for_ChromaDCLevel;
extern bool OpenCLEnabled;

extern int _qParameter;