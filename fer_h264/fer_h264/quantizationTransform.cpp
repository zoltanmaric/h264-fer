#include "scaleTransform.h"
#include "quantizationTransform.h"
#include "residual.h"
#include "h264_globals.h"
#include "h264_math.h"
#include "headers_and_parameter_sets.h"
#include "inttransform.h"

//Stubs for compilation
int x0C, y0C, x0;

//Multiplication factors for quantization
int MF[6][4][4] = 
{
	{{13107,8066,13107,8066},{8066,5243,8066,5243},{13107,8066,13107,8066},{8066,5243,8066,5243}},
	{{11916,7490,11916,7490},{7490,4660,7490,4660},{11916,7490,11916,7490},{7490,4660,7490,4660}},
	{{10082,6554,10082,6554},{6554,4194,6554,4194},{10082,6554,10082,6554},{6554,4194,6554,4194}},
	{{9362,5825, 9352,5825},{5825,3647,5825,3647},{9362,5825, 9352,5825},{5825,3647,5825,3647}},
	{{8192,5243, 8192,5243},{5243,3355,5243,3355},{8192,5243, 8192,5243},{5243,3355,5243,3355}},
	{{7282,4559, 7282,4559},{4559,2893,4559,2893},{7282,4559, 7282,4559},{4559,2893,4559,2893}}
};

// Quantization factors based on level scale factor from decoder (round(reciprocal>>15))
int LevelQuantize[6][4][4] =
{
	{{205, 158, 205, 158}, {158, 128, 158, 128}, {205, 158, 205, 158}, {158, 128, 158, 128}},
	{{186, 146, 186, 146}, {146, 114, 146, 114}, {186, 146, 186, 146}, {146, 114, 146, 114}},
	{{158, 128, 158, 128}, {128, 102, 128, 102}, {158, 128, 158, 128}, {128, 102, 128, 102}},
	{{146, 114, 146, 114}, {114, 89, 114, 89}, {146, 114, 146, 114}, {114, 89, 114, 89}},
	{{128, 102, 128, 102}, {102, 82, 102, 82}, {128, 102, 128, 102}, {102, 82, 102, 82}},
	{{114, 89, 114, 89}, {89, 71, 89, 71}, {114, 89, 114, 89}, {89, 71, 89, 71}}
};

int Sign (int number)
{
	if (number < 0) return (number * (-1));
	else return number;
}
// --------------------------------------------------
// Integer transformation process for residual blocks
void forwardTransform4x4(int r[4][4], int d[4][4])
{
	// Y = C'*D'*X*B'*A'

	int i, j;
	int f[4][4], h[4][4];
	int temp;

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
		temp = (h[0][j] << 8) + (h[1][j] << 8) + (h[2][j] << 8) + (h[3][j] << 8);
		f[0][j] = (temp + 512) >> 10;

		temp = ((h[0][j] << 8) + (f[0][j] << 7) + (h[0][j] << 5))
			+ ((h[1][j] << 7) + (h[1][j] << 6) + (h[1][j] << 4))
			- ((h[2][j] << 7) + (h[2][j] << 6) + (h[2][j] << 4))
			- ((h[3][j] << 8) + (h[3][j] << 7) + (h[3][j] << 5));
		f[1][j] = (temp + 512) >> 10;

		temp = (h[0][j] << 8) - (h[1][j] << 8) - (h[2][j] << 8) + (h[3][j] << 8);
		f[2][j] = (temp + 512) >> 10;

		temp = ((h[0][j] << 7) + (h[0][j] << 6) + (h[0][j] << 4))
			- ((h[1][j] << 8) + (h[1][j] << 7) + (h[1][j] << 5))
			+ ((h[2][j] << 8) + (h[2][j] << 7) + (h[2][j] << 5))
			- ((h[3][j] << 7) + (h[3][j] << 6) + (h[3][j] << 4));
		f[3][j] = (temp + 512) >> 10;
	}

	// (C'*D'*X)*(B'*A')
	for (int i = 0; i < 4; i++)
	{
		temp = (f[i][0] << 8) + (f[i][1] << 8) + (f[i][2] << 8) + (f[i][3] << 8);
		d[i][0] = (temp + 512) >> 10;

		temp = ((f[i][0] << 8) + (f[i][0] << 7) + (f[i][0] << 5))
			+ ((f[i][1] << 7) + (f[i][1] << 6) + (f[i][1] << 4))
			- ((f[i][2] << 7) + (f[i][2] << 6) + (f[i][2] << 4))
			- ((f[i][3] << 8) + (f[i][3] << 7) + (f[i][3] << 5));
		d[i][1] = (temp + 512) >> 10;

		temp = (f[i][0] << 8) - (f[i][1] << 8) - (f[i][2] << 8) + (f[i][3] << 8);
		d[i][2] = (temp + 512) >> 10;

		temp = ((f[i][0] << 7) + (f[i][0] << 6) + (f[i][0] << 4))
			- ((f[i][1] << 8) + (f[i][1] << 7) + (f[i][1] << 5))
			+ ((f[i][2] << 8) + (f[i][2] << 7) + (f[i][2] << 5))
			- ((f[i][3] << 7) + (f[i][3] << 6) + (f[i][3] << 4));
		d[i][3] = (temp + 512) >> 10;
	}
}


