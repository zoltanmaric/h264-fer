#include <stdio.h>
#include <stdlib.h>

#include "nal.h"
#include "intra.h"

void testNAL(FILE *stream)
{
	unsigned long ptr;		// the pointer inside the file

	ptr = 0;
	NALunit nu = getNAL(stream, &ptr);

	printf("NAL header:\n");
	printf("forbidden_zero_bit = %u\n", nu.forbidden_zero_bit);
	printf("nal_ref_idc = %u\n", nu.nal_ref_idc);
	printf("nal_unit_type = %u\n", nu.nal_unit_type);
	printf("Size of RBSP: %u\n", nu.NumBytesInRBSP);
	system("pause");
	printf("\nRBSP data:\n");
	for (unsigned int i = 0; i < nu.NumBytesInRBSP; i++)
	{
		printf("%2x ", nu.rbsp_byte[i]);
		if ((i % 20) == 19) printf("\n");
	}

	printf("\n");
}

void testIntra(frame &f)
{
	mb_mode mb;
	mb.MbPartPredMode[0] = Intra_16x16;
	
	mode_pred_info mpi;
	mpi.MbHeight = f.Lheight / 16;
	mpi.MbWidth = f.Lwidth / 16;
	mpi.MbMode = (int *)malloc(mpi.MbHeight*mpi.MbWidth*sizeof(int));
	mpi.TbHeight = f.Lheight / 4;
	mpi.TbWidth = f.Lwidth / 4;
	mpi.Intra4x4PredMode = (int *)malloc(mpi.TbHeight*mpi.TbWidth*sizeof(int));

	int *predL = NULL, *predCr = NULL, *predCb = NULL;
	int intra_chroma_pred_mode = 0;
	intraPrediction(f, mpi, mb, 33, predL, predCr, predCb, intra_chroma_pred_mode);
}

int main(int argc, char *argv[])
{
	FILE *stream;

	stream = fopen("test.264", "rb");

	FILE *input; 
	input = fopen("lenna.ppm", "rb");	// otvori ulaznu datoteku
	if(input == NULL)
	{
		fprintf(stderr, "Nema ulazne datoteke lenna.ppm!\n");
		return 0;
	}

	frame f;

	fscanf(input, "%*s");   // read 'magic number' (ignored)
	fscanf(input, "%d %d", &f.Lwidth, &f.Lheight);	// read picture dimensions
	fscanf(input, "%*d\n");	// read maximum value (ignored)

	f.Cheight = f.Lheight / 2;
	f.Cwidth = f.Lwidth / 2;

	f.L = (unsigned char *) malloc (f.Lheight * f.Lwidth);
	f.C[0] = (unsigned char *) malloc (f.Cheight * f.Cwidth);
	f.C[1] = (unsigned char *) malloc (f.Cheight * f.Cwidth);

	unsigned char *rgb;
	rgb = (unsigned char *) malloc (f.Lwidth * f.Lheight * 3 * sizeof(char));
	fread(rgb, 1, f.Lwidth * f.Lheight * 3, input);	// TODO: doskok neuspjelom citanju
	fclose(input);

	int k = 0;
	int l = 0;
	for (int i = 0; i < f.Lheight; i++)
	{
		for (int j = 0; j < f.Lwidth; j++)
		{
			int r = rgb[3*k];
			int g = rgb[3*k + 1];
			int b = rgb[3*k + 2];
			f.L[k] = 0.2990 * r + 0.5870 * g + 0.1140 * b - 128;
			if ((i%2 == 0) && (j%2 == 0))
			{
				f.C[0][l] = -0.168736 * r - 0.331264 * g + 0.5 * b + 128;
				f.C[1][l] = 0.5 * r - 0.418688 * g - 0.081312 * b + 128;
				l++;
			}
			k++;
		}
	}
	
	//testNAL(stream);
	testIntra(f);

	system("pause");
	return 0;
}