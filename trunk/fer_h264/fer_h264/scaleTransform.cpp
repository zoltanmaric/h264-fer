//#include <math.h>
#include "scaleTransform.h"

//Multiplication factors for quantization
int factorsQ[6][4][4] = 
{
	{{13107,8066,13107,8066},{8066,5243,8066,5243},{13107,8066,13107,8066},{8066,5243,8066,5243}},
	{{11916,7490,11916,7490},{7490,4660,7490,4660},{11916,7490,11916,7490},{7490,4660,7490,4660}},
	{{10082,6554,10082,6554},{6554,4194,6554,4194},{10082,6554,10082,6554},{6554,4194,6554,4194}},
	{{9362,5825, 9352,5825},{5825,3647,5825,3647},{9362,5825, 9352,5825},{5825,3647,5825,3647}},
	{{8192,5243, 8192,5243},{5243,3355,5243,3355},{8192,5243, 8192,5243},{5243,3355,5243,3355}},
	{{7282,4559, 7282,4559},{4559,2893,4559,2893},{7282,4559, 7282,4559},{4559,2893,4559,2893}}
};

//Multiplication factors for inverse quantization
int factorsDQ[6][4][4] = 
{
	{{10,13,10,13},{13,16,13,16},{10,13,10,13},{13,16,13,16}},
	{{11,14,11,14},{14,18,14,18},{11,14,11,14},{14,18,14,18}},
	{{13,16,13,16},{16,20,16,20},{13,16,13,16},{16,20,16,20}},
	{{14,18,14,18},{18,23,18,23},{14,18,14,18},{18,23,18,23}},
	{{16,20,16,20},{20,25,20,25},{16,20,16,20},{20,25,20,25}},
	{{18,23,18,23},{23,29,23,29},{18,23,18,23},{23,29,23,29}}
};

int ZigZagReordering[16][2] = 
{
	{0,0}, {0,1}, {1,0}, {2,0}, {1,1}, {0,2}, {0,3}, {1,2},
	{2,1}, {3,0}, {3,1}, {2,2}, {1,3}, {2,3}, {3,2}, {3,3}
};

//transformation for all the residual block
//all the blocks are transformed using this transform
void forwardTransform4x4(int input[4][4], int output[4][4])
{
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


void inverseTransform4x4(int d[4][4], int r[4][4])
{	
	int e[4][4], f[4][4], g[4][4], h[4][4];
	
	for (int i = 0; i < 3; i++)
	{
		e[i][0] = d[i][0] + d[i][2];
		e[i][1] = d[i][0] - d[i][2];
		e[i][2] = (d[i][1] >> 1) - d[i][3];
		e[i][3] = d[i][1] + (d[i][3] >> 1);
	}

	for (int i = 0; i < 3; i++)
	{
		f[i][0] = e[i][0] + e[i][3];
		f[i][1] = e[i][1] + e[i][2];
		f[i][2] = e[i][1] - e[i][2];
		f[i][3] = e[i][0] - e[i][3];
	}

	for (int j = 0; j < 3; j++)
	{
		g[0][j] = f[0][j] + f[2][j];
		g[1][j] = f[0][j] - f[2][j];
		g[2][j] = (f[1][j] >> 1) - f[3][j];
		g[3][j] = f[1][j] + (f[3][j] >> 1);
	}

	for (int j = 0; j < 3; j++)
	{
		h[0][j] = g[0][j] + g[3][j];
		h[1][j] = g[1][j] + g[2][j];
		h[2][j] = g[1][j] - g[2][j];
		h[3][j] = g[0][j] - g[3][j];
	}

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			r[i][j] = (h[i][j] + 32) >> 6;
		}
	}
}


// transformation for the DC coeff from Luma block previously transformed using integer
// transformation
void HadamardLumaTransform(int input[4][4], int output[4][4])
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
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			output[i][j] = output[i][j] >> 1;
		}
	}

}


void inverseHadamardLumaTransform (int input[4][4], int output[4][4])
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
}


