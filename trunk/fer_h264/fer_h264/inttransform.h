#pragma once

void transformDecoding4x4LumaResidual(int LumaLevel[16][16], int predL[16][16], int luma4x4BlkIdx, int QPy);
void transformDecodingIntra_16x16Luma(int Intra16x16DCLevel[16], int Intra16x16ACLevel[16][16],int predL[16][16], int QPy);
void transformDecodingP_Skip(int predL[16][16], int predCb[8][8], int predCr[8][8], int QPy);
void transformDecodingChroma(int ChromaDCLevel[4], int ChromaACLevel[4][16], int predC[8][8], int QPy, bool Cb);

extern int qPiToQPc[52];