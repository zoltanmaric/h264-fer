#pragma once

int Clip3(int x, int y, int z);
int Clip1Y(int x);
int Clip1C(int x);

#define roundRightShift(x,n) ((x) >> (n)) + (((x) >> ((n)-1)) & 1)
#define InverseRasterScan(a,b,c,d,e) (((e) == 0) ? ((a) % ((d)/(b))) * (b) : ((a) / ((d)/(b))) * (c))
