
__kernel void 
absDiff(__global uchar16 *a,
	__global uchar16 *b,
	__global uchar16 *answer)
{
	int gid = get_global_id(0);
	answer[gid] = abs_diff(a[gid],b[gid]);
}

__kernel void
FillRefFrameKar(__global int *refFrameKar,
				__global int *refFrameInterpolatedL,
				int frameHeight,
				int frameWidth)
{
	int gid = get_global_id(0);
	int i, j, suma[5];
	int x, y, tx, ty, trenutna;
	int frameSize = (frameHeight+8) * (frameWidth+8);
	
	for (j = 0; j < 16; j++)
	{
		x = gid % (frameWidth+8);
		y = gid / (frameWidth+8);
		for (i = 0; i < 5; i++)
		{
			suma[i] = 0x0f0f0f0f;
		}
		if (x < frameWidth && y < frameHeight)
		{
			for (i = 0; i < 5; i++)
			{
				suma[i] = 0;
			}
			for (tx = x; tx < x+8; tx++)
				for (ty = y; ty < y+8; ty++)
				{
					if (tx >= frameWidth && ty >= frameHeight)
					{
						trenutna = refFrameInterpolatedL[(j+1) * frameHeight * frameWidth - 1];
					} else if (tx >= frameWidth)
					{
						trenutna = refFrameInterpolatedL[j * frameHeight * frameWidth + (ty+1)*frameWidth-1];
					} else if (ty >= frameHeight)
					{
						trenutna = refFrameInterpolatedL[j * frameHeight * frameWidth + (frameHeight-1)*frameWidth+tx];
					} else 
					{
						trenutna = refFrameInterpolatedL[j * frameHeight * frameWidth + ty*frameWidth+tx];
					}
					suma[0] += trenutna;
					if (ty < 4) suma[1] += trenutna;
					if (tx < 4) suma[2] += trenutna;
					if ((ty&3)<2) suma[3] += trenutna;
					if ((tx&3)<2) suma[4] += trenutna;
				}
		} 
		for (i = 0; i < 5; i++)
		{
			refFrameKar[(i*16+j)*frameSize + gid] = suma[i];
		}
	}
}



__kernel void
IntraPrediction(__global int *frameLuma,
				__global int *predModes)
{
	
}