#pragma once

void forwardResidual(int bitDepth, int qP, int c[4][4], int r[4][4], bool luma, bool Intra);

void forwardDCLumaIntra(int qP, int dcY[4][4], int c[4][4]);

void forwardDCChroma (int qP, int dcC[2][2], int c[2][2], bool Intra);

#define ExtractSign(x) ((x)>>31)
#define CombineSign(sign,value) ((sign)?(-(value)):(value))