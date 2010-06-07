#define NA 511
#define Clip1Y(x) ((x) < 0) ? 0 : (((x) > 255) ? 255 : (x))

//////////////////////////////////////////////////////////
//////// Lookup tables and transform functions ///////////
//////////////////////////////////////////////////////////
constant int intra4x4ScanOrder[16][2]={
  { 0, 0},  { 4, 0},  { 0, 4},  { 4, 4},
  { 8, 0},  {12, 0},  { 8, 4},  {12, 4},
  { 0, 8},  { 4, 8},  { 0,12},  { 4,12},
  { 8, 8},  {12, 8},  { 8,12},  {12,12}
};

constant int4 levelQuantize[6][4] =
{
	{(int4)(205, 158, 205, 158), (int4)(158, 128, 158, 128), (int4)(205, 158, 205, 158), (int4)(158, 128, 158, 128)},
	{(int4)(186, 146, 186, 146), (int4)(146, 114, 146, 114), (int4)(186, 146, 186, 146), (int4)(146, 114, 146, 114)},
	{(int4)(158, 128, 158, 128), (int4)(128, 102, 128, 102), (int4)(158, 128, 158, 128), (int4)(128, 102, 128, 102)},
	{(int4)(146, 114, 146, 114), (int4)(114, 89, 114, 89), (int4)(146, 114, 146, 114), (int4)(114, 89, 114, 89)},
	{(int4)(128, 102, 128, 102), (int4)(102, 82, 102, 82), (int4)(128, 102, 128, 102), (int4)(102, 82, 102, 82)},
	{(int4)(114, 89, 114, 89), (int4)(89, 71, 89, 71), (int4)(114, 89, 114, 89), (int4)(89, 71, 89, 71)}
};

// Because the quantization matrices are symmetrical, it makes
// no difference whether the elements of dv and cv represent
// rows or columns of a matrix.
void quantisationResidualBlock(int qP, int4 dv[4], int4 cv[4])
{
	int qPCalculated = qP / 6;
	int qPMod = qP % 6;
	
	int qbits = 4 - qPCalculated;
	int adjust = 1 << (3 - qP/6);

	for (int i = 0; i < 4; i++)
	{
		cv[i] = (((dv[i] << qbits) - adjust) * levelQuantize[qPMod][i] + 16384) >> 15;
	}
}

// Each element of rv contains one ROW of a matrix.
// Each element of dv contains one COLUMN of a matrix.
void forwardTransform4x4(int4 rv[4], int4 dv[4])
{
	// Y = C'*D'*X*B'*A'
	int4 fv[4], ftv[4], hv[4];
	
	// each element of the vector array hv contains
	// one ROW of the scalar matrix h
	for (int i = 0; i < 4; i++)
	{
		// For vector data types, true returns -1 (all bits set)
		hv[i] = (rv[i] != (int4)0) & ((rv[i] << 6) - 32);
	}
	
	// each element of the vector array fv contains
	// one ROW of the scalar matrix f
	fv[0] = ((hv[0] << 8) + (hv[1] << 8) + (hv[2] << 8) + (hv[3] << 8) + 512) >> 10;
	fv[1] = (((hv[0] << 8) + (hv[0] << 7) + (hv[0] << 5))
			+ ((hv[1] << 7) + (hv[1] << 6) + (hv[1] << 4))
			- ((hv[2] << 7) + (hv[2] << 6) + (hv[2] << 4))
			- ((hv[3] << 8) + (hv[3] << 7) + (hv[3] << 5)) + 512) >> 10;
	fv[2] = ((hv[0] << 8) - (hv[1] << 8) - (hv[2] << 8) + (hv[3] << 8) + 512) >> 10;
	fv[3] = (((hv[0] << 7) + (hv[0] << 6) + (hv[0] << 4))
			- ((hv[1] << 8) + (hv[1] << 7) + (hv[1] << 5))
			+ ((hv[2] << 8) + (hv[2] << 7) + (hv[2] << 5))
			- ((hv[3] << 7) + (hv[3] << 6) + (hv[3] << 4)) + 512) >> 10;
			
	// TRANSPOSITION: each element of the vector array
	// ftv contains one COLUMN of the scalar matrix f
	ftv[0] = (int4)(fv[0].s0, fv[1].s0, fv[2].s0, fv[3].s0);
	ftv[1] = (int4)(fv[0].s1, fv[1].s1, fv[2].s1, fv[3].s1);
	ftv[2] = (int4)(fv[0].s2, fv[1].s2, fv[2].s2, fv[3].s2);
	ftv[3] = (int4)(fv[0].s3, fv[1].s3, fv[2].s3, fv[3].s3);
	
	// each element of the vector array dv contains
	// one COLUMN of the scalar matrix d
	dv[0] = ((ftv[0] << 8) + (ftv[1] << 8) + (ftv[2] << 8) + (ftv[3] << 8) + 512) >> 10;
	dv[1] = (((ftv[0] << 8) + (ftv[0] << 7) + (ftv[0] << 5))
		+ ((ftv[1] << 7) + (ftv[1] << 6) + (ftv[1] << 4))
		- ((ftv[2] << 7) + (ftv[2] << 6) + (ftv[2] << 4))
		- ((ftv[3] << 8) + (ftv[3] << 7) + (ftv[3] << 5)) + 512) >> 10;
	dv[2] = ((ftv[0] << 8) - (ftv[1] << 8) - (ftv[2] << 8) + (ftv[3] << 8) + 512) >> 10;
	dv[3] = (((ftv[0] << 7) + (ftv[0] << 6) + (ftv[0] << 4))
		- ((ftv[1] << 8) + (ftv[1] << 7) + (ftv[1] << 5))
		+ ((ftv[2] << 8) + (ftv[2] << 7) + (ftv[2] << 5))
		- ((ftv[3] << 7) + (ftv[3] << 6) + (ftv[3] << 4)) + 512) >> 10;
}

