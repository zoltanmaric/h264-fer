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

int Sign (int number)
{
	if (number < 0) return (number * (-1));
	else return number;
}

void forwardTransform4x4(int input[4][4], int output[4][4])
{
	/*int i, j;
	int e[4][4], f[4][4], g[4][4], h[4][4];

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			h[i][j] = (input[i][j] << 6) - 32;			
		}
	}


	for (j = 0; j < 4; j++)
	{
		g[0][j] = (h[0][j] + h[3][j]) >> 1;
		g[1][j] = (h[1][j] + h[2][j]) >> 1;
		g[2][j] = (h[1][j] - h[2][j]) >> 1;
		g[3][j] = (h[0][j] - h[3][j]) >> 1;
	}

	for (j = 0; j < 4; j++)
	{
		f[0][j] = (g[0][j] + g[1][j]) >> 1;
		f[1][j] = (g[2][j] + (g[3][j] << 1)) >> 1;
		f[2][j] = (g[0][j] - g[1][j]) >> 1;
		f[3][j] = (-(g[2][j] << 1) + g[3][j]) >> 1;
	}

	for (i = 0; i < 4; i++)
	{
		e[i][0] = (f[i][0] + f[i][3]) >> 1;
		e[i][1] = (f[i][1] + f[i][2]) >> 1;
		e[i][2] = (f[i][1] - f[i][2]) >> 1;
		e[i][3] = (f[i][0] - f[i][3]) >> 1;
	}

	for (i = 0; i < 4; i++)
	{
		output[i][0] = (e[i][0] + e[i][1]) >> 1;
		output[i][1] = (e[i][2] + (e[i][3] << 1)) >> 1;
		output[i][2] = (e[i][0] - e[i][1]) >> 1;
		output[i][3] = (-(e[i][2] << 1) + e[i][3]) >> 1;
	}*/

	int i, j, pom_0, pom_1, pom_2, pom_3;
	int output_temp[4][4];	
	
	//multiply A*X
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			switch (i)
			{
				case 0: 
					output_temp[i][j] = input[0][j] + input[1][j] + input[2][j] + input[3][j];					
					break;
				case 1: 
					pom_0 = input[0][j] << 1;
					pom_3 = input[3][j] << 1;
					output_temp[i][j] = pom_0 + input[1][j] - input[2][j] - pom_3;
					break;
				case 2:
					output_temp[i][j] = input[0][j] - input[1][j] - input[2][j] + input[3][j];
					break;
				case 3:
					pom_1 = input[1][j] << 1;
					pom_2 = input[2][j] << 1;
					output_temp[i][j] = input[0][j] - pom_1 + pom_2 - input[3][j];
					break;
			}
		}
	}
	
	//multiply (A*X)*At
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			switch (j)
			{
				case 0: 
					output[i][j] = output_temp[i][0] + output_temp[i][1] + output_temp[i][2] + output_temp[i][3];
					break;
				case 1: 
					pom_0 = output_temp[i][0] << 1;
					pom_3 = output_temp[i][3] << 1;
					output[i][j] = pom_0 + output_temp[i][1] - output_temp[i][2] - pom_3;
					break;
				case 2:
					output[i][j] = output_temp[i][0] - output_temp[i][1] - output_temp[i][2] + output_temp[i][3];
					break;
				case 3:
					pom_1 = output_temp[i][1] << 1;
					pom_2 = output_temp[i][2] << 1;
					output[i][j] = output_temp[i][0] - pom_1 + pom_2 - output_temp[i][3];
					break;
			}
		}
	}

}


void forwardTransformDCLumaIntra(int input[4][4], int output[4][4])
{
	int i, j;
	int output_temp[4][4];

	//multiply A*W
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			switch (i)
			{
				case 0: 
					output_temp[i][j] = input[0][j] + input[1][j] + input[2][j] + input[3][j];					
					break;
				case 1: 					
					output_temp[i][j] = input[0][j] + input[1][j] - input[2][j] - input[3][j];
					break;
				case 2:
					output_temp[i][j] = input[0][j] - input[1][j] - input[2][j] + input[3][j];
					break;
				case 3:					
					output_temp[i][j] = input[0][j] - input[1][j] + input[2][j] - input[3][j];
					break;
			}
		}
	}
	
	//multiply (A*W)*At
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			switch (j)
			{
				case 0: 
					output[i][j] = output_temp[i][0] + output_temp[i][1] + output_temp[i][2] + output_temp[i][3];
					break;
				case 1: 					
					output[i][j] = output_temp[i][0] + output_temp[i][1] - output_temp[i][2] - output_temp[i][3];
					break;
				case 2:
					output[i][j] = output_temp[i][0] - output_temp[i][1] - output_temp[i][2] + output_temp[i][3];
					break;
				case 3:					
					output[i][j] = output_temp[i][0] - output_temp[i][1] + output_temp[i][2] - output_temp[i][3];
					break;
			}
		}
	}

	//shifting one bit to the right, equaly to dividing with 2
	/*for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			output[i][j] = output[i][j] >> 1;
		}
	}*/

}


