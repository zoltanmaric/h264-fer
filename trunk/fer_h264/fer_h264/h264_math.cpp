// This file contains some helper functions defined in chapter 5.7 (Mathematical functions) in the standard.
#include "h264_math.h"

int Clip3(int x, int y, int z)
{
	if (z < x)
		return x;
	else if (z > y)
		return y;
	else
		return z;
}

int Clip1Y(int x)
{
	// Clip3(0, 255, x); 255 == (1 << BitDepthY) - 1; BitDepthY == 8 when baseline
	if (x < 0)
		return 0;
	if (x > 255)
		return 255;

	return x;
}

int Clip1C(int x)
{
	// Clip3(0, 255, x); 255 == (1 << BitDepthC) - 1; BitDepthC == 8 when baseline
	if (x < 0)
		return 0;
	if (x > 255)
		return 255;

	return x;
}