//////////////////////////////////////////////////////////
///////////////// Intra 16x16 prediction /////////////////
//////////////////////////////////////////////////////////

void fetchPredictionSamples16(int *predSamples, global uchar16 *frame, int frameWidth, int CurrMbAddr)
{
	int frameWidthInMbs = frameWidth >> 4;
	int xF, yF, frameIdx;
	
	int xP = ((CurrMbAddr%frameWidthInMbs)<<4);
	int yP = ((CurrMbAddr/frameWidthInMbs)<<4);
	
	// predSamples[0]:
	xF = (xP - 1) >> 4;
	yF = yP - 1;
	frameIdx = yF*frameWidthInMbs + xF;
	predSamples[0] = ((xF >= 0) && (yF >= 0)) ? frame[frameIdx].sf : NA;
	
	// predSamples[1..16]:
	xF = (xP - 1) >> 4;
	yF = yP;
	frameIdx = yF*frameWidthInMbs + xF;
	for (int i = 1; i < 17; i++)
	{
		predSamples[i] = (xF >= 0) ? frame[frameIdx].sf : NA;
		frameIdx += frameWidthInMbs;
	}
	
	// predSamples[17..32]:
	xF = xP >> 4;
	yF = yP - 1;
	frameIdx = yF*frameWidthInMbs + xF;
	uchar16 temp = frame[frameIdx];
	
	predSamples[17] = (yF >= 0) ? temp.s0 : NA;
	predSamples[18] = (yF >= 0) ? temp.s1 : NA;
	predSamples[19] = (yF >= 0) ? temp.s2 : NA;
	predSamples[20] = (yF >= 0) ? temp.s3 : NA;
	predSamples[21] = (yF >= 0) ? temp.s4 : NA;
	predSamples[22] = (yF >= 0) ? temp.s5 : NA;
	predSamples[23] = (yF >= 0) ? temp.s6 : NA;
	predSamples[24] = (yF >= 0) ? temp.s7 : NA;
	predSamples[25] = (yF >= 0) ? temp.s8 : NA;
	predSamples[26] = (yF >= 0) ? temp.s9 : NA;
	predSamples[27] = (yF >= 0) ? temp.sa : NA;
	predSamples[28] = (yF >= 0) ? temp.sb : NA;
	predSamples[29] = (yF >= 0) ? temp.sc : NA;
	predSamples[30] = (yF >= 0) ? temp.sd : NA;
	predSamples[31] = (yF >= 0) ? temp.se : NA;
	predSamples[32] = (yF >= 0) ? temp.sf : NA;
}

