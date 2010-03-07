#include "mocomp.h"

#include "mode_pred.h"

int L_Temp_4x4_refPart[9][9];
int C_Temp_4x4_refPart[2][3][3];

void FillTemp_4x4_refPart(frame *ref, int org_x, int org_y) {
	int x,y,sx,sy;
	for(y=0; y<9; ++y) {
		sy=org_y+y;
		if(sy<0) sy=0;
		if(sy >= FRAME_Height) sy=FRAME_Height-1;
		for(x=0; x<9; ++x) {
			sx=org_x+x;
			if (sx < 0) sx = 0;
			if (sx >= FRAME_Width) sx = FRAME_Width-1;
			L_Temp_4x4_refPart[y][x]=ref->Luma[sy*FRAME_Width+sx];
		}
	}
	for(y=0; y<3; ++y) {
		sy=org_y/2+y;
		if(sy<0) sy=0;
		if(sy>=FRAME_Height/2) sy=FRAME_Height/2-1;
		for(x=0; x<3; ++x) {
			sx=org_x/2+x;
			if (sx < 0) sx = 0;
			if (sx >= FRAME_Width/2) sx = FRAME_Width/2-1;
			C_Temp_4x4_refPart[0][y][x] = ref->Chroma[0][sy*FRAME_Width+sx];
			C_Temp_4x4_refPart[1][y][x] = ref->Chroma[1][sy*FRAME_Width+sx];
		}
	}
}


#define Tap6Filter(E,F,G,H,I,J) Border(((E)-5*(F)+20*(G)+20*(H)-5*(I)+(J)+16)>>5)
int Border(int i) {
  if(i<0) return 0; else if(i>255) return 255; else return i;
}

#define iffrac(x,y) if(frac==y*4+x)
#define Middle(a,b) (((a)+(b)+1)>>1)

int L_MC_frac_interpol(int *data, int frac) {
#define p(x,y) data[(y)*9+(x)]
  int b,cc,dd,ee,ff,h,j,m,s;
  iffrac(0,0) return p(0,0);
  b=Tap6Filter(p(-2,0),p(-1,0),p(0,0),p(1,0),p(2,0),p(3,0));
  iffrac(1,0) return Middle(p(0,0),b);
  iffrac(2,0) return b;
  iffrac(3,0) return Middle(b,p(1,0));
  h=Tap6Filter(p(0,-2),p(0,-1),p(0,0),p(0,1),p(0,2),p(0,3));
  iffrac(0,1) return Middle(p(0,0),h);
  iffrac(0,2) return h;
  iffrac(0,3) return Middle(h,p(0,1));
  iffrac(1,1) return Middle(b,h);
  m=Tap6Filter(p(1,-2),p(1,-1),p(1,0),p(1,1),p(1,2),p(1,3));
  iffrac(3,1) return Middle(b,m);
  s=Tap6Filter(p(-2,1),p(-1,1),p(0,1),p(1,1),p(2,1),p(3,1));
  iffrac(1,3) return Middle(h,s);
  iffrac(3,3) return Middle(s,m);
  cc=Tap6Filter(p(-2,-2),p(-2,-1),p(-2,0),p(-2,1),p(-2,2),p(-2,3));
  dd=Tap6Filter(p(-1,-2),p(-1,-1),p(-1,0),p(-1,1),p(-1,2),p(-1,3));
  ee=Tap6Filter(p(2,-2),p(2,-1),p(2,0),p(2,1),p(2,2),p(2,3));
  ff=Tap6Filter(p(3,-2),p(3,-1),p(3,0),p(3,1),p(3,2),p(3,3));
  j=Tap6Filter(cc,dd,h,m,ee,ff);
  iffrac(2,2) return j;
  iffrac(2,1) return Middle(b,j);
  iffrac(1,2) return Middle(h,j);
  iffrac(2,3) return Middle(j,s);
  iffrac(3,2) return Middle(j,m);
  return 128;  // some error
#undef p
}

void MotionCompensateSubMBPart(int predL[16][16], int predCr[8][8], int predCb[8][8], frame *refPic,
                        int mbPartIdx,
                        int subMbIdx, 
						int subMbPartIdx) {
	int x,y, org_x,org_y; // org_x and org_y are origin point of 4x4 submbpart in current macroblock
	int mvx, mvy;
	org_x = ((subMbIdx & 2)<<2) + ((subMbPartIdx & 2)<<1);
	org_y = ((subMbIdx & 1)<<3) + ((subMbPartIdx & 1)<<2);
	mvx = MPI_mvSubL0x_byIdx(mbPartIdx,subMbIdx,subMbPartIdx);
	mvy = MPI_mvSubL0y_byIdx(mbPartIdx,subMbIdx,subMbPartIdx);

	// Fills temp tables used in fractional interpolation (luma) and linear interpolation (chroma).
	FillTemp_4x4_refPart(refPic, org_x+(mvx>>2)-2,org_y+(mvy>>2)-2);

	int frac=(mvy&3)*4+(mvx&3);
	for(y=0; y<4; ++y)
		for(x=0; x<4; ++x)
			predL[org_y+y][org_x+x] = L_MC_frac_interpol(&(L_Temp_4x4_refPart[y+2][x+2]), frac);

	org_x>>=1; org_y>>=1; // Chroma resolution is halved luma resolution.
    
	int xLinear=(mvx&7), yLinear=(mvy&7); 
	// CB component - iCbCr=0
	for(y=0; y<2; ++y)
		for(x=0; x<2; ++x)
			predCb[org_y+y][org_x+x] =
			  ((8-xLinear)*(8-yLinear) * C_Temp_4x4_refPart[0][y][x]  +
				  xLinear *(8-yLinear) * C_Temp_4x4_refPart[0][y][x+1]+
			   (8-xLinear)*   yLinear  * C_Temp_4x4_refPart[0][y+1][x]  +
				  xLinear *   yLinear  * C_Temp_4x4_refPart[0][y+1][x+1]+
				32) >> 6; // Linear interpolation and rounding.

	// CR component - iCbCr=1
	for(y=0; y<2; ++y)
		for(x=0; x<2; ++x)
			predCr[org_y+y][org_x+x] =
			  ((8-xLinear)*(8-yLinear) * C_Temp_4x4_refPart[1][y][x]  +
				  xLinear *(8-yLinear) * C_Temp_4x4_refPart[1][y][x+1]+
			   (8-xLinear)*   yLinear  * C_Temp_4x4_refPart[1][y+1][x]  +
				  xLinear *   yLinear  * C_Temp_4x4_refPart[1][y+1][x+1]+
				32) >> 6; // Linear interpolation and rounding.
}

// In baseline profile there is only refPicL0 used. There's no weighted prediction.
// In global structure "MB_pred_info * infos" are placed all information needed for
// decode process (motion compensation).
void Decode(int predL[16][16], int predCr[8][8], int predCb[8][8], frame ** refPicL0, int mbPartIdx)
{
	// Smallest granularity for macroblock is 8x8 subpart (and in that case partition to 4x4 can be used).
	// After DeriveMVs called, all MV are prepared (for every subMB and every subMBPart - granularity to part 4x4 sized).
	frame * refPic = refPicL0[*(refIdxL0+mbPartIdx)];
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			MotionCompensateSubMBPart(predL, predCr, predCb, refPic, mbPartIdx, i, j);
}
