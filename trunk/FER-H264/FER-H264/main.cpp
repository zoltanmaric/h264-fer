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

void testIntra()
{
	frame f;
	// Full HD, bitchezz!
	f.Lwidth = 1920;
	f.Lheight = 1080;
	f.L = (unsigned char *)malloc(f.Lwidth*f.Lheight);

	for (int i = 0; i < f.Lheight; i++)
	{
		for (int j = 0; j < f.Lwidth; j++)
		{
			f.L[i*f.Lwidth + j] = i*f.Lwidth + j;
		}
	}

	mb_mode mb;
	mb.MbPartPredMode[0] = Intra_4x4;
	
	mode_pred_info mpi;
	mpi.MbHeight = f.Lheight / 16;
	mpi.MbWidth = f.Lwidth / 16;
	mpi.MbMode = (int *)malloc(mpi.MbHeight*mpi.MbWidth*sizeof(int));
	mpi.TbHeight = f.Lheight / 4;
	mpi.TbWidth = f.Lwidth / 4;
	mpi.Intra4x4PredMode = (int *)malloc(mpi.TbHeight*mpi.TbWidth*sizeof(int));

	intraPrediction(f, mpi, mb, 121);
}

int main(int argc, char *argv[])
{
	FILE *stream;

	stream = fopen("test.264", "rb");
	
	//testNAL(stream);
	testIntra();

	system("pause");
	return 0;
}