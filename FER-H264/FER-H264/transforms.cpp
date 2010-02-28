#include <math.h>
#include "transforms.h"

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


void inverseTransform4x4(int input[4][4], int output[4][4])
{
	int i, j, pom_1, pom_3;
	int output_temp[4][4];	

	//multiply A*X
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			switch (i)
			{
				case 0:
					pom_3 = input[3][j] >> 1;
					output_temp[i][j] = input[0][j] + input[1][j] + input[2][j] + pom_3;					
					break;
				case 1: 
					pom_1 = input[1][j] >> 1;					
					output_temp[i][j] = input[0][j] + pom_1 - input[2][j] - input[3][j];
					break;
				case 2:
					pom_1 = input[1][j] >> 1;					
					output_temp[i][j] = input[0][j] - pom_1 - input[2][j] + input[3][j];
					break;
				case 3:
					pom_3 = input[3][j] >> 1;
					output_temp[i][j] = input[0][j] - input[1][j] + input[2][j] - pom_3;
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
					pom_3 = output_temp[i][3] >> 1;
					output[i][j] = output_temp[i][0] + output_temp[i][1] + output_temp[i][2] + pom_3;
					break;
				case 1: 
					pom_1 = output_temp[i][1] >> 1;					
					output[i][j] = output_temp[i][0] + pom_1 - output_temp[i][2] - output_temp[i][3];
					break;
				case 2:
					pom_1 = output_temp[i][1] >> 1;
					output[i][j] = output_temp[i][0] - pom_1 - output_temp[i][2] + output_temp[i][3];
					break;
				case 3:					
					pom_3 = output_temp[i][3] >> 1;
					output[i][j] = output_temp[i][0] - output_temp[i][1] + output_temp[i][2] - pom_3;
					break;
			}
		}
	}

	//divide by 64 and round
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			output[i][j] = (output[i][j] + 32) >> 6;
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


//---------------------------------------------------//
//---------------- Quantization ---------------------//
//---------------------------------------------------//
block quantizeForward(block input, int quantizer, int predMode) {

	block output;
	int qbits=(quantizer/6)+15;
	int qp=(quantizer%6);
	int f=(1<<qbits)/predMode;
  
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			output.elements[i][j] = Sign((input.elements[i][j]>>31), (abs(input.elements[i][j])*factorsQ[qp][i][j] + f) >> qbits);
		}
	}

	return output;
}



//Scaling process for the residual 4x4 blocks (Chroma and Luma)
block quantizeInverse(int input[4][4], int output, int quantizer, bool dc)
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
			output[i][j] = Sign((input[i][j]>>31), (abs(input[i][j])*factorsDQ[qp][i][j] + adjust) << qbits);
		}
	}

	if (!dc) output[0][0] = input[0][0];	
}


block quantizeLumaDCInverse(block input, int quantizer)
{
	block output;

	int qp = quantizer/6;
	int scaleV = factorsDQ[quantizer%6][0][0];
	int shift12 = qp-2;
	int shift0 = 2 - qp;

	if (quantizer >= 12)
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				output.elements[i][j] = (input.elements[i][j]*scaleV) << shift12;
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
				output.elements[i][j] = (input.elements[i][j]*scaleV + pom) >> shift0;
			}
		}
	}

	return output;
}

small_block quantizeChromaDCInverse (small_block input, int quantizer)
{
	small_block output;

	int qp = quantizer/6;
	int scaleV = factorsDQ[quantizer%6][0][0];

	if (quantizer >= 6)
	{
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				output.elements[i][j] = (input.elements[i][j]*scaleV) << (qp - 1);
			}
		}		
	}
	else
	{
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				output.elements[i][j] = (input.elements[i][j]*scaleV) >> 1;
			}
		}
	}

	return output;
}
//---------------------------------------------------//
//-------------------- ZIG ZAG ----------------------//
//---------------------------------------------------//

block ReorderElements (int *input)
{
	block Reordered;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			Reordered.elements[ZigZagReordering[i*4 + j][0]][ZigZagReordering[i*4 + j][0]] = input[i*4 + j];
		}
	}

	return Reordered;
}

//---------------------------------------------------//
//---------------- PUBLIC FUNCTIONS -----------------//
//---------------------------------------------------//

void inverseResidual(int bitDepth, int qP, int c[4][4], int r[4][4], bool luma)
{
	int temp[4][4];

	quantizeInverse(c, temp, qp, !luma);
	inverseTransform4x4(temp, r);
}
/*
void InverseResidualLuma (int *input, frame current, int x, int y, int quantizer, int dc)
{
	int c[][];
	block temp;
	int currentPosition;

	temp = ReorderElements (input);

	temp = quantizeInverse(temp.elements, quantizer, dc);

	temp = inverseTransform4x4(temp);

	currentPosition = y * current.Lwidth + x;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			current.L[currentPosition + i*4 + j] = temp.elements[i][j];
		}
	}
}

void InverseResidualChroma (int *input, frame current, int x, int y, int quantizer, int dc, int chromaComponent)
{
	block temp;
	int currentPosition;

	temp = ReorderElements (input);

	temp = quantizeInverse(temp.elements, quantizer, dc);

	temp = inverseTransform4x4(temp);

	currentPosition = y * current.Cwidth + x;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			current.C[chromaComponent][currentPosition + i*4 + j] = temp.elements[i][j];
		}
	}
}*/


void InverseDCLumaIntra (int *input, block output, int quantizer)
{
	block temp = ReorderElements(input);
	block transformed;

	inverseHadamardLumaTransform(temp.elements, transformed.elements);

	temp = quantizeLumaDCInverse(transformed, quantizer);

	output = temp;
}

void InverseDCChroma (int *input, small_block output, int quantizer)
{
	small_block temp, transformed;

	HadamardChromaTransform(temp.elements, transformed.elements);

	temp = quantizeChromaDCInverse(transformed, quantizer);

	output = temp;
}
//int main()
//{
//	int polje[4][4] = {{5, 11, 8, 10},{9, 8, 4, 12},{1, 10, 11, 4},{19, 6, 15, 7}};
//	int revpolje[4][4] = {{544, 0, -32, 0}, {-40, -100, 0, -250}, {96, 40, 32, 80}, {-80, -50, -200, -50}};
//	int rez[4][4];
//	block revrez;
//	int temp[4][4];
//
//	int i, j;
//
//	//Residual 4x4 blocks transform, quantization and inverse
//	forwardTransform4x4(polje, rez);
//	quantizeForward(rez, 10, 3, temp);
//	revrez = quantizeInverse(temp, 10, 1);	
//	inverseTransform4x4(revrez.elements, temp);
//
//	for (i = 0; i < 4; i++)
//	{
//		for (j = 0; j < 4; j++)
//		{
//			printf("%d ", temp[i][j]);
//		}
//		printf("\n");
//	}
//
//}

