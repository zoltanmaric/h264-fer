#pragma once
//
//typedef struct __frame {
//  int Lwidth,Lheight,Lpitch;
//  int Cwidth,Cheight,Cpitch;
//  unsigned char *L, *C[2];
//} frame;

void transformDecoding4x4LumaResidual(int LumaLevel[16][16], int predL[16][16], int *QPy_prev, int CurrMbAddr);
void transformDecodingIntra_16x16Luma(int Intra16x16DCLevel[16], int Intra16x16ACLevel[16][16],int predL[16][16], int *QPy_prev, int CurrMbAddr);