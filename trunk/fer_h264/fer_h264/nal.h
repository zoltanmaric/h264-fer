#pragma once
#include "stdafx.h"

const unsigned int BUFFER_SIZE = 1024; // the number of bytes read from the file in one access

typedef struct
{
	bool forbidden_zero_bit;
	unsigned int nal_ref_idc, nal_unit_type, NumBytesInRBSP;
	unsigned char *rbsp_byte;
}
NALunit;

// Returns the address of the subsequent NAL unit in the 
// input file.
// fPtr - the current position in the input file
unsigned long findNALstart(FILE *input, unsigned long fPtr);

// Returns the address of the position after the
// last byte of this NAL (the address of a
// subsequent NAL start code)
// fPtr - the current position in the input file
unsigned long findNALend(FILE *input, unsigned long fPtr);

// This function implements quite literally the
// NAL unit syntax found in chapter 7.3.1 of
// the H.264 specification.
// *NALbytes: an array of bytes representing
// one NAL unit read from the input stream.
NALunit parseNAL(unsigned char *NALbytes, unsigned int NumBytesInNALunit);

// Returns the NAL unit data in a NALunit structure.
// fPtr - the current position in the input file
NALunit getNAL(FILE *input, unsigned long *fPtr);