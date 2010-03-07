#pragma once

#include "mode_pred.h"

typedef struct __frame {
  unsigned char *Luma, *Chroma[2];
} frame;

void Decode(int predL[16][16], int predCr[8][8], int predCb[8][8], frame * refPicL0, int mbPartIdx);
