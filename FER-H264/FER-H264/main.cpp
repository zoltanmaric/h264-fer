#include <stdio.h>
#include <stdlib.h>

#include "nal.h"

int main(int argc, char *argv[])
{
	FILE *f;

	f = fopen("test.264", "rb");
	unsigned long ptr;		// the pointer inside the file

	ptr = 0;
	NALunit nu = getNAL(f, &ptr);

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
		if (i % 20 == 19) printf("\n");
	}

	printf("\n");
	system("pause");
	return 0;
}