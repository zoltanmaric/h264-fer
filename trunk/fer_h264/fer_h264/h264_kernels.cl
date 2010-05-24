
__kernel void 
absDiff(__global uchar16 *a,
	__global uchar16 *b,
	__global uchar16 *answer)
{
	int gid = get_global_id(0);
	answer[gid] = abs_diff(a[gid],b[gid]);
}

kernel void
CharToInt(__global unsigned int *input,
		  unsigned int size,
		  __global int *output)
{
	int gid = get_global_id(0);
	for (int i = 0; i < 4; i++)
	{
		int oid = (gid * 4) + i;
		output[oid] = (input[i] >> (8 * i)) & 0xff;
	}
}

__kernel void
FillRefFrameKar(__global int *refFrameKar,
				__global unsigned int *refFrameInterpolatedL,
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

void
FetchPredictionSamplesIntra16x16(int *predSamples,
			__global int *frame,
			int frameWidth,
			int CurrMbAddr)
{
	int frameWidthInMbs = frameWidth >> 4;
	int mbAddrA, mbAddrB, mbAddrC;
	
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
	
	// predSamples[0]:
	if ((mbAddrA == -1) || (mbAddrB == -1))
	{
		predSamples[0] = -1;
	}
	else
	{
		mbAddrC = mbAddrB - 1;
		int xF = ((mbAddrC % frameWidthInMbs) << 4) + 15;
		int yF = ((mbAddrC / frameWidthInMbs) << 4) + 15;
		int frameIdx = yF*frameWidth + xF;
		predSamples[0] = frame[frameIdx];
	}
	
	for (int i = 1; i < 17; i++)
	{
		if (mbAddrA == -1)
		{
			predSamples[i] = -1;
		}
		else
		{
			int xF = ((mbAddrA % frameWidthInMbs) << 4) + 15;
			int yF = ((mbAddrA / frameWidthInMbs) << 4) + (i-1);
			int frameIdx = yF*frameWidth + xF;
			predSamples[i] = frame[frameIdx];
		}
	}
	for (int i = 17; i < 33; i++)
	{
		if (mbAddrB == -1)
		{
			predSamples[i] = -1;
		}
		else
		{
			int xF = ((mbAddrB % frameWidthInMbs) << 4) + (i - 17);
			int yF = ((mbAddrB / frameWidthInMbs) << 4) + 15;
			int frameIdx = yF*frameWidth + xF;
			predSamples[i] = frame[frameIdx];
		}
	}
}

#define p(x,y) (((x) == -1) ? p[(y) + 1] : p[(x) + 17])
#define Clip1Y(x) ((x) < 0) ? 0 : (((x) > 255) ? 255 : (x))

// (8.3.3.1)
void Intra_16x16_Vertical(int *p, int predL[16][16])
{
	for (int y = 0; y < 16; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			predL[y][x] = p(x,-1);
		}
	}
}

// (8.3.3.2)
void Intra_16x16_Horizontal(int *p, int predL[16][16])
{
	for (int y = 0; y < 16; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			predL[y][x] = p(-1,y);
		}
	}
}

// (8.3.3.3)
void Intra_16x16_DC(int *p, int predL[16][16])
{
	int sumXi = 0;		// = sum(p[x',-1]) | x € (0..15)
	int sumYi = 0;		// = sum(p[-1,y']) | y € (0..15)
	for (int i = 0; i < 16; i++)
	{
		sumXi += p(i,-1);
		sumYi += p(-1,i);
	}

	// check availability of neighbouring samples:
	int allAvailable = 1;
	int leftAvailable = 1;
	int topAvailable = 1;
	for (int i = 0; i < 33; i++)
	{
		if (p[i] == -1)
		{
			allAvailable = 0;
			if ((i > 0) && (i < 17))
				leftAvailable = 0;
			else if ((i >= 17) && (i < 33))
				topAvailable = 0;
		}
	}

	for (int y = 0; y < 16; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			if (allAvailable)
			{
				predL[y][x] = (sumXi + sumYi + 16) >> 5;
			}
			else if (leftAvailable)
			{
				predL[y][x] = (sumYi + 8) >> 4;
			}
			else if (topAvailable)
			{
				predL[y][x] = (sumXi + 8) >> 4;
			}
			else
			{
				predL[y][x] = 1 << 7;		// == 1 << (BitDepthY-1) (BitDepthY is always equal to 8 in the baseline profile)
			}
		}
	}
}

