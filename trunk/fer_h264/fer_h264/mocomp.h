#pragma once

#include "h264_globals.h"

void Decode(int predL[16][16], int predCr[8][8], int predCb[8][8]);

void MotionCompensateSubMBPart(int predL[16][16], int predCr[8][8], int predCb[8][8], frame_type *refPic,
                        int mbPartIdx,
                        int subMbIdx, 
						int subMbPartIdx);

extern int PR_WIDTH;