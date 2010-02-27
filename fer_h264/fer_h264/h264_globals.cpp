#include "headers_and_parameter_sets.h"
#include "residual.h"
#include "h264_globals.h"

//Inter prediction slices - Macroblock types
//Defined strictly by norm, page 121.
//(Table 7-13 – Macroblock type values 0 to 4 for P and SP slices)
/*
First column:	mb_type
Second column:	Name of mb_type
Third column:	NumMbPart( mb_type )
Fourth column:	MbPartPredMode( mb_type, 0 )
Fifth column:	MbPartPredMode( mb_type, 1 )
Sixth column:	MbPartWidth( mb_type )
Seventh column:	MbPartHeight( mb_type )
*/
int P_and_SP_macroblock_modes[6][7]=
{
  {0,	P_L0_16x16,		1, Pred_L0, NA,      16, 16},
  {1,	P_L0_L0_16x8,	2, Pred_L0, Pred_L0, 16,  8},
  {2,	P_L0_L0_8x16,	2, Pred_L0, Pred_L0,  8, 16},
  {3,	P_8x8,			4, NA,      NA,       8,  8},
  {4,	P_8x8ref0,		4, NA,      NA,       8,  8},
  {NA,	P_Skip,			1, Pred_L0, NA,      16, 16}
};


//The table for B slices is not implemented.
//B slices are not supported.


//Intra prediction slices - Macroblock types
//Defined strictly by norm, page 119.
//(Table 7-11 – Macroblock types for I slices)
/*
First column:	mb_type
Second column:	Name of mb_type
Third column:	tranform_size_8x8_flag
Fourth column:	MbPartPredMode( mb_type, 0 )
Fifth column:	Intra16x16PredMode
Sixth column:	CodedBlockPatternChroma
Seventh column:	CodedBlockPatternLuma
*/

int I_Macroblock_Modes[27][7]=
{
  {0,	I_4x4,			0,	Intra_4x4,   NA, NA, NA},
  //If this line was to be commented out, the MbPartPredMode macro would have to be changed
  //since it relies on the linear rise of the first column.
  //{0,	I_NxN,			1,	Intra_8x8,	 NA, NA, NA},
  {1,	I_16x16_0_0_0,	NA,	Intra_16x16,  0,  0,  0},
  {2,	I_16x16_1_0_0,	NA,	Intra_16x16,  1,  0,  0},
  {3,	I_16x16_2_0_0,	NA,	Intra_16x16,  2,  0,  0},
  {4,	I_16x16_3_0_0,	NA,	Intra_16x16,  3,  0,  0},
  {5,	I_16x16_0_1_0,	NA,	Intra_16x16,  0,  1,  0},
  {6,	I_16x16_1_1_0,	NA,	Intra_16x16,  1,  1,  0},
  {7,	I_16x16_2_1_0,	NA,	Intra_16x16,  2,  1,  0},
  {8,	I_16x16_3_1_0,	NA,	Intra_16x16,  3,  1,  0},
  {9,	I_16x16_0_2_0,	NA,	Intra_16x16,  0,  2,  0},
  {10,	I_16x16_1_2_0,	NA,	Intra_16x16,  1,  2,  0},
  {11,	I_16x16_2_2_0,	NA,	Intra_16x16,  2,  2,  0},
  {12,	I_16x16_3_2_0,	NA,	Intra_16x16,  3,  2,  0},
  {13,	I_16x16_0_0_1,	NA,	Intra_16x16,  0,  0, 15},
  {14,	I_16x16_1_0_1,	NA,	Intra_16x16,  1,  0, 15},
  {15,	I_16x16_2_0_1,	NA,	Intra_16x16,  2,  0, 15},
  {16,	I_16x16_3_0_1,	NA,	Intra_16x16,  3,  0, 15},
  {17,	I_16x16_0_1_1,	NA,	Intra_16x16,  0,  1, 15},
  {18,	I_16x16_1_1_1,	NA,	Intra_16x16,  1,  1, 15},
  {19,	I_16x16_2_1_1,	NA,	Intra_16x16,  2,  1, 15},
  {20,	I_16x16_3_1_1,	NA,	Intra_16x16,  3,  1, 15},
  {21,	I_16x16_0_2_1,	NA,	Intra_16x16,  0,  2, 15},
  {22,	I_16x16_1_2_1,	NA,	Intra_16x16,  1,  2, 15},
  {23,	I_16x16_2_2_1,	NA,	Intra_16x16,  2,  2, 15},
  {24,	I_16x16_3_2_1,	NA,	Intra_16x16,  3,  2, 15},
  {25,	I_PCM,			NA,	NA,			  NA, NA, NA}
};


