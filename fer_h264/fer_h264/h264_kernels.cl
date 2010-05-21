
__kernel void 
absDiff(__global uchar16 *a,
	__global uchar16 *b,
	__global uchar16 *answer)
{
	int gid = get_global_id(0);
	answer[gid] = abs_diff(a[gid],b[gid]);
}

// The array frameLuma contains 16 luma samples
// in one (vector) element.
__kernel void
fetchPredictionSamples16(__global uchar16 *frameLuma,
	uint frameWidth,
	__global int *predSamples)
{
	// xF and yF help addressing the vector elements
	// within frameLuma which contain the prediction
	// sample to be fetched. They DO NOT address
	// the exact sample.
	int xF, yF;
	uint frameWidthInMbs = frameWidth >> 4;
	uint numPredSamples = 33;
	
	int mbAddrA, mbAddrB, mbAddrC;
	
	uint gid = get_global_id(0);
	uint lid = gid % numPredSamples;
	uint CurrMbAddr = gid / numPredSamples;
	int frameIdx;
	
	if ((CurrMbAddr % frameWidthInMbs) == 0)
	{
		mbAddrA = -1;
	}
	else
	{
		mbAddrA = CurrMbAddr - 1;
	}
	if (CurrMbAddr < frameWidthInMbs)
	{
		mbAddrB = -1;
	}
	else
	{
		mbAddrB = CurrMbAddr - frameWidthInMbs;
	}
	
	if (lid == 0)
	{
		if ((mbAddrA == -1) || (mbAddrB == -1))
		{
			predSamples[gid] = -1;
		}
		else
		{
			mbAddrC = mbAddrB - 1;
			xF = (mbAddrC % frameWidthInMbs);
			yF = ((mbAddrC / frameWidthInMbs) << 4) + 15;
			frameIdx = yF*frameWidthInMbs + xF;
			predSamples[gid] = frameLuma[frameIdx].sF;
		}
	}
	else if (lid < 17)
	{
		if (mbAddrA == -1)
		{
			predSamples[gid] = -1;
		}
		else
		{
			xF = (mbAddrA % frameWidthInMbs);
			yF = ((mbAddrA / frameWidthInMbs) << 4) + (lid-1);
			frameIdx = yF*frameWidthInMbs + xF;
			predSamples[gid] = frameLuma[frameIdx].sF;
		}
	}
	else
	{
		if (mbAddrB == -1)
		{
			predSamples[gid] = -1;
		}
		else
		{
			xF = (mbAddrB % frameWidthInMbs);
			yF = ((mbAddrB / frameWidthInMbs) << 4) + 15;
			frameIdx = yF*frameWidthInMbs + xF;
			switch(lid)
			{
				case 17:
					predSamples[gid] = frameLuma[frameIdx].s0;
					break;
				case 18:
					predSamples[gid] = frameLuma[frameIdx].s1;
					break;
				case 19:
					predSamples[gid] = frameLuma[frameIdx].s2;
					break;
				case 20:
					predSamples[gid] = frameLuma[frameIdx].s3;
					break;
				case 21:
					predSamples[gid] = frameLuma[frameIdx].s4;
					break;
				case 22:
					predSamples[gid] = frameLuma[frameIdx].s5;
					break;
				case 23:
					predSamples[gid] = frameLuma[frameIdx].s6;
					break;
				case 24:
					predSamples[gid] = frameLuma[frameIdx].s7;
					break;
				case 25:
					predSamples[gid] = frameLuma[frameIdx].s8;
					break;
				case 26:
					predSamples[gid] = frameLuma[frameIdx].s9;
					break;
				case 27:
					predSamples[gid] = frameLuma[frameIdx].sA;
					break;
				case 28:
					predSamples[gid] = frameLuma[frameIdx].sB;
					break;
				case 29:
					predSamples[gid] = frameLuma[frameIdx].sC;
					break;
				case 30:
					predSamples[gid] = frameLuma[frameIdx].sD;
					break;
				case 31:
					predSamples[gid] = frameLuma[frameIdx].sE;
					break;
				case 32:
					predSamples[gid] = frameLuma[frameIdx].sF;
					break;
			}
		}
	}
}

__kernel void
FillRefFrameKar(__global int *refFrameKar,
				__global char *refFrameInterpolatedL,
				int frameSize)
{
	int gid = get_global_id(0);
	int i, j;
	
	for (i = 0; i < 6; i++)
	{
		for (j = 0; j < 16; j++)
		{
			refFrameKar[(i*16+j)*frameSize + gid] = gid;
		}
	}
}//