// transformation for the DC coeff from Chroma block previously transformed using integer
// transformation
// forward and inverse transformations are the same
void HadamardChromaTransform (int input[2][2], int output[2][2])
{
	int i, j;
	int output_temp[2][2];

	//multiply A*W
	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < 2; j++)
		{
			switch (i)
			{
				case 0: 
					output_temp[i][j] = input[0][j] + input[1][j];
					break;
				case 1: 					
					output_temp[i][j] = input[0][j] - input[1][j];
					break;				
			}
		}
	}
	
	//multiply (A*W)*At
	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < 2; j++)
		{
			switch (j)
			{
				case 0: 
					output[i][j] = output_temp[i][0] + output_temp[i][1];
					break;
				case 1: 					
					output[i][j] = output_temp[i][0] + output_temp[i][1];
					break;				
			}
		}
	}
}


//Scaling process for the residual 4x4 blocks (Chroma and Luma)
void scaleResidualBlock(int input[4][4], int output[4][4], int quantizer, bool intra16x16OrChroma)
{	
	int qbits = quantizer/6;
	int adjust = 0;
	int qp=quantizer%6;

	if (quantizer>=24)
	{
		qbits = qbits - 4;
		adjust = 0;
	}
	else
	{
		adjust = 1 << (3 - qbits);
		qbits = 4 - qbits;		
	}

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			output[i][j] = (input[i][j] * factorsDQ[qp][i][j] + adjust) << qbits;
		}
	}

	if (intra16x16OrChroma) output[0][0] = input[0][0];	
}


void scaleLumaDCIntra(int input[4][4], int qp, int output[4][4])
{
	int qp_calculate = qp/6;
	int scaleV = factorsDQ[qp%6][0][0];	

	int shiftG36 = qp_calculate - 6;
	int shiftL36 = 6 - qp_calculate;
	int adjust = 1 << (5 - qp_calculate);

	//Scaling proccess according to the latest H.264 ITU specification
	if (qp >= 36)
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				output[i][j] = (input[i][j]*scaleV) << shiftG36;
			}
		}
	}
	else
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				output[i][j] = (input[i][j]*scaleV + adjust) >> shiftL36;
			}
		}
	}

	//Scaling process according to the Ian Richardson's book
	/*
	int shift12 = qp_calculate-2;
	int shift0 = 2 - qp_calculate;

	if (qp >= 12)
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				output[i][j] = (input[i][j]*scaleV) << shift12;
			}
		}
	}
	else
	{
		int pom = 1 << (1 - qp);
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				output[i][j] = (input[i][j]*scaleV + pom) >> shift0;
			}
		}
	}
	*/
}

void scaleChromaDC(int input[2][2], int qp, int output[2][2])
{
	int qp_calculate = qp/6;
	int scaleV = factorsDQ[qp%6][0][0];

	// Scaling proccess according to the Ian Richardson's book
	/*
	if (qp >= 6)
	{
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				output[i][j] = (input[i][j]*scaleV) << (qp_calculate - 1);
			}
		}		
	}
	else
	{
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				output[i][j] = (input[i][j]*scaleV) >> 1;
			}
		}
	}
	*/

	//Scaling proccess according to the latest H.264 ITU specification	
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			output[i][j] = ((input[i][j]*scaleV) << qp_calculate) >> 5;
		}
	}
		
}

//---------------------------------------------------//
//---------------- PUBLIC FUNCTIONS -----------------//
//---------------------------------------------------//

void inverseResidual(int bitDepth, int qP, int c[4][4], int r[4][4], bool intra16x16OrChroma)
{
	int d[4][4];

	scaleResidualBlock(c, d, qP, intra16x16OrChroma);
	inverseTransform4x4(d, r);
}

void InverseDCLumaIntra (int bitDepth, int qP, int c[4][4], int dcY[4][4])
{
	int f[4][4];

	inverseHadamardLumaTransform(c, f);
	scaleLumaDCIntra(f, qP, dcY);	
}

void InverseDCChroma (int bitDepth, int qP, int c[2][2], int dcC[2][2])
{
	int f[2][2];

	HadamardChromaTransform(c, f);
	scaleChromaDC(f, qP, dcC);	
}