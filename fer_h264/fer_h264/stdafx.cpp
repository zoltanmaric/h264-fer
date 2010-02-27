// stdafx.cpp : source file that includes just the standard includes
// fer_h264.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

typedef struct {
  int Lwidth,Lheight;
  int Cwidth,Cheight;
  unsigned char *L, *C[2];
} frame_type;

int ***b;

void a()
{
b=new int**[2];
b[0]=new int*[2];
b[0][0]=new int[2];
}

void c(int ***b)
{
b[2][3][3]=1;
}