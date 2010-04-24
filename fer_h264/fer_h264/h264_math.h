#pragma once

int InverseRasterScan(int a, int b, int c, int d, int e);
int Clip3(int x, int y, int z);
int Clip1Y(int x);
int Clip1C(int x);
#define roundRightShift(x,n) ((x) >> (n)) + (((x) >> ((n)-1)) & 1)
