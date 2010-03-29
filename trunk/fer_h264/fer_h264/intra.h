#pragma once

void intraPrediction(int predL[16][16], int predCr[8][8], int predCb[8][8]);

int intraPredictionEncoding(int predL[16][16], int predCr[8][8], int predCb[8][8]);

// macroblock type prediction modes:
#define Intra_4x4 0
#define Intra_16x16 1