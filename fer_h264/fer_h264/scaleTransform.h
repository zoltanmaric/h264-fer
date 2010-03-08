#pragma once

#include <stdio.h>
#include "h264_globals.h"

extern int ZigZagReordering[16][2];

void transformInverseScan(int list[16], int c[4][4]);
void inverseResidual(int bitDepth, int qP, int c[4][4], int r[4][4], bool intra16x16OrChroma);
void InverseDCLumaIntra (int bitDepth, int qP, int c[4][4], int dcY[4][4]);
void InverseDCChroma (int bitDepth, int qP, int c[2][2], int dcC[2][2]);