// --------------------------------------------------
// Integer transformation process for DC Luma Intra coefficients
void forwardTransformDCLumaIntra(int f[4][4], int c[4][4]) //(int input[4][4], int output[4][4])
{
	int i, j;
	int temp[4];

	int d[4][4], e[4][4], g[4][4];

	// D'*X
	for (j = 0; j < 4; j++)
	{
		g[0][j] = f[0][j] + f[3][j];
		g[1][j] = f[1][j] + f[2][j];
		g[2][j] = f[1][j] - f[2][j];
		g[3][j] = f[0][j] - f[3][j];
	}

	// C'*(D'*X)
	for (j = 0; j < 4; j++)
	{
		e[0][j] = g[0][j] + g[1][j];
		e[1][j] = g[3][j] + g[2][j];
		e[2][j] = g[0][j] - g[1][j];
		e[3][j] = g[3][j] - g[2][j];
	}

	// (C'*D'*X)*B'
	for (i = 0; i < 4; i++)
	{
		d[i][0] = e[i][0] + e[i][3];
		d[i][1] = e[i][1] + e[i][2];
		d[i][2] = e[i][1] - e[i][2];
		d[i][3] = e[i][0] - e[i][3];
	}
	
	// (C'*D'*X*B')*A'
	for (i = 0; i < 4; i++)
	{
		temp[0] = d[i][0] + d[i][1];
		temp[1] = d[i][3] + d[i][2];
		temp[2] = d[i][0] - d[i][1];
		temp[3] = d[i][3] - d[i][2];

		c[i][0] = (temp[0] + 8) >> 4;
		c[i][1] = (temp[1] + 8) >> 4;
		c[i][2] = (temp[2] + 8) >> 4;
		c[i][3] = (temp[3] + 8) >> 4;
	}
}


// --------------------------------------------------
// Integer transformation process for Chroma DC coefficients
void forwardTransformDCChroma (int f[2][2], int c[2][2])
{
	int d[2][2];
	int temp;

	d[0][0] = f[0][0] + f[0][1];
	d[0][1] = f[0][0] - f[0][1];
	d[1][0] = f[1][0] + f[1][1];
	d[1][1] = f[1][0] - f[1][1];

	temp = d[0][0] + d[1][0];
	c[0][0] = (temp + 2) >> 2;

	temp = d[0][1] + d[1][1];
	c[0][1] = (temp + 2) >> 2;

	temp = d[0][0] - d[1][0];
	c[1][0] = (temp + 2) >> 2;

	temp = d[0][1] - d[1][1];
	c[1][1] = (temp + 2) >> 2;
}


// --------------------------------------------------
// Quantization process for residual blocks
void quantisationResidualBlock(int d[4][4], int c[4][4], int qP, bool Intra, bool Intra16x16OrChroma)
{
	int qPCalculated = qP / 6;	
	int qbits;
	int qPMod = qP % 6;
	int temp;

	if (qP < 24)
	{

		qbits = 4 - qPCalculated;
		int adjust = 1 << (3 - qP/6);

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				temp = ((d[i][j] << qbits) - adjust) * LevelQuantize[qPMod][i][j];
				c[i][j] = (temp + 16384) >> 15;
			}
		}
	}
	else
	{
		qbits = qPCalculated - 4;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				temp = (d[i][j] >> qbits) * LevelQuantize[qPMod][i][j];
				c[i][j] = (temp + 16384) >> 15;
			}
		}
	}

	if (Intra16x16OrChroma == true)
	{
		// Do not quantize DC.
		c[0][0] = d[0][0];
	}
}