void fetchOriginalMBSamples16(global uchar16 *frame, int frameWidth, int CurrMbAddr, int original[16][16])
{
	int frameWidthInMbs = frameWidth >> 4;

	int xP = ((CurrMbAddr%frameWidthInMbs)<<4);
	int yP = ((CurrMbAddr/frameWidthInMbs)<<4);
	int frameIdx = yP * frameWidthInMbs + (xP >> 4);

	for (int y = 0; y < 16; y++)
	{
		uchar16 temp = frame[frameIdx];
		original[y][0] = temp.s0;
		original[y][1] = temp.s1;
		original[y][2] = temp.s2;
		original[y][3] = temp.s3;
		original[y][4] = temp.s4;
		original[y][5] = temp.s5;
		original[y][6] = temp.s6;
		original[y][7] = temp.s7;
		original[y][8] = temp.s8;
		original[y][9] = temp.s9;
		original[y][10] = temp.sa;
		original[y][11] = temp.sb;
		original[y][12] = temp.sc;
		original[y][13] = temp.sd;
		original[y][14] = temp.se;
		original[y][15] = temp.sf;
		frameIdx += frameWidthInMbs;
	}
}

// (8.3.3.1)
void Intra_16x16_Vertical(int *p, int predL[16][16])
{
	for (int y = 0; y < 16; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			predL[y][x] = p[x+17];
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
			predL[y][x] = p[y+1];
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
		sumXi += p[i+17];
		sumYi += p[i+1];
	}
	
	int result = 128;
	if (p[0] != NA)			// if all available
		result = (sumXi + sumYi + 16) >> 5;
	else if (p[1] != NA) 	// if left available
		result = (sumYi + 8) >> 4;
	else if (p[17] != NA)	// if top available
		result = (sumXi + 8) >> 4;
	
	for (int y = 0; y < 16; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			predL[y][x] = result;
		}
	}
}

