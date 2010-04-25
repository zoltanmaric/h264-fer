#pragma once

void RBSP_encode(NALunit &nal_unit);

unsigned int coded_mb_size(int intra16x16PredMode, int predL[16][16], int predCb[8][8], int predCr[8][8]);