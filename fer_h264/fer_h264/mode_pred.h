#pragma once

#include "h264_globals.h"
#include "headers_and_parameter_sets.h"

#define MV_NA 0x80808080
#define MB_Width 16
#define MB_Height 16

#define MPI_mb_type(mbAddr) *(mb_type_array+mbAddr)
#define MPI_mvL0x(mbAddr,subMbIdx) *(mvL0x+16*mbAddr+subMbIdx*4)
#define MPI_mvL0y(mbAddr,subMbIdx) *(mvL0x+16*mbAddr+subMbIdx*4)
#define MPI_mvSubL0x_byIdx(mbAddr,subMbIdx,subMbPartIdx) *(mvL0x+16*mbAddr+4*subMbIdx+subMbPartIdx)
#define MPI_mvSubL0y_byIdx(mbAddr,subMbIdx,subMbPartIdx) *(mvL0y+16*mbAddr+4*subMbIdx+subMbPartIdx)
#define MPI_mvCL0x(mbAddr,subMbIdx) *(mvL0x+16*mbAddr+subMbIdx*4)
#define MPI_mvCL0y(mbAddr,subMbIdx) *(mvL0y+16*mbAddr+subMbIdx*4)
#define MPI_subMvCnt(mbAddr) *(subMvCnt+mbAddr)
#define MPI_refIdxL0(mbAddr) *(refIdxL0+mbAddr)

//int mvL0x[Frame_Width][Frame_Height], mvL0y[Frame_Height][Frame_Height];
extern int mvL0x[], mvL0y[];
//int subMvCnt[Frame_Width/MB_Width][Frame_Height/MB_Height], refIdxL0[Frame_Height/MB_Width][Frame_Height/MB_Height];
extern int subMvCnt[], refIdxL0[];

// in following methods global "MB_pred_info * infos" is used, so it has to be initialized before calling any of these methods
bool get_neighbour_mv(int org_x, int org_y, int mbPartIdx, int curr_refIdxL0, int * mvNx, int * mvNy, int * refIdxL0N);
void PredictMV_Luma(int org_x, int org_y, int mbPartIdx);
void PredictMV(int org_x, int org_y);
void DeriveMVs();