// (8.3.3.4)
void Intra_16x16_Plane(int *p, int predL[16][16])
{
	int H = 0, V = 0;
	for (int i = 0; i <= 7; i++)
	{
		H += (i+1)*(p[25+i]- p[(i < 7) ? (23-i) : 0]);
		V += (i+1)*(p[9+i] - p[7-i]);
	}
	
	int a = ((p[16] + p[32]) << 4);
	int b = (5*H + 32) >> 6;
	int c = (5*V + 32) >> 6;
	
	for (int y = 0; y < 16; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			predL[y][x] = Clip1Y((a + b*(x-7) + c*(y-7) + 16) >> 5);
		}
	}
	
	if (p[0] == NA)
	{
		predL[0][0] = NA;
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

uint satd16(int predL[16][16], int original[16][16], int qP)
{
	uint satd = 0;
	
	for (int luma4x4BlkIdx = 0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
	{
		int4 diffL4x4[4];
		int x0 = intra4x4ScanOrder[luma4x4BlkIdx][0];
		int y0 = intra4x4ScanOrder[luma4x4BlkIdx][1];
		for (int i = 0; i < 4; i++)
		{
			diffL4x4[i].s0 = original[y0+i][x0+0] - predL[y0+i][x0+0];
			diffL4x4[i].s1 = original[y0+i][x0+1] - predL[y0+i][x0+1];
			diffL4x4[i].s2 = original[y0+i][x0+2] - predL[y0+i][x0+2];
			diffL4x4[i].s3 = original[y0+i][x0+3] - predL[y0+i][x0+3];
		}

		int4 rLuma[4], dv[4];
		forwardTransform4x4(diffL4x4, dv);
		quantisationResidualBlock(qP, dv, rLuma);

		for (int i = 0; i < 4; i++)
		{
			uint4 temp = abs(rLuma[i]);
			satd += temp.s0 + temp.s1 + temp.s2 + temp.s3;
		}
	}
	
	if (predL[0][0] == NA)
	{
		satd = INT_MAX;
	}

	return satd;
}

kernel void GetIntra16x16PredModes(global uchar16 *frame, int frameWidth, int qP, global int *predModes)
{
	uint CurrMbAddr = get_global_id(0);
	
	int predL[16][16];
	int original[16][16];
	int p[33];
	
	fetchPredictionSamples16(p, frame, frameWidth, CurrMbAddr);
	fetchOriginalMBSamples16(frame, frameWidth, CurrMbAddr, original);
	
	uint min = INT_MAX;
	int chosenPredMode;
	for (int i = 0; i < 4; i++)
	{
		performIntra16x16Prediction(p, predL, i);
		
		int satd = satd16(predL, original, qP);
		
		if (satd < min)
		{
			min = satd;
			chosenPredMode = i;
		}
	}
	
	predModes[CurrMbAddr] = chosenPredMode;
}

//////////////////////////////////////////////////////////
////////////////// Intra 4x4 prediction //////////////////
//////////////////////////////////////////////////////////
#define p(x,y) (((x) == -1) ? p[(y) + 1] : p[(x) + 5])

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
	int result = 128;
	if (p(-1,-1) != NA)		// if all available
		result = (p(0,-1) + p(1,-1) + p(2,-1) + p(3,-1) +
				  p(-1,0) + p(-1,1) + p(-1,2) + p(-1,3) + 4) >> 3;
	else if (p(-1,0) != NA)	// if left available
		result = (p(-1,0) + p(-1,1) + p(-1,2) + p(-1,3) + 2) >> 2;
	else if (p(0,-1) != NA) // if top available
		result = (p(0,-1) + p(1,-1) + p(2,-1) + p(3,-1) + 2) >> 2;
				  
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			pred4x4L[y][x] = result;
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
			pred4x4L[y][x] = (p(x+y,-1) + (p(x+y+1,-1) << 1) + p(x+y+2,-1) + 2) >> 2;
		}
	}	
	pred4x4L[3][3] = (p(6,-1) + 3*p(7,-1) + 2) >> 2;
	
	// pred4x4L[0][0] == NA triggers satd = INT_MAX
	if (p(0,-1) == NA) pred4x4L[0][0] = NA;
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
	
	// pred4x4L[0][0] == NA triggers satd = INT_MAX
	if (p(0,-1) == NA) pred4x4L[0][0] = NA;	
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
	
	// pred4x4L[0][0] == NA triggers satd = INT_MAX
	if (p(0,-1) == NA) pred4x4L[0][0] = NA;	
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
	
	// pred4x4L[0][0] == NA triggers satd = INT_MAX
	if (p(0,-1) == NA) pred4x4L[0][0] = NA;	
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
	
	// pred4x4L[0][0] == NA triggers satd = INT_MAX
	if (p(0,-1) == NA) pred4x4L[0][0] = NA;	
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
	
	// pred4x4L[0][0] == NA triggers satd = INT_MAX
	if (p(-1,0) == NA) pred4x4L[0][0] = NA;	
}

void performIntra4x4Prediction(int intra4x4PredMode, int pred4x4L[4][4], int p[13])
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

void fetchPredictionSamples4(int CurrMbAddr, global uchar4 *frame,
				int frameWidth, int luma4x4BlkIdx, int *p)
{
	int frameWidthInMbs = frameWidth >> 4;
	
	int xP = ((CurrMbAddr%frameWidthInMbs)<<4);
	int yP = ((CurrMbAddr/frameWidthInMbs)<<4);
	
	int x0 = intra4x4ScanOrder[luma4x4BlkIdx][0];
	int y0 = intra4x4ScanOrder[luma4x4BlkIdx][1];
	
	int x = xP + x0;
	int y = yP + y0;
	
	int xF = (x - 1) >> 2;
	int yF = y - 1;
	int frameIdx = yF * (frameWidth>>2) + xF;
	if ((xF < 0) || (yF < 0))
	{
		p[0] = NA;
	}
	else
	{
		p[0] = frame[frameIdx].s3;
	}
	
	xF = (x - 1) >> 2;
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
			frameIdx = yF * (frameWidth>>2) + xF;
			p[i] = frame[frameIdx].s3;
			yF++;
		}
	}
	
	xF = x >> 2;
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
		frameIdx = yF * (frameWidth >> 2) + xF;
		p[5] = frame[frameIdx].s0;
		p[6] = frame[frameIdx].s1;
		p[7] = frame[frameIdx].s2;
		p[8] = frame[frameIdx].s3;
		
		xF = (x + 4) >> 2;
		int edgeSubMB = (xF >= frameWidth >> 2) || ((x0 == 12) && (y0 > 0));
		if ((edgeSubMB == 1) || (luma4x4BlkIdx == 3) || (luma4x4BlkIdx == 11))
		{
			xF = (x + 3) >> 2;
			frameIdx = yF * (frameWidth >> 2) + xF;
			for (int i = 9; i < 13; i++)
			{
				// Copy the rightmost prediction sample to
				// the samples above-right
				p[i] = frame[frameIdx].s3;
			}
		}
		else
		{
			frameIdx = yF * (frameWidth >> 2) + xF;
			p[9] = frame[frameIdx].s0;
			p[10] = frame[frameIdx].s1;
			p[11] = frame[frameIdx].s2;
			p[12] = frame[frameIdx].s3;
		}
	}
}