// (8.3.3.4)
void Intra_16x16_Plane(int *p, int predL[16][16])
{
	int H = 0, V = 0;
	for (int i = 0; i <= 7; i++)
	{
		H += (i+1)*(p(8+i,-1) - p(6-i,-1));
		V += (i+1)*(p(-1,8+i) - p(-1,6-i));
	}

	int a = ((p(-1,15) + p(15,-1)) << 4);
	int b = (5*H + 32) >> 6;
	int c = (5*V + 32) >> 6;

	for (int y = 0; y < 16; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			predL[y][x] = Clip1Y((a + b*(x-7) + c*(y-7) + 16) >> 5);
		}
	}
}

void performIntra16x16Prediction(int *p, int predL[16][16], int Intra16x16PredMode)
{
	switch(Intra16x16PredMode)
	{
		case 0:
			Intra_16x16_Vertical(p, predL);
			break;
		case 1:
			Intra_16x16_Horizontal(p, predL);
			break;
		case 2:
			Intra_16x16_DC(p, predL);
			break;
		case 3:
			Intra_16x16_Plane(p, predL);
			break;				
	}
}

int getSad(__global int *frame,
		int CurrMbAddr,
		int frameWidth,
		int predL[16][16])
{
	int sad = 0;
	int frameWidthInMbs = frameWidth >> 4;
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			int xF = ((CurrMbAddr % frameWidthInMbs) << 4) + j;
			int yF = ((CurrMbAddr / frameWidthInMbs) << 4) + i;
			sad += abs_diff(predL[i][j], frame[yF*frameWidth + xF]);
		}
	}
	
	return sad;
}

void getNeighbouringMacroblocks(int CurrMbAddr, int frameWidthInMbs,
								int mbAddrA[1], int mbAddrB[1])
{
	if ((CurrMbAddr % frameWidthInMbs) == 0)
	{
		mbAddrA[0] = -1;
	}
	else
	{
		mbAddrA[0] = CurrMbAddr - 1;
	}
	if (CurrMbAddr < frameWidthInMbs)
	{
		mbAddrB[0] = -1;
	}
	else
	{
		mbAddrB[0] = CurrMbAddr - frameWidthInMbs;
	}
}

__kernel void
IntraPrediction(__global int *frame,
						 int frameWidth,
				__global int *predModes)
{
	uint frameWidthInMbs = frameWidth >> 4;
	uint CurrMbAddr = get_global_id(0);
	
	int predL[16][16];
	int p[33];
	FetchPredictionSamplesIntra16x16(p, frame, frameWidth, CurrMbAddr)	;
	
	int mbAddrA, mbAddrB;
	getNeighbouringMacroblocks(CurrMbAddr, frameWidthInMbs, &mbAddrA, &mbAddrB);
	
	int min = 65280;
	// 16x16 prediction:
	for (int i = 0; i < 4; i++)
	{
		// Skip prediction if required neighbouring macroblocks are not available
		if (((i == 0) && (mbAddrB == -1)) ||
			((i == 1) && (mbAddrA == -1)) ||
			((i == 3) && ((mbAddrA == -1) || (mbAddrB == -1))))
		{
			continue;
		}
		performIntra16x16Prediction(p, predL, i);

		int sad = getSad(frame, CurrMbAddr, frameWidth, predL);
		if (sad < min)
		{
			min = sad;
			predModes[CurrMbAddr] = i;
		}
	}
}