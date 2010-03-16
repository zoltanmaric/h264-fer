#include "nal.h"

// Returns 0 if end of stream found.
unsigned long findNALstart(FILE *input, unsigned long fPtr)
{
	unsigned char buffer[BUFFER_SIZE];		// the buffer for the bytes read
	unsigned int bytesRead;
	bool startPrefixFound;

	fseek(input, fPtr, SEEK_SET);

	// read BUFFER_SIZE bytes from the file:
	while (bytesRead = fread(buffer, 1, BUFFER_SIZE, input))
	{
		startPrefixFound = false;
		for (unsigned int i = 0; i < bytesRead - 3; i++)
		{
			if (buffer[i] == 0)
			{
				if (buffer[i+1] == 0)
				{
					if (buffer[i+2] == 0)
					{
						if (buffer[i+3] == 1)
						{
							// found start code prefix
							startPrefixFound = true;
							// set the file pointer to after the prefix:
							fPtr += i + 4;
							fseek(input, fPtr, SEEK_SET);
							break;
						}
					}
				}
			}
		}

		if (startPrefixFound) break;

		// start code not found in this
		// access, set the new fPtr position:
		fPtr += BUFFER_SIZE - 3;
		fseek(input, fPtr, SEEK_SET);
	}

	if (startPrefixFound == false)
	{
		return 0;
	}

	return fPtr;
}

unsigned long findNALend(FILE *input, unsigned long fPtr)
{
	unsigned char buffer[BUFFER_SIZE];		// the buffer for the bytes read
	unsigned int bytesRead;
	bool startPrefixFound;

	fseek(input, fPtr, SEEK_SET);

	// read BUFFER_SIZE bytes from the file:
	while (bytesRead = fread(buffer, 1, BUFFER_SIZE, input))
	{
		startPrefixFound = false;
		for (unsigned int i = 0; i < bytesRead - 2; i++)
		{
			if (buffer[i] == 0)
			{
				if (buffer[i+1] == 0)
				{
					if ((buffer[i+2] == 0) ||
						(buffer[i+2] == 1))
					{
						// found start code prefix
						startPrefixFound = true;
						// set the file pointer to before the prefix:
						fPtr += i;
						fseek(input, fPtr, SEEK_SET);
						break;
					}
				}
			}
		}

		if (startPrefixFound) break;

		// Start code not found in this
		// access, set the new fPtr position.
		// -2 prevents missing the start code
		// if it's on the border between two
		// file accesses
		fPtr += BUFFER_SIZE - 2;
		fseek(input, fPtr, SEEK_SET);
	}

	if (startPrefixFound == false)
	{
		fPtr += bytesRead;
	}

	return fPtr;
}

NALunit parseNAL(unsigned char *NALbytes, unsigned int NumBytesInNALunit)
{
	NALunit nal_unit;
	nal_unit.forbidden_zero_bit = (bool) (NALbytes[0] >> 7);
	nal_unit.nal_ref_idc = (NALbytes[0] & 0x7f) >> 5;
	nal_unit.nal_unit_type = NALbytes[0] & 0x1f;

	nal_unit.NumBytesInRBSP = 0;
	unsigned int nalUnitHeaderBytes = 1;

	if ((nal_unit.nal_unit_type == 14) ||
		(nal_unit.nal_unit_type == 20))
	{
		perror("NAL unit types 14 and 20 not supported yet.");
		system("pause");
		exit(1);
	}

	nal_unit.rbsp_byte = (unsigned char*) malloc(NumBytesInNALunit);
	if (nal_unit.rbsp_byte == NULL)
	{
		perror("Error allocating memory for RBSP.");
		system("pause");
		exit(1);
	}
	for (unsigned int i = nalUnitHeaderBytes; i < NumBytesInNALunit; i++)
	{
		unsigned int next_bits24 = (unsigned int)NALbytes[i]   << 16 |
								   (unsigned int)NALbytes[i+1] <<  8 |
								   (unsigned int)NALbytes[i+2];
		if ((i+2 < NumBytesInNALunit)  &&
			(next_bits24 == 0x03))
		{
			nal_unit.rbsp_byte[nal_unit.NumBytesInRBSP++] = NALbytes[i];
			nal_unit.rbsp_byte[nal_unit.NumBytesInRBSP++] = NALbytes[i+1];
			i += 2;
			// skip the emulation prevention byte
		}
		else
		{
			nal_unit.rbsp_byte[nal_unit.NumBytesInRBSP++] = NALbytes[i];
		}
	}
	return nal_unit;
}

NALunit getNAL(FILE *input, unsigned long *fPtr)
{
	unsigned long startPtr = findNALstart(input, *fPtr);

	if (startPtr == 0)
	{
		printf("\n\n====================================================\n");
		printf("End of stream found.\n");
		system("pause");
		exit(0);	// the 0 error code indicates that this is not an error
	}

	unsigned long endPtr = findNALend(input, startPtr);
	unsigned int NumBytesInNALunit = endPtr - startPtr;

	printf("\n\n====================================================\n");
	printf("NAL start found at %d (0x%x)\n", startPtr, startPtr);
	printf("NAL size = %d (0x%x)\n\n", NumBytesInNALunit, NumBytesInNALunit);

	unsigned char *NALbytes;
	//NALbytes = (unsigned char*) malloc(NumBytesInNALunit);
	NALbytes = new unsigned char[NumBytesInNALunit];
	if (NALbytes == NULL)
	{
		perror("Error allocating memory for NAL unit.\n");
		system("pause");
		exit(1);
	}

	fseek(input, startPtr, SEEK_SET);
	int test;
	if ((test = fread(NALbytes, 1, NumBytesInNALunit, input)) != NumBytesInNALunit)
	{
		//perror("Error reading NAL unit from file.\n");
		//system("pause");
		//exit(1);

		// TEST: MOZK
		printf("\nEnd of stream found.\n");
		system("pause");
		exit(0);
	}

	*fPtr = endPtr;

	NALunit nu = parseNAL(NALbytes, NumBytesInNALunit);
	free(NALbytes);
	return nu;
}