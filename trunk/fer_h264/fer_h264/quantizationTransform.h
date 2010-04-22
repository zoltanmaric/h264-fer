#pragma once

void forwardResidual(int qP, int c[4][4], int r[4][4], bool Intra, bool Intra16x16OrChroma);
void transformScan(int c[4][4], int list[16], bool Intra16x16AC);

void forwardDCLumaIntra(int qP, int dcY[4][4], int c[4][4]);

void forwardDCChroma (int qP, int dcC[2][2], int c[2][2], bool Intra);

void quantizationTransform(int predL[16][16], int predCb[8][8], int predCr[8][8], bool reconstruct);

#define ExtractSign(x) ((x)>>31)
#define CombineSign(sign,value) ((sign)?(-(value)):(value))