// --------------------------------------------------
// Quantization process for DC Luma Intra coefficients
void quantisationLumaDCIntra (int f[4][4], int qP, int c[4][4])
{
	int qP_calculate = qP/6;
	int quantizeL = LevelQuantize[qP%6][0][0];
	int qbits;
	int temp;
	
	if (qP >= 36)
	{
		qbits = qP_calculate - 6;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				temp = (f[i][j] >> qbits) * quantizeL;
				c[i][j] = (temp + 16384) >> 15;
			}
		}
	}
	else
	{
		int adjust = 1 << (5 - qP_calculate);
		qbits = 6 - qP_calculate;

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				temp = ((f[i][j] << qbits) - adjust) * quantizeL;
				c[i][j] = (temp + 16384) >> 15;
			}
		}
	}
}

// --------------------------------------------------
// Quantization process for DC Chroma coefficients
void quantisationChromaDC(int f[2][2], int qP, int c[2][2], bool Intra)
{
	int qbits = 15 + qP/6;
	int qPMod = qP % 6;
	//int f_adjust;	
	int temp;

	int qP_calculate = qP/6;
	int quantizeL = LevelQuantize[qPMod][0][0];

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			temp = ((f[i][j] << 5) >> qP_calculate) * quantizeL;
			c[i][j] = (temp + 16384) >> 15;
		}
	}
}

void forwardResidual(int qP, int c[4][4], int r[4][4], bool Intra, bool Intra16x16OrChroma)
{
	int d[4][4];

	forwardTransform4x4(c, d);
	quantisationResidualBlock(d, r, qP, Intra, Intra16x16OrChroma);

}

void forwardDCLumaIntra(int qP, int dcY[4][4], int c[4][4])
{
	int f[4][4];

	forwardTransformDCLumaIntra(dcY, f);
	quantisationLumaDCIntra(f, qP, c);

}

void forwardDCChroma (int qP, int dcC[2][2], int c[2][2], bool Intra)
{
	int f[2][2];

	forwardTransformDCChroma(dcC, f);
	quantisationChromaDC(f, qP, c, Intra);
}

void transformScan(int c[4][4], int list[16], bool Intra16x16AC)
{
	if (Intra16x16AC)
	{
		for (int i = 1; i < 16; i++)
		{
			int y = ZigZagReordering[i][0];
			int x = ZigZagReordering[i][1];
			list[i-1] = c[y][x];
		}		
	}
	else {
		for (int i = 0; i < 16; i++)
		{
			int y = ZigZagReordering[i][0];
			int x = ZigZagReordering[i][1];
			list[i] = c[y][x];
		}
	}
}

void scanChroma(int rChroma[4][4], int list[16])
{
	for (int i = 1; i < 16; i++)
	{
		int y = ZigZagReordering[i][0];
		int x = ZigZagReordering[i][1];
		list[i-1] = rChroma[y][x];
	}
}

void scanDCChroma(int rDCChroma[2][2], int list[4])
{
	for (int i = 0; i < 4; i++)
	{
		list[i] = rDCChroma[i/2][i%2];
	}
}

