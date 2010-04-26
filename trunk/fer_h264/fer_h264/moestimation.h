#pragma once
#include "h264_globals.h"

void interEncoding(int predL[16][16], int predCr[8][8], int predCb[8][8]);

void InitializeInterpolatedRefFrame();

void FillInterpolatedRefFrame();

extern frame_type refFrameInterpolated[16];