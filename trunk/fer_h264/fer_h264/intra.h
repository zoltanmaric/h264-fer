#pragma once

void intraPrediction(int CurrMbAddr, int *predL, int *predCr, int *predCb);

// macroblock type prediction modes:
#define Intra_4x4 0
#define Intra_16x16 1