void fetchOriginalSamples4x4(global uchar4 *frame, int frameWidth, int CurrMbAddr, int luma4x4BlkIdx, int original[4][4])
{
	int frameWidthInMbs = frameWidth >> 4;

	int xP = ((CurrMbAddr%frameWidthInMbs)<<4) + intra4x4ScanOrder[luma4x4BlkIdx][0];
	int yP = ((CurrMbAddr/frameWidthInMbs)<<4) + intra4x4ScanOrder[luma4x4BlkIdx][1];

	for (int y = 0; y < 4; y++)
	{
		int frameIdx = (yP + y) * (frameWidth >> 2) + (xP >> 2);
		original[y][0] = frame[frameIdx].s0;
		original[y][1] = frame[frameIdx].s1;
		original[y][2] = frame[frameIdx].s2;
		original[y][3] = frame[frameIdx].s3;
	}
}

uint satdLuma4x4(int pred4x4L[4][4], int CurrMbAddr, int original[4][4], int qP)
{
	int4 diffL4x4[4];
	for (int i = 0; i < 4; i++)
	{
		diffL4x4[i].s0 = original[i][0] - pred4x4L[i][0];
		diffL4x4[i].s1 = original[i][1] - pred4x4L[i][1];
		diffL4x4[i].s2 = original[i][2] - pred4x4L[i][2];
		diffL4x4[i].s3 = original[i][3] - pred4x4L[i][3];
	}

	int4 rLuma[4], dv[4];
	forwardTransform4x4(diffL4x4, dv);
	quantisationResidualBlock(qP, dv, rLuma);

	uint satd = 0;
	for (int i = 0; i < 4; i++)
	{
		uint4 temp = abs(rLuma[i]);
		satd += temp.s0 + temp.s1 + temp.s2 + temp.s3;
	}
	
	if (pred4x4L[0][0] == NA) satd = INT_MAX;

	return satd;
}

kernel void GetIntra4x4PredModes(global uchar4 *frame, int frameWidth, int qP, global int *predModes4x4)
{
	uint absIdx = get_global_id(0);
	uint CurrMbAddr = absIdx >> 4;
	uint luma4x4BlkIdx = absIdx & 15;
	
	uint min4x4 = INT_MAX;

	int p[13];
	fetchPredictionSamples4(CurrMbAddr, frame, frameWidth, luma4x4BlkIdx, p);
	
	int original[4][4];
	fetchOriginalSamples4x4(frame, frameWidth, CurrMbAddr, luma4x4BlkIdx, original);
	
	int pred4x4L[4][4];
	int chosenPredMode;
	for(int predMode = 0; predMode < 9; predMode++)
	{		
		performIntra4x4Prediction(predMode, pred4x4L, p);
		int satd4x4 = satdLuma4x4(pred4x4L, CurrMbAddr, original, qP);
		if (satd4x4 < min4x4)
		{
			chosenPredMode = predMode;
			min4x4 = satd4x4;			
		}
	}
	
	predModes4x4[absIdx] = chosenPredMode;
}