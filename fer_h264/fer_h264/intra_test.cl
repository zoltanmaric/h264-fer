#define Clip1Y(x) ((x) < 0) ? 0 : (((x) > 255) ? 255 : (x))
#define NA 0xffff

//////////////////////////////////////////////////////////
//////// Lookup tables and transform functions ///////////
//////////////////////////////////////////////////////////
__constant int intra4x4ScanOrder[16][2]={
  { 0, 0},  { 4, 0},  { 0, 4},  { 4, 4},
  { 8, 0},  {12, 0},  { 8, 4},  {12, 4},
  { 0, 8},  { 4, 8},  { 0,12},  { 4,12},
  { 8, 8},  {12, 8},  { 8,12},  {12,12}
};

__constant int levelQuantize[4][4] = {
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
			int temp = ((d[i][j] << qbits) - adjust) * levelQuantize[i][j];
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


//////////////////////////////////////////////////////////
////////////////// Intra 4x4 prediction //////////////////
//////////////////////////////////////////////////////////
#define p(x,y) (((x) == -1) ? p[(y) + 1] : p[(x) + 5])
__constant int subMBNeighbours[16][2] = {
{ 5, 10}, { 0, 11}, { 7,  0}, { 2,  1},
{ 1, 14}, { 4, 15}, { 3,  4}, { 6,  5},
{13,  2}, { 8,  3}, {15,  8}, {10,  9},
{ 9,  6}, {12,  7}, {11, 12}, {14, 13}};

// (8.3.1.2.1)
void Intra_4x4_Vertical(int *p, int pred4x4L[4][4])
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			pred4x4L[y][x] = p(x,-1);
		}
	}
}

// (8.3.1.2.2)
void Intra_4x4_Horizontal(int *p, int pred4x4L[4][4])
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			pred4x4L[y][x] = p(-1,y);
		}
	}
}

// (8.3.1.2.3)
void Intra_4x4_DC(int *p, int pred4x4L[4][4])
{
	bool allAvailable = true;
	bool leftAvailable = true;
	bool topAvailable = true;

	for (int i = 0; i < 13; i++)
	{
		if (p[i] == NA)
		{
			allAvailable = false;
			if ((i > 0) && (i < 5))
				leftAvailable = false;
			else if ((i >= 5) && (i < 9))
				topAvailable = false;
		}
	}
	
	if (allAvailable)
	{
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				pred4x4L[y][x] = (p(0,-1) + p(1,-1) + p(2,-1) + p(3,-1) +
									   p(-1,0) + p(-1,1) + p(-1,2) + p(-1,3) + 4) >> 3;
			}
		}
	}
	else if (leftAvailable)
	{
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				pred4x4L[y][x] = (p(-1,0) + p(-1,1) + p(-1,2) + p(-1,3) + 2) >> 2;
			}
		}
	}
	else if (topAvailable)
	{
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				pred4x4L[y][x] = (p(0,-1) + p(1,-1) + p(2,-1) + p(3,-1) + 2) >> 2;
			}
		}
	}
	else
	{
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				pred4x4L[y][x] = 128; // = (1 << (BitDepthY - 1); BitDepthY = 8 in baseline
			}
		}
	}
}

// (8.3.1.2.4)
void Intra_4x4_Diagonal_Down_Left(int *p, int pred4x4L[4][4])
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			if ((x == 3) && (y == 3))
				pred4x4L[y][x] = (p(6,-1) + 3*p(7,-1) + 2) >> 2;
			else
				pred4x4L[y][x] = (p(x+y,-1) + (p(x+y+1,-1) << 1) + p(x+y+2,-1) + 2) >> 2;
		}
	}
}

// (8.3.1.2.5)
void Intra_4x4_Diagonal_Down_Right(int *p, int pred4x4L[4][4])
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			if (x > y)
				pred4x4L[y][x] = (p(x-y-2,-1) + (p(x-y-1,-1) << 1) + p(x-y,-1) + 2) >> 2;
			else if (x < y)
				pred4x4L[y][x] = (p(-1,y-x-2) + (p(-1,y-x-1) << 1) + p(-1,y-x) + 2) >> 2;
			else
				pred4x4L[y][x] = (p(0,-1) + (p(-1,-1) << 1) + p(-1,0) + 2) >> 2;
		}
	}
}

