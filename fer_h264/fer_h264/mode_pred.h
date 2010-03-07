#pragma once

#define FRAME_Width 720
#define FRAME_Height 576
#define MV_NA 0x80808080
#define MB_Width 16
#define MB_Height 16
#define FR_MB_in_row FRAME_Width/MB_Width
#define FR_numRows FRAME_Height/MB_Height

#define MPI_mb_type(org_x,org_y) *(mb_type+((org_y/MB_Height)*FR_MB_in_row+org_x/MB_Width))
#define MPI_mvL0x(org_x,org_y,subMbIdx) *(mvL0x+16*((org_y/MB_Height)*FR_MB_in_row+org_x/MB_Width)+subMbIdx*4)
#define MPI_mvL0y(org_x,org_y,subMbIdx) *(mvL0y+16*((org_y/MB_Height)*FR_MB_in_row+org_x/MB_Width)+subMbIdx*4)
#define MPI_mvSubL0x_byIdx(mbPartIdx,subMbIdx,subMbPartIdx) *(mvL0x+16*mbPartIdx+4*subMbIdx+subMbPartIdx)
#define MPI_mvSubL0y_byIdx(mbPartIdx,subMbIdx,subMbPartIdx) *(mvL0y+16*mbPartIdx+4*subMbIdx+subMbPartIdx)
#define MPI_mvSubL0x(org_x,org_y,subMbIdx,subMbPartIdx) *(mvL0x+16*((org_y/MB_Height)*FR_MB_in_row+org_x/MB_Width)+subMbIdx*4+subMbPartIdx)
#define MPI_mvSubL0y(org_x,org_y,subMbIdx,subMbPartIdx) *(mvL0y+16*((org_y/MB_Height)*FR_MB_in_row+org_x/MB_Width)+subMbIdx*4+subMbPartIdx)
#define MPI_mvCL0x(org_x,org_y,subMbIdx) *(mvL0x+16*((org_y/MB_Height)*FR_MB_in_row+org_x/MB_Width)+subMbIdx*4)
#define MPI_mvCL0y(org_x,org_y,subMbIdx) *(mvL0y+16*((org_y/MB_Height)*FR_MB_in_row+org_x/MB_Width)+subMbIdx*4)
#define MPI_subMvCnt(org_x,org_y) *(subMvCnt+((org_y/MB_Height)*FR_MB_in_row+org_x/MB_Width))
#define MPI_refIdxL0(org_x,org_y) *(refIdxL0+((org_y/MB_Height)*FR_MB_in_row+org_x/MB_Width))

// in following methods global "MB_pred_info * infos" is used, so it has to be initialized before calling any of these methods
bool get_neighbour_mv(int org_x, int org_y, int mbPartIdx, int curr_refIdxL0, int * mvNx, int * mvNy, int * refIdxL0N);
void PredictMV_Luma(int org_x, int org_y, int mbPartIdx);
void PredictMV(int org_x, int org_y);
void DeriveMVs(int org_x, int org_y, int mvdx, int mvdy);