// --------------------------------------------------//
// 8.5.11.1 Transformation process for chroma DC transform coefficients
void transformDCChromaF (int c[2][2], int f[2][2])
{
	int d[2][2];

	d[0][0] = c[0][0] + c[1][0];
	d[0][1] = c[0][1] + c[1][1];
	d[1][0] = c[0][0] - c[1][0];
	d[1][1] = c[0][1] - c[1][1];

	f[0][0] = d[0][0] + d[0][1];
	f[0][1] = d[0][0] - d[0][1];
	f[1][0] = d[1][0] + d[1][1];
	f[1][1] = d[1][0] - d[1][1];
	
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			f[i][j] = f[i][j] >> 2;
}


void quantisationResidualBlock(int d[4][4], int c[4][4], int qP, bool Intra)
{
	int qbits = 15 + qP/6;
	int qPMod = qP % 6;
	int f;	

	if (Intra)
	{
		f = (1 << qbits) / 3;
	}
	else
	{
		f = (1 << qbits) / 6;
	}

	/*qbits = 4 - qP/6;
	int adjust = 1 << (3 - qP/6);

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			c[i][j] = ((d[i][j] << qbits) - adjust) / LevelScale[qPMod][i][j];
		}
	}*/


	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			int sign = ExtractSign(d[i][j]);
			if (sign) d[i][j] =  sign * d[i][j];			
			c[i][j] = (d[i][j] * MF[qPMod][i][j] + f) >> qbits;
			if (sign) c[i][j] = sign * c[i][j];
		}
	}
}

void quantisationLumaDCIntra (int f[4][4], int qP, int c[4][4])
{
	int qbits = 15 + qP/6;
	int qPMod = qP % 6;
	int f_adjust;	

	f_adjust = (1 << qbits) / 3;	

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			int sign = ExtractSign(f[i][j]);
			if (sign) f[i][j] =  sign * f[i][j];	
			c[i][j] = (f[i][j] * MF[qPMod][0][0] + 2 * f_adjust) >> (qbits + 3);
			if (sign) c[i][j] =  sign * c[i][j];
		}
	}
}

void quantisationChromaDC(int f[2][2], int qP, int c[2][2], bool Intra)
{
	int qbits = 15 + qP/6;
	int qPMod = qP % 6;
	int f_adjust;	

	/*int qP_calculate = qP/6;
	int scaleL = LevelScale[qPMod][0][0];

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			c[i][j] = ((f[i][j] << 5) >> qP_calculate) / scaleL;
		}
	}*/

	if (Intra)
	{
		f_adjust = (1 << qbits) / 3;
	}
	else
	{
		f_adjust = (1 << qbits) / 6;
	}

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			int sign = ExtractSign(f[i][j]);
			if (sign) f[i][j] =  sign * f[i][j];	
			c[i][j] = (f[i][j] * MF[qPMod][0][0] + 2 * f_adjust) >> (qbits + 3);
			if (sign) c[i][j] =  sign * c[i][j];
		}
	}
}

void forwardResidual(int qP, int c[4][4], int r[4][4], bool Intra)
{
	int d[4][4];

	forwardTransform4x4(c, d);
	quantisationResidualBlock(d, r, qP, Intra);

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

	transformDCChromaF(dcC, f);
	quantisationChromaDC(f, qP, c, Intra);
}

void transformScan(int c[4][4], int list[16], bool Intra16x16AC)
{
	if (Intra16x16AC)
	{
		for (int i = 1; i < 16; i++)
		{
			int x = ZigZagReordering[i][0];
			int y = ZigZagReordering[i][1];
			list[i-1] = c[x][y];
		}		
	}
	else {
		for (int i = 0; i < 16; i++)
		{
			int x = ZigZagReordering[i][0];
			int y = ZigZagReordering[i][1];
			list[i] = c[x][y];
		}
	}
}