//Inter prediction data for submacroblocks
//Defined strictly by the norm
//(Table 7-17 – Sub-macroblock types in P macroblocks)
/*
First column:	sub_mb_type[ mbPartIdx ]
Second coumn:	Name of sub_mb_type[ mbPartIdx ]
Third column:	NumSubMbPart( sub_mb_type[ mbPartIdx ] )
Fourth column:	SubMbPredMode( sub_mb_type[ mbPartIdx ] )
Fifth column:	SubMbPartWidth( sub_mb_type[ mbPartIdx ] )
Sixth column:	SubMbPartHeight( sub_mb_type[ mbPartIdx ] )
*/

int P_sub_macroblock_modes[5][6]=
{
	{NA,	NA,					NA,	NA,		 NA,	NA},
	{0,		P_L0_8x8,			1,	Pred_L0, 8,		8},
	{1,		P_L0_8x4,			2,	Pred_L0, 8,		4},
	{2,		P_L0_4x8,			3,	Pred_L0, 4,		8},
	{3,		P_L0_4x4,			4,	Pred_L0, 4,		4}
};

//Mapping of "codencum" to the coded_block_pattern to the 
//Table 9-4 – Assignment of codeNum to values of coded_block_pattern for macroblock prediction modes
//When ChromaArrayType is equal to 1 or 2
//(We only support "ChromaArrayType=1")

int codeNum_to_coded_block_pattern_intra[48]=
{
  47,31,15, 0,23,27,29,30, 7,11,13,14,39,43,45,46,
  16, 3, 5,10,12,19,21,26,28,35,37,42,44, 1, 2, 4,
   8,17,18,20,24, 6, 9,22,25,32,33,34,36,40,38,41
};

int codeNum_to_coded_block_pattern_inter[48]=
{
   0,16, 1, 2, 4, 8,32, 3, 5,10,12,15,47, 7,11,13,
  14, 6, 9,31,35,37,42,44,33,34,36,40,39,43,45,46,
  17,18,20,24,19,21,26,28,23,27,29,30,22,25,38,41
};

int CodedBlockPatternLuma;
int CodedBlockPatternChroma;

/* TODO: These variables are currently unmanaged
*/

int transform_block_width;
int colour_block_width;

int mb_type;

//mb_type values for all the macroblocks in the current frame/slice/NAL unit
int *mb_type_array;

//Picture/Frame dimensions in macroblocks (divided by 16 in both dimensions)
int PicWidthInMbs, PicHeightInMbs;

//Transform block = 4x4 luma block (same as previos, but divided by 4)
int PicWidthInTbs, PicHeightInTbs;

//Colour block = Cb/Cr block (same as previous, but divided by 8)
int PicWidthInCbs, PicHeightInCbs;

//Various intra prediction globals
//(Used in decoding the rbsp as well.)
int rem_intra4x4_pred_mode[16];
bool prev_intra4x4_pred_mode_flag[16];
int intra_chroma_pred_mode;

int Intra4x4ScanOrder[16][2]={
  { 0, 0},  { 4, 0},  { 0, 4},  { 4, 4},
  { 8, 0},  {12, 0},  { 8, 4},  {12, 4},
  { 0, 8},  { 4, 8},  { 0,12},  { 4,12},
  { 8, 8},  {12, 8},  { 8,12},  {12,12}
};

int ABS(int a)
{
	if (a>0)
		return a;
	else
		return -a;
}

int init_h264_structures()
{
	TotalCoeff_luma_array = new int*[sps.PicWidthInMbs];
	for (int i=0;i<sps.PicWidthInMbs;i++)
	{
		TotalCoeff_luma_array[i] = new int [sps.FrameHeightInMbs];
	}

	TotalCoeff_chroma_array = new int**[2];
	for (int cbcr=0;cbcr<2;cbcr++)
	{
		TotalCoeff_chroma_array[cbcr] = new int* [sps.FrameHeightInMbs];

		for (int i=0;i<sps.FrameHeightInMbs;i++)
		{
				TotalCoeff_chroma_array[cbcr][i] = new int [sps.PicWidthInMbs];
		}
	}

	mb_type_array=new int[sps.PicWidthInMbs*sps.FrameHeightInMbs];

	init_cavlc_tables();

	//TODO: init frame

	//Everything is ok
	return 0;
}

frame_type frame;

//frame/4 dimenzije
int *Intra4x4PredMode;