#pragma once

// TEST INSERT


typedef struct _mode_pred_info {
  // per-macroblock information     (16x16)
  int MbWidth, MbHeight, MbPitch;
  int *MbMode;
  // per-chroma block information    (8x8)
  int CbWidth, CbHeight, CbPitch;
  int *TotalCoeffC[2];
  // per-transform block information (4x4)
  int TbWidth, TbHeight, TbPitch;
  int *TotalCoeffL;
  int *Intra4x4PredMode;
  int *MVx,*MVy;
} mode_pred_info;



#define ModePredInfo_MbMode(mpi,x,y) (mpi->MbMode[(y)*mpi->MbPitch+(x)])
#define ModePredInfo_TotalCoeffC(mpi,x,y,iCbCr) (mpi->TotalCoeffC[iCbCr][(y)*mpi->CbPitch+(x)])
#define ModePredInfo_TotalCoeffL(mpi,x,y) (mpi->TotalCoeffL[(y)*mpi->TbPitch+(x)])
#define ModePredInfo_Intra4x4PredMode(mpi,x,y) (mpi->Intra4x4PredMode[(y)*mpi->TbPitch+(x)])
#define ModePredInfo_MVx(mpi,x,y) (mpi->MVx[(y)*mpi->TbPitch+(x)])
#define ModePredInfo_MVy(mpi,x,y) (mpi->MVy[(y)*mpi->TbPitch+(x)])


// some macros for easier access to the various ModePredInfo structures
#define LumaDC_nC     get_luma_nC(mpi,(mb_pos_x*16),(mb_pos_y*16))
#define LumaAC_nC     get_luma_nC(mpi,(mb_pos_x*16)+Intra4x4ScanOrder[i8x8*4+i4x4][0],(mb_pos_y*16)+Intra4x4ScanOrder[i8x8*4+i4x4][1])
#define ChromaDC_nC   -1
#define ChromaAC_nC   get_chroma_nC(mpi,(mb_pos_x*16)+(i4x4&1)*8,(mb_pos_y*16)+(i4x4>>1)*8,iCbCr)
#define LumaAdjust    ModePredInfo_TotalCoeffL(mpi,((mb_pos_x*16)+Intra4x4ScanOrder[i8x8*4+i4x4][0])>>2,((mb_pos_y*16)+Intra4x4ScanOrder[i8x8*4+i4x4][1])>>2) =
#define ChromaAdjust  ModePredInfo_TotalCoeffC(mpi,((mb_pos_x*16)+(i4x4&1)*8)>>3,((mb_pos_y*16)+(i4x4>>1)*8)>>3,iCbCr) =



int get_luma_nC(mode_pred_info *mpi, int x, int y);
int get_chroma_nC(mode_pred_info *mpi, int x, int y, int iCbCr);

extern int LumaDCLevel[16];
extern int LumaACLevel[16][16];
extern int ChromaDCLevelX[2][4];
extern int ChromaACLevelX[2][4][16];

extern mode_pred_info *mpi;

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

#define NA				-1

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

#define P_Skip       0xFF

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

extern int P_and_SP_macroblock_modes[6][7];
extern int I_Macroblock_Modes[27][7];
extern int P_sub_macroblock_modes[5][6];
extern int codeNum_to_coded_block_pattern_intra[48];
extern int codeNum_to_coded_block_pattern_inter[48];

extern int CodedBlockPatternLuma;
extern int CodedBlockPatternChroma;

/* TODO: These variables are currently unmanaged
*/

extern int transform_block_width;
extern int colour_block_width;

//mb_type for current macroblock
extern int mb_type;

//mb_type values for all the macroblocks in the current frame/slice/NAL unit
extern int mb_type_array[100000];

//Picture/Frame dimensions in macroblocks (divided by 16 in both dimensions)
extern int PicWidthInMbs, PicHeightInMbs;

//Transform block = 4x4 luma block (same as previos, but divided by 4)
extern int PicWidthInTbs, PicHeightInTbs;

//Colour block = Cb/Cr block (same as previous, but divided by 8)
extern int PicWidthInCbs, PicHeightInCbs;

//Macro functions to make the "slice_decode" part of the program look more like the norm :)

//"x" can be only "0" or "1".
#define MbPartPredMode(mb_type, x)				(((shd.slice_type%5)==P_SLICE)?P_and_SP_macroblock_modes[mb_type][3+x]:I_Macroblock_Modes[mb_type][3])
#define NumMbPart(mb_type)						P_and_SP_macroblock_modes[mb_type][2]
#define Intra16x16PredMode						I_Macroblock_Modes[mb_type][4]
												
//Offset +1 at sub_mb_type because the first line in the table is "special". TODO
#define SubMbPredMode(sub_mb_type)				P_sub_macroblock_modes[sub_mb_type+1][3]
#define NumSubMbPart(sub_mb_type)				P_sub_macroblock_modes[sub_mb_type+1][2]


//Various intra prediction globals
//(Used in decoding the rbsp as well.)
extern int rem_intra4x4_pred_mode[16];
extern bool prev_intra4x4_pred_mode_flag[16];
extern int intra_chroma_pred_mode;

extern int Intra4x4ScanOrder[16][2];

//Various helping functions

int ABS(int a);

//Initalize per-movie data 
int init_h264_structures();

//INIT!
typedef struct {
  int Lwidth,Lheight;
  int Cwidth,Cheight;
  unsigned char *L, *C[2];
} frame_type;

extern frame_type frame;

//frame/4 dimenzije
extern int *Intra4x4PredMode;

extern	int mb_pos_x;
extern	int mb_pos_y;

extern int mb_qp_delta;
extern int QPy;

extern int CurrMbAddr;

extern int to_4x4_luma_block[16];

extern int mb_pos_array[100000];

extern int totalcoeff_array_luma[1000000][16];
extern int totalcoeff_array_chroma[2][1000000][4];

extern int invoked_for_Intra16x16DCLevel, invoked_for_Intra16x16ACLevel, invoked_for_LumaLevel, invoked_for_ChromaACLevel, invoked_for_ChromaDCLevel;