void quantizationTransform(int predL[16][16], int predCb[8][8], int predCr[8][8], bool reconstruct)
{
	int diffL4x4[4][4];						// unprocessed luma residual
	int DCLuma[4][4];						// DC luma coefficients for Intra16x16 prediction
	int rLuma[4][4];						// processed luma residual
	int rDCLuma[4][4];						// processed DC luma coefficients
	int diffCb4x4[4][4], diffCr4x4[4][4];	// unprocessed chroma residuals
	int rCb[4][4], rCr[4][4];				// processed chroma residuals
	int DCCb[2][2], DCCr[2][2];				// DC chroma coefficients
	int rDCCb[2][2], rDCCr[2][2];			// processed DC chroma coefficients

	int reconstructedBlock[4][4];
	int reconstructedLuma[16][16];
	int reconstructedDCY[4][4];

	int qP;									// quantizer

	// Luma transform and quantization process

	int xP = ((CurrMbAddr%PicWidthInMbs)<<4);
	int yP = ((CurrMbAddr/PicWidthInMbs)<<4);

	qP = QPy;

	// The transformation and the picture reconstruction process
	// for Intra_4x4 prediction modes has been performed within
	// the prediction process.
	if (MbPartPredMode(mb_type,0) != Intra_4x4)
	{
		for (int luma4x4BlkIdx = 0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
		{
			int x0 = Intra4x4ScanOrder[luma4x4BlkIdx][0];
			int y0 = Intra4x4ScanOrder[luma4x4BlkIdx][1];

			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{				
					diffL4x4[i][j] = frame.L[(yP + y0 + i)*frame.Lwidth+(xP + x0 + j)] - predL[y0 + i][x0 + j];
				}
			}			

			if (MbPartPredMode(mb_type , 0) == Intra_16x16)
			{
				forwardResidual(qP, diffL4x4, rLuma, true, true);	
				DCLuma[y0/4][x0/4] = rLuma[0][0];			
				transformScan(rLuma, Intra16x16ACLevel[luma4x4BlkIdx], true);			
			}
			else if (MbPartPredMode(mb_type,0) != Intra_4x4)
			{
				forwardResidual(qP, diffL4x4, rLuma, false, false);
				transformScan(rLuma, LumaLevel[luma4x4BlkIdx], false);
				// picture reconstruction:
				if (reconstruct == true)
				{
					transformDecoding4x4LumaResidual(LumaLevel, predL, luma4x4BlkIdx, QPy);
				}
			}
		}

		if (MbPartPredMode(mb_type , 0) == Intra_16x16)
		{
			forwardDCLumaIntra(QPy, DCLuma, rDCLuma);

			transformScan(rDCLuma, Intra16x16DCLevel, false);

			//picture reconstruction:
			if (reconstruct == true)
			{
				transformDecodingIntra_16x16Luma(Intra16x16DCLevel, Intra16x16ACLevel, predL, QPy);
			}
		}
	}

	//Chroma quantization and transform process

	int xPC = xP/2;
	int yPC = yP/2;

	int QpBdOffsetC = 0;	// Norm: = 6 * bit_depth_chroma_minus8; bit_depth_chroma_minus_8 == 0 in baseline
	int qPoffset = pps.chroma_qp_index_offset;	// Norm: qPoffset = second_chroma_qp_index_offset,
												// second_chroma_qp_index_offset == chroma_qp_index_offset when not
												// present. It is not present in baseline.
	int qPi = Clip3(-QpBdOffsetC, 51, QPy + qPoffset);
	int QPc = qPiToQPc[qPi];
	int QP_c = QPc + QpBdOffsetC;

	qP = QP_c;

	for (int chroma4x4BlkIdx = 0; chroma4x4BlkIdx < 4; chroma4x4BlkIdx++)
	{
		x0C = (chroma4x4BlkIdx % 2) * 4;
		y0C = (chroma4x4BlkIdx / 2) * 4;
		
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{				
				diffCb4x4[i][j] = frame.C[0][(yPC + y0C + i)*frame.Cwidth+(xPC + x0C + j)] - predCb[y0C + i][x0C + j];
				diffCr4x4[i][j] = frame.C[1][(yPC + y0C + i)*frame.Cwidth+(xPC + x0C + j)] - predCr[y0C + i][x0C + j];
			}
		}

		if (MbPartPredMode(mb_type , 0) == Intra_16x16 || MbPartPredMode(mb_type , 0) == Intra_4x4)
		{
			forwardResidual(qP, diffCb4x4, rCb, true, true);
			forwardResidual(qP, diffCr4x4, rCr, true, true);
		}
		else
		{
			forwardResidual(qP, diffCb4x4, rCb, false, true);
			forwardResidual(qP, diffCr4x4, rCr, false, true);
		}

		DCCb[y0C>>2][x0C>>2] = rCb[0][0];
		DCCr[y0C>>2][x0C>>2] = rCr[0][0];

		scanChroma(rCb, ChromaACLevel[0][chroma4x4BlkIdx]);
		scanChroma(rCr, ChromaACLevel[1][chroma4x4BlkIdx]);
	}

	if (MbPartPredMode(mb_type , 0) == Intra_16x16 || MbPartPredMode(mb_type , 0) == Intra_4x4)
	{
		forwardDCChroma(qP, DCCb, rDCCb, true);
		forwardDCChroma(qP, DCCr, rDCCr, true);		
	}
	else
	{
		forwardDCChroma(qP, DCCb, rDCCb, false);
		forwardDCChroma(qP, DCCr, rDCCr, false);
	}

	scanDCChroma(rDCCb, ChromaDCLevel[0]);
	scanDCChroma(rDCCr, ChromaDCLevel[1]);

	// Reconstruction process:
	if (reconstruct == true)
	{
		transformDecodingChroma(ChromaDCLevel[0], ChromaACLevel[0], predCb, QPy, true);
		transformDecodingChroma(ChromaDCLevel[1], ChromaACLevel[1], predCr, QPy, false);
	}
}