void scanChroma(int rChroma[4][4], int list[16])
{
	for (int i = 1; i < 16; i++)
	{
		int x = ZigZagReordering[i][0];
		int y = ZigZagReordering[i][1];
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

void quantizationTransform(int predL[16][16], int predCb[8][8], int predCr[8][8])
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

	int xP = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 0);
	int yP = InverseRasterScan(CurrMbAddr, 16, 16, frame.Lwidth, 1);

	qP = QPy;	

	for (int luma4x4BlkIdx = 0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
	{
		int x0 = Intra4x4ScanOrder[luma4x4BlkIdx][0];
		int y0 = Intra4x4ScanOrder[luma4x4BlkIdx][1];

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{				
				diffL4x4[i][j] = frame.L[yP + y0 + i][xP + x0 + j] - predL[y0 + i][x0 + j];
			}
		}

		forwardResidual(qP, diffL4x4, rLuma, true);		

		if (MbPartPredMode(mb_type , 0) == Intra_16x16)
		{
			DCLuma[y0/4][x0/4] = rLuma[0][0];			
			transformScan(rLuma, Intra16x16ACLevel[luma4x4BlkIdx], true);

			inverseResidual(8, qP, rLuma, reconstructedBlock, true);
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					reconstructedLuma[y0 + i][x0 + i] = reconstructedBlock[i][j];
				}
			}
		}
		else
		{
			transformScan(rLuma, LumaLevel[luma4x4BlkIdx], false);
			inverseResidual(8, qP, rLuma, reconstructedBlock, false);
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					frame.L[yP + y0 + i][xP + x0 + j] = reconstructedBlock[i][j] + predL[y0 + i][x0 + j];
				}
			}
		}
	}

	if (MbPartPredMode(mb_type , 0) == Intra_16x16)
	{
		forwardDCLumaIntra(QPy, DCLuma, rDCLuma);
		transformScan(rDCLuma, Intra16x16DCLevel, false);

		//picture reconstruction
		
		InverseDCLumaIntra(8, qP, rDCLuma, reconstructedDCY);

		for (int luma4x4BlkIdx = 0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
		{
			int x0 = Intra4x4ScanOrder[luma4x4BlkIdx][0];
			int y0 = Intra4x4ScanOrder[luma4x4BlkIdx][1];

			reconstructedLuma[y0][x0] = reconstructedDCY[y0/4][x0/4];

			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					frame.L[yP + y0 + i][xP + x0 + j] = reconstructedLuma[y0 + i][x0 + j] + predL[y0 + i][x0 + j];
				}
			}
		}		
	}

	//Chroma quantization and transform process

	int reconstructedChroma[2][8][8];
	int reconstructedChromaBlock[2][4][4];
	int reconstructedDCChroma[2][2][2];

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
				diffCb4x4[i][j] = frame.C[0][yPC + y0C + i][xPC + x0C + j] - predCb[y0C + i][x0C + j];
				diffCr4x4[i][j] = frame.C[1][yPC + y0C + i][xPC + x0C + j] - predCr[y0C + i][x0C + j];				
			}
		}

		forwardResidual(qP, diffCb4x4, rCb, true);
		forwardResidual(qP, diffCr4x4, rCr, true);

		inverseResidual(8, qP, rCb,reconstructedChromaBlock[0], true);
		inverseResidual(8, qP, rCr,reconstructedChromaBlock[1], true);

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{				
				reconstructedChroma[0][y0C + i][x0C + j] = reconstructedChromaBlock[0][i][j];
				reconstructedChroma[1][y0C + i][x0C + j] = reconstructedChromaBlock[1][i][j];
			}
		}

		DCCb[y0C/4][x0C/4] = rCb[0][0];
		DCCr[y0C/4][x0C/4] = rCr[0][0];

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

	InverseDCChroma(8, qP, rDCCb, reconstructedDCChroma[0]);
	InverseDCChroma(8, qP, rDCCb, reconstructedDCChroma[1]);

	for (int chroma4x4BlkIdx = 0; chroma4x4BlkIdx < 4; chroma4x4BlkIdx++)
	{
		x0C = (chroma4x4BlkIdx % 2) * 4;
		y0C = (chroma4x4BlkIdx / 2) * 4;

		reconstructedChroma[0][y0C][x0C] = reconstructedDCChroma[0][y0C/4][x0C/4];
		reconstructedChroma[1][y0C][x0C] = reconstructedDCChroma[1][y0C/4][x0C/4];
		
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{				
				frame.C[0][yPC + y0C + i][xPC + x0C + j] = reconstructedChroma[0][y0C + i][x0C + j] + predCb[y0C + i][x0C + j];
				frame.C[1][yPC + y0C + i][xPC + x0C + j] = reconstructedChroma[0][y0C + i][x0C + j] + predCr[y0C + i][x0C + j];				
			}
		}
	}

	scanDCChroma(rDCCb, ChromaDCLevel[0]);
	scanDCChroma(rDCCr, ChromaDCLevel[1]);
}