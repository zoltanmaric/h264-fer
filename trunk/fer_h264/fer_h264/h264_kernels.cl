
__kernel void 
absDiff(__global uchar16 *a,
	__global uchar16 *b,
	__global uchar16 *answer)
{
	int gid = get_global_id(0);
	answer[gid] = abs_diff(a[gid],b[gid]);
}

// Global work size: size of input buffer / 4
kernel void
CharToInt(__global unsigned int *input,
		  __global int *output)
{
	int gid = get_global_id(0);
	for (int i = 0; i < 4; i++)
	{
		int oid = (gid * 4) + i;
		output[oid] = (input[gid] >> (8 * i)) & 0xff;
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

__constant int Intra4x4ScanOrder[16][2]={
  { 0, 0},  { 4, 0},  { 0, 4},  { 4, 4},
  { 8, 0},  {12, 0},  { 8, 4},  {12, 4},
  { 0, 8},  { 4, 8},  { 0,12},  { 4,12},
  { 8, 8},  {12, 8},  { 8,12},  {12,12}
};

__constant int LevelQuantize[4][4] = {
{205, 158, 205, 158},
{158, 128, 158, 128},
{205, 158, 205, 158},
{158, 128, 158, 128}};

void quantisationResidualBlock(int d[4][4], int c[4][4])
{
	int qbits = 2;
	int adjust = 2;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			int temp = ((d[i][j] << qbits) - adjust) * LevelQuantize[i][j];
			c[i][j] = (temp >> 15) + ((temp >> 14) & 1);
		}
	}
}

void forwardTransform4x4(int r[4][4], int d[4][4])
{
	// Y = C'*D'*X*B'*A'

	int i, j;
	int f[4][4], h[4][4];
	int temp[4];

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			h[i][j] = (r[i][j] == 0) ? 0 : ((r[i][j] << 6) - 32);
		}
	}

	// (C'*D')*X
	for (j = 0; j < 4; j++)
	{
		temp[0] = (h[0][j] << 8) + (h[1][j] << 8) + (h[2][j] << 8) + (h[3][j] << 8);
		f[0][j] = (temp[0] >> 10) + ((temp[0] >> 9) & 1);

		temp[1] = ((h[0][j] << 8) + (f[0][j] << 7) + (h[0][j] << 5))
			+ ((h[1][j] << 7) + (h[1][j] << 6) + (h[1][j] << 4))
			- ((h[2][j] << 7) + (h[2][j] << 6) + (h[2][j] << 4))
			- ((h[3][j] << 8) + (h[3][j] << 7) + (h[3][j] << 5));
		f[1][j] = (temp[1] >> 10) + ((temp[1] >> 9) & 1);

		temp[2] = (h[0][j] << 8) - (h[1][j] << 8) - (h[2][j] << 8) + (h[3][j] << 8);
		f[2][j] = (temp[2] >> 10) + ((temp[2] >> 9) & 1);

		temp[3] = ((h[0][j] << 7) + (h[0][j] << 6) + (h[0][j] << 4))
			- ((h[1][j] << 8) + (h[1][j] << 7) + (h[1][j] << 5))
			+ ((h[2][j] << 8) + (h[2][j] << 7) + (h[2][j] << 5))
			- ((h[3][j] << 7) + (h[3][j] << 6) + (h[3][j] << 4));
		f[3][j] = (temp[3] >> 10) + ((temp[3] >> 9) & 1);
	}

	// (C'*D'*X)*(B'*A')
	for (int i = 0; i < 4; i++)
	{

		temp[0] = (f[i][0] << 8) + (f[i][1] << 8) + (f[i][2] << 8) + (f[i][3] << 8);
		d[i][0] = (temp[0] >> 10) + ((temp[0] >> 9) & 1);

		temp[1] = ((f[i][0] << 8) + (f[i][0] << 7) + (f[i][0] << 5))
			+ ((f[i][1] << 7) + (f[i][1] << 6) + (f[i][1] << 4))
			- ((f[i][2] << 7) + (f[i][2] << 6) + (f[i][2] << 4))
			- ((f[i][3] << 8) + (f[i][3] << 7) + (f[i][3] << 5));
		d[i][1] = (temp[1] >> 10) + ((temp[1] >> 9) & 1);

		temp[2] = (f[i][0] << 8) - (f[i][1] << 8) - (f[i][2] << 8) + (f[i][3] << 8);
		d[i][2] = (temp[2] >> 10) + ((temp[2] >> 9) & 1);

		temp[3] = ((f[i][0] << 7) + (f[i][0] << 6) + (f[i][0] << 4))
			- ((f[i][1] << 8) + (f[i][1] << 7) + (f[i][1] << 5))
			+ ((f[i][2] << 8) + (f[i][2] << 7) + (f[i][2] << 5))
			- ((f[i][3] << 7) + (f[i][3] << 6) + (f[i][3] << 4));
		d[i][3] = (temp[3] >> 10) + ((temp[3] >> 9) & 1);
	}
}

int satd16(int predL[16][16],
			  __global int *frame,
			  int frameWidth,
			  int CurrMbAddr)
{
	int satd = 0;
	
	int frameWidthInMbs = frameWidth >> 4;

	int xP = ((CurrMbAddr%frameWidthInMbs)<<4);
	int yP = ((CurrMbAddr/frameWidthInMbs)<<4);
	
	for (int i = 0; i < 16; i++)
	{
		int diffL4x4[4][4];
		int x0 = Intra4x4ScanOrder[i][0];
		int y0 = Intra4x4ScanOrder[i][1];
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				diffL4x4[i][j] = frame[(yP+y0+i)*frameWidth+(xP+x0+j)] - predL[y0+i][x0+j];
			}
		}

		int rLuma[4][4];
		forwardTransform4x4(diffL4x4, rLuma);
		quantisationResidualBlock(rLuma, rLuma);

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				satd += abs(rLuma[i][j]);
			}
		}
	}

	return satd;
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
	FetchPredictionSamplesIntra16x16(p, frame, frameWidth, CurrMbAddr);
	
	int mbAddrA, mbAddrB;
	getNeighbouringMacroblocks(CurrMbAddr, frameWidthInMbs, &mbAddrA, &mbAddrB);
	
	int min = 1000000;
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

		int satd = satd16(predL, frame, frameWidth, CurrMbAddr);
		if (satd < min)
		{
			min = satd;
			predModes[CurrMbAddr] = i;
		}
	}
}