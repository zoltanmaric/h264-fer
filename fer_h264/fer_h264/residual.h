#pragma once

void derivation_process_for_4x4_chroma_block_indices(int x, int y, int *chroma4x4BlkIdx);

//Coefficient levels
extern int i16x16DClevel[16];
extern int i16x16AClevel[16][16];
extern int Intra16x16DCLevel[16];
extern int Intra16x16ACLevel[16][16];
extern int level[16][16], LumaLevel[16][16];

void clear_residual_structures();
void inverse_4x4_luma_block_scanning_process(int luma4x4BlkIdx, int *x, int *y);

extern int ChromaDCLevel[2][4];
extern int ChromaACLevel[2][4][16];

extern int luma4x4BlkIdx, cb4x4BlkIdx;
extern int iCbCr;

extern int i8x8, i4x4;

extern int NumC8x8;

//TODO: Niz tipa mb_width*mb_height*16 integera NETOCNO, izrazeni u transform/colour blokovima
//Number of non-zero coefficients in luma and chroma blocks (integer from 0 to 16)
extern int **TotalCoeff_luma_array;
extern int **TotalCoeff_chroma_array[2];

int get_nC(int x, int y, int luma_or_select_chroma);

void residual(int startIdx, int endIdx);
void residual_luma(int i16x16DClevel[16], int i16x16AClevel[16][16], int level[16][16], int startIdx, int endIdx);
void residual_block_cavlc(int ChromaDCLevel[16], int, int, int);

void residual_write();
void residual_luma_write(int i16x16DClevel[16], int i16x16AClevel[16][16], int level[16][16], int startIdx, int endIdx);
void residual_block_cavlc_write(int coeffLevel[16], int startIdx, int endIdx, int maxNumCoeff);

unsigned int residual_block_cavlc_size(int coeffLevel[16], int startIdx, int endIdx, int maxNumCoeff);