// (8.3.1.2.6)
void Intra_4x4_Vertical_Right(int *p, int pred4x4L[4][4])
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			int zVR = (x << 1) - y;
			if ((zVR == 0) || (zVR == 2) ||
				(zVR == 4) || (zVR == 6))
				pred4x4L[y][x] = (p(x-(y>>1)-1,-1) + p(x-(y>>1),-1) + 1) >> 1;
			else if ((zVR == 1) ||
				(zVR == 3) || (zVR == 5))
				pred4x4L[y][x] = (p(x-(y>>1)-2,-1) + (p(x-(y>>1)-1,-1) << 1) + p(x-(y>>1),-1) + 2) >> 2;
			else if (zVR == -1)
				pred4x4L[y][x] = (p(-1,0) + (p(-1,-1) << 1) + p(0,-1) + 2) >> 2;
			else
				pred4x4L[y][x] = (p(-1,y-1) + (p(-1,y-2) << 1) + p(-1,y-3) + 2) >> 2;
		}
	}
}

// (8.3.1.2.7)
void Intra_4x4_Horizontal_Down(int *p, int pred4x4L[4][4])
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			int zHD = (y << 1) - x;
			if ((zHD == 0) || (zHD == 2) ||
				(zHD == 4) || (zHD == 6))
				pred4x4L[y][x] = (p(-1,y-(x>>1)-1) + p(-1,y-(x>>1)) + 1) >> 1;
			else if ((zHD == 1) ||
				(zHD == 3) || (zHD == 5))
				pred4x4L[y][x] = (p(-1,y-(x>>1)-2) + (p(-1,y-(x>>1)-1) << 1) + p(-1,y-(x>>1)) + 2) >> 2;
			else if (zHD == -1)
				pred4x4L[y][x] = (p(-1,0) + (p(-1,-1) << 1) + p(0,-1) + 2) >> 2;
			else
				pred4x4L[y][x] = (p(x-1,-1) + (p(x-2,-1) << 1) + p(x-3,-1) + 2) >> 2;
		}
	}
}

// (8.3.1.2.8)
void Intra_4x4_Vertical_Left(int *p, int pred4x4L[4][4])
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			if ((y == 0) || (y==2))
				pred4x4L[y][x] = (p(x+(y>>1),-1) + p(x+(y>>1)+1,-1) + 1) >> 1;
			else
				pred4x4L[y][x] = (p(x+(y>>1),-1) + (p(x+(y>>1)+1,-1) << 1) + p(x+(y>>1)+2,-1) + 2) >> 2;
		}
	}
}

// (8.3.1.2.9)
void Intra_4x4_Horizontal_Up(int *p, int pred4x4L[4][4])
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			int zHU = x + (y << 1);
			if ((zHU == 0) || (zHU == 2) ||
				(zHU == 4))
				pred4x4L[y][x] = (p(-1,y+(x>>1)) + p(-1,y+(x>>1)+1) + 1) >> 1;
			else if ((zHU == 1) || (zHU == 3))
				pred4x4L[y][x] = (p(-1,y+(x>>1)) + (p(-1,y+(x>>1)+1) << 1) + p(-1,y+(x>>1)+2) + 2) >> 2;
			else if (zHU == 5)
				pred4x4L[y][x] = (p(-1,2) + 3*p(-1,3) + 2) >> 2;
			else
				pred4x4L[y][x] = p(-1,3);
		}
	}
}

void performIntra4x4Prediction(int luma4x4BlkIdx, int intra4x4PredMode, int pred4x4L[4][4], int p[13])
{
	switch (intra4x4PredMode)
	{
		case 0:
			Intra_4x4_Vertical(p, pred4x4L);
			break;
		case 1:
			Intra_4x4_Horizontal(p, pred4x4L);
			break;
		case 2:
			Intra_4x4_DC(p, pred4x4L);
			break;
		case 3:
			Intra_4x4_Diagonal_Down_Left(p, pred4x4L);
			break;
		case 4:
			Intra_4x4_Diagonal_Down_Right(p, pred4x4L);
			break;
		case 5:
			Intra_4x4_Vertical_Right(p, pred4x4L);
			break;
		case 6:
			Intra_4x4_Horizontal_Down(p, pred4x4L);
			break;
		case 7:
			Intra_4x4_Vertical_Left(p, pred4x4L);
			break;
		case 8:
			Intra_4x4_Horizontal_Up(p, pred4x4L);
			break;
	}
}

void fetchPredictionSamples4(int CurrMbAddr, __global int *frame,
				int frameWidth, int luma4x4BlkIdx, int *p)
{
	int frameWidthInMbs = frameWidth >> 4;
	
	int xP = ((CurrMbAddr%frameWidthInMbs)<<4);
	int yP = ((CurrMbAddr/frameWidthInMbs)<<4);
	
	int x0 = intra4x4ScanOrder[luma4x4BlkIdx][0];
	int y0 = intra4x4ScanOrder[luma4x4BlkIdx][1];
	
	int x = xP + x0;
	int y = yP + y0;
	
	int xF = x - 1;
	int yF = y - 1;
	
	if ((xF < 0) || (yF < 0))
	{
		p[0] = NA;
	}
	else
	{
		p[0] = frame[yF * frameWidth + xF];
	}
	
	xF = x - 1;
	yF = y;	
	if (xF < 0)
	{
		for (int i = 1; i < 5; i++)
		{
			p[i] = NA;		// Unavailable for prediction
		}
	}
	else
	{
		for (int i = 1; i < 5; i++)
		{
			p[i] = frame[yF * frameWidth + xF];
			yF++;
		}
	}
	
	xF = x;
	yF = y - 1;
	if (yF < 0)
	{
		for (int i = 5; i < 13; i++)
		{
			// Samples above and above-right marked as unavailable for prediction
			p[i] = NA;
		}
	}
	else
	{
		for (int i = 5; i < 9; i++)
		{
			p[i] = frame[yF * frameWidth + xF];
			xF++;
		}
		
		xF = x + 4;
		yF = y - 1;
		if ((xF > frameWidth) || (luma4x4BlkIdx == 3) || (luma4x4BlkIdx == 11))
		{
			xF = x + 3;
			for (int i = 9; i < 13; i++)
			{
				// Copy the rightmost prediction sample to
				// the samples above-right
				p[i] = frame[yF * frameWidth + xF];
			}
		}
		else
		{
			for (int i = 9; i < 13; i++)
			{
				p[i] = frame[yF * frameWidth + xF];
				xF++;
			}
		}
	}
}

int satdLuma4x4(int pred4x4l[4][4], int luma4x4blkidx, int currmbaddr, int framewidth, __global int *frame)
{
	int framewidthinmbs = framewidth >> 4;

	int xp = ((currmbaddr%framewidthinmbs)<<4);
	int yp = ((currmbaddr/framewidthinmbs)<<4);

	int x0 = intra4x4ScanOrder[luma4x4blkidx][0];
	int y0 = intra4x4ScanOrder[luma4x4blkidx][1];

	int diffl4x4[4][4];
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			diffl4x4[i][j] = frame[(yp+y0+i)*framewidth+(xp+x0+j)] - pred4x4l[i][j];
		}
	}

	int rluma[4][4];
	forwardTransform4x4(diffl4x4, rluma);
	quantisationResidualBlock(rluma, rluma);

	int satd = 0;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			satd += abs(rluma[i][j]);
		}
	}

	return satd;
}

__kernel void
GetIntra4x4PredModes(__global int *frame,
						 int frameWidth,
				__global int *predModes4x4)
{
	uint absIdx = get_global_id(0);
	uint CurrMbAddr = absIdx >> 4;
	uint luma4x4BlkIdx = absIdx & 15;
	
	predModes4x4[absIdx] = 0;

	int min4x4 = 600000000;

	int p[13];
	fetchPredictionSamples4(CurrMbAddr, frame, frameWidth, luma4x4BlkIdx, p);
	
	int luma4x4BlkIdxA, luma4x4BlkIdxB;
	
	int pred4x4L[4][4];
	for(int predMode = 0; predMode < 9; predMode++)
	{		
		performIntra4x4Prediction(luma4x4BlkIdx, predMode, pred4x4L, p);

		int satd4x4 = satdLuma4x4(pred4x4L, luma4x4BlkIdx, CurrMbAddr, frameWidth, frame);
		if (satd4x4 < min4x4)
		{
			predModes4x4[absIdx] = predMode;
			min4x4 = satd4x4;
			if (min4x4 == 0)
			{
				break;
			}					
		}
	}
}
