#pragma once

typedef struct __frame {
  int Lwidth,Lheight,Lpitch;
  int Cwidth,Cheight,Cpitch;
  unsigned char *L, *C[2];
} frame;

// (Figure 8-9a)
int ZigZagReordering[16][2] = 
{
	{0,0}, {0,1}, {1,0}, {2,0}, {1,1}, {0,2}, {0,3}, {1,2},
	{2,1}, {3,0}, {3,1}, {2,2}, {1,3}, {2,3}, {3,2}, {3,3}
};

// Specification of QPc as a function of qPi (Table 8-15)
int qPiToQPc[52] = { 0,  1,  2,  3,  4,  5,  6,  7, 
					 8,  9, 10, 11,	12, 13, 14, 15,
					16, 17, 18, 19, 20, 21, 22, 23,
					24, 25, 26, 27, 28, 29, 29, 30,
					31, 32, 32, 33, 34, 34, 35, 35,
					36, 36, 37, 37, 37, 38, 38, 38,
					39, 39, 39, 39};

// globalne varijable koje oèekujem od tebe, ljubo
int mb_qp_delta;
int chroma_qp_index_offset;
frame f;