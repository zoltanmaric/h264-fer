#pragma once

#include <stdio.h>
#include "h264_globals.h"

#define Sign(sign,value) ((sign)?(-(value)):(value))

typedef struct {
	int elements[4][4];
} block;

//used for Chroma DC elements transformation
typedef struct {
	int elements[2][2];
} small_block;

extern int ZigZagReordering[16][2];

block ReorderElements (int *input);
void InverseResidualLuma (int *input, frame_type current, int x, int y, int quantizer, int dc);
void inverseResidual(int bitDepth, int qP, int c[4][4], int r[4][4], bool luma);

void forwardTransform4x4(int input[4][4], int output[4][4]);