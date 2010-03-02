#include "residual.h"
#include "rawreader.h"
#include "h264_globals.h"
#include "headers_and_parameter_sets.h"

bool ChromaDCLevel_active;

//Coefficient levels

int i16x16DClevel[16];
int i16x16AClevel[16][16];
int Intra16x16DCLevel[16];
int Intra16x16ACLevel[16][16];
int level[16][16], LumaLevel[16][16];

int NumC8x8;

int ChromaDCLevel[2][4];
int ChromaACLevel[2][4][16];

int coeffLevel_luma_DC[16];
int coeffLevel_luma_AC[16][16];
int coeffLevel_chroma_DC[2][4];
int coeffLevel_chroma_AC[2][4][16];

//TODO: Niz tipa mb_width*mb_height*16 integera NETOCNO, izrazeni u transform/colour blokovima
//Number of non-zero coefficients in luma and chroma blocks (integer from 0 to 16)
int **TotalCoeff_luma_array;
int ***TotalCoeff_chroma_array;

struct cavlc_table *CoeffTokenCodeTable[4];
struct cavlc_table *CoeffTokenCodeTable_ChromaDC;
struct cavlc_table *TotalZerosCodeTable_4x4[15];
struct cavlc_table *TotalZerosCodeTable_ChromaDC[3];
struct cavlc_table *RunBeforeCodeTable[6];


int CoeffTokenCodes[4][64][3]={ {
///// 0  <=  nC  <  2 /////
  { 0x00000000,  0, 0 },  // BOT
  { 0x00020000, 15, COEFF_TOKEN(1,13) },  // 0000 0000 0000 001
  { 0x00040000, 16, COEFF_TOKEN(0,16) },  // 0000 0000 0000 0100
  { 0x00050000, 16, COEFF_TOKEN(2,16) },  // 0000 0000 0000 0101
  { 0x00060000, 16, COEFF_TOKEN(1,16) },  // 0000 0000 0000 0110
  { 0x00070000, 16, COEFF_TOKEN(0,15) },  // 0000 0000 0000 0111
  { 0x00080000, 16, COEFF_TOKEN(3,16) },  // 0000 0000 0000 1000
  { 0x00090000, 16, COEFF_TOKEN(2,15) },  // 0000 0000 0000 1001
  { 0x000A0000, 16, COEFF_TOKEN(1,15) },  // 0000 0000 0000 1010
  { 0x000B0000, 16, COEFF_TOKEN(0,14) },  // 0000 0000 0000 1011
  { 0x000C0000, 16, COEFF_TOKEN(3,15) },  // 0000 0000 0000 1100
  { 0x000D0000, 16, COEFF_TOKEN(2,14) },  // 0000 0000 0000 1101
  { 0x000E0000, 16, COEFF_TOKEN(1,14) },  // 0000 0000 0000 1110
  { 0x000F0000, 16, COEFF_TOKEN(0,13) },  // 0000 0000 0000 1111
  { 0x00100000, 15, COEFF_TOKEN(3,14) },  // 0000 0000 0001 000
  { 0x00120000, 15, COEFF_TOKEN(2,13) },  // 0000 0000 0001 001
  { 0x00140000, 15, COEFF_TOKEN(1,12) },  // 0000 0000 0001 010
  { 0x00160000, 15, COEFF_TOKEN(0,12) },  // 0000 0000 0001 011
  { 0x00180000, 15, COEFF_TOKEN(3,13) },  // 0000 0000 0001 100
  { 0x001A0000, 15, COEFF_TOKEN(2,12) },  // 0000 0000 0001 101
  { 0x001C0000, 15, COEFF_TOKEN(1,11) },  // 0000 0000 0001 110
  { 0x001E0000, 15, COEFF_TOKEN(0,11) },  // 0000 0000 0001 111
  { 0x00200000, 14, COEFF_TOKEN(3,12) },  // 0000 0000 0010 00
  { 0x00240000, 14, COEFF_TOKEN(2,11) },  // 0000 0000 0010 01
  { 0x00280000, 14, COEFF_TOKEN(1,10) },  // 0000 0000 0010 10
  { 0x002C0000, 14, COEFF_TOKEN(0,10) },  // 0000 0000 0010 11
  { 0x00300000, 14, COEFF_TOKEN(3,11) },  // 0000 0000 0011 00
  { 0x00340000, 14, COEFF_TOKEN(2,10) },  // 0000 0000 0011 01
  { 0x00380000, 14, COEFF_TOKEN(1, 9) },  // 0000 0000 0011 10
  { 0x003C0000, 14, COEFF_TOKEN(0, 9) },  // 0000 0000 0011 11
  { 0x00400000, 13, COEFF_TOKEN(0, 8) },  // 0000 0000 0100 0
  { 0x00480000, 13, COEFF_TOKEN(2, 9) },  // 0000 0000 0100 1
  { 0x00500000, 13, COEFF_TOKEN(1, 8) },  // 0000 0000 0101 0
  { 0x00580000, 13, COEFF_TOKEN(0, 7) },  // 0000 0000 0101 1
  { 0x00600000, 13, COEFF_TOKEN(3,10) },  // 0000 0000 0110 0
  { 0x00680000, 13, COEFF_TOKEN(2, 8) },  // 0000 0000 0110 1
  { 0x00700000, 13, COEFF_TOKEN(1, 7) },  // 0000 0000 0111 0
  { 0x00780000, 13, COEFF_TOKEN(0, 6) },  // 0000 0000 0111 1
  { 0x00800000, 11, COEFF_TOKEN(3, 9) },  // 0000 0000 100
  { 0x00A00000, 11, COEFF_TOKEN(2, 7) },  // 0000 0000 101
  { 0x00C00000, 11, COEFF_TOKEN(1, 6) },  // 0000 0000 110
  { 0x00E00000, 11, COEFF_TOKEN(0, 5) },  // 0000 0000 111
  { 0x01000000, 10, COEFF_TOKEN(3, 8) },  // 0000 0001 00
  { 0x01400000, 10, COEFF_TOKEN(2, 6) },  // 0000 0001 01
  { 0x01800000, 10, COEFF_TOKEN(1, 5) },  // 0000 0001 10
  { 0x01C00000, 10, COEFF_TOKEN(0, 4) },  // 0000 0001 11
  { 0x02000000,  9, COEFF_TOKEN(3, 7) },  // 0000 0010 0
  { 0x02800000,  9, COEFF_TOKEN(2, 5) },  // 0000 0010 1
  { 0x03000000,  9, COEFF_TOKEN(1, 4) },  // 0000 0011 0
  { 0x03800000,  9, COEFF_TOKEN(0, 3) },  // 0000 0011 1
  { 0x04000000,  8, COEFF_TOKEN(3, 6) },  // 0000 0100
  { 0x05000000,  8, COEFF_TOKEN(2, 4) },  // 0000 0101
  { 0x06000000,  8, COEFF_TOKEN(1, 3) },  // 0000 0110
  { 0x07000000,  8, COEFF_TOKEN(0, 2) },  // 0000 0111
  { 0x08000000,  7, COEFF_TOKEN(3, 5) },  // 0000 100
  { 0x0A000000,  7, COEFF_TOKEN(2, 3) },  // 0000 101
  { 0x0C000000,  6, COEFF_TOKEN(3, 4) },  // 0000 11
  { 0x10000000,  6, COEFF_TOKEN(1, 2) },  // 0001 00
  { 0x14000000,  6, COEFF_TOKEN(0, 1) },  // 0001 01
  { 0x18000000,  5, COEFF_TOKEN(3, 3) },  // 0001 1
  { 0x20000000,  3, COEFF_TOKEN(2, 2) },  // 001
  { 0x40000000,  2, COEFF_TOKEN(1, 1) },  // 01
  { 0x80000000,  1, COEFF_TOKEN(0, 0) },  // 1
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 2  <=  nC  <  4 /////
  { 0x00000000,  0, 0 },  // BOT
  { 0x00080000, 13, COEFF_TOKEN(3,15) },  // 0000 0000 0000 1
  { 0x00100000, 14, COEFF_TOKEN(3,16) },  // 0000 0000 0001 00
  { 0x00140000, 14, COEFF_TOKEN(2,16) },  // 0000 0000 0001 01
  { 0x00180000, 14, COEFF_TOKEN(1,16) },  // 0000 0000 0001 10
  { 0x001C0000, 14, COEFF_TOKEN(0,16) },  // 0000 0000 0001 11
  { 0x00200000, 14, COEFF_TOKEN(1,15) },  // 0000 0000 0010 00
  { 0x00240000, 14, COEFF_TOKEN(0,15) },  // 0000 0000 0010 01
  { 0x00280000, 14, COEFF_TOKEN(2,15) },  // 0000 0000 0010 10
  { 0x002C0000, 14, COEFF_TOKEN(1,14) },  // 0000 0000 0010 11
  { 0x00300000, 13, COEFF_TOKEN(2,14) },  // 0000 0000 0011 0
  { 0x00380000, 13, COEFF_TOKEN(0,14) },  // 0000 0000 0011 1
  { 0x00400000, 13, COEFF_TOKEN(3,14) },  // 0000 0000 0100 0
  { 0x00480000, 13, COEFF_TOKEN(2,13) },  // 0000 0000 0100 1
  { 0x00500000, 13, COEFF_TOKEN(1,13) },  // 0000 0000 0101 0
  { 0x00580000, 13, COEFF_TOKEN(0,13) },  // 0000 0000 0101 1
  { 0x00600000, 13, COEFF_TOKEN(3,13) },  // 0000 0000 0110 0
  { 0x00680000, 13, COEFF_TOKEN(2,12) },  // 0000 0000 0110 1
  { 0x00700000, 13, COEFF_TOKEN(1,12) },  // 0000 0000 0111 0
  { 0x00780000, 13, COEFF_TOKEN(0,12) },  // 0000 0000 0111 1
  { 0x00800000, 12, COEFF_TOKEN(0,11) },  // 0000 0000 1000
  { 0x00900000, 12, COEFF_TOKEN(2,11) },  // 0000 0000 1001
  { 0x00A00000, 12, COEFF_TOKEN(1,11) },  // 0000 0000 1010
  { 0x00B00000, 12, COEFF_TOKEN(0,10) },  // 0000 0000 1011
  { 0x00C00000, 12, COEFF_TOKEN(3,12) },  // 0000 0000 1100
  { 0x00D00000, 12, COEFF_TOKEN(2,10) },  // 0000 0000 1101
  { 0x00E00000, 12, COEFF_TOKEN(1,10) },  // 0000 0000 1110
  { 0x00F00000, 12, COEFF_TOKEN(0, 9) },  // 0000 0000 1111
  { 0x01000000, 11, COEFF_TOKEN(3,11) },  // 0000 0001 000
  { 0x01200000, 11, COEFF_TOKEN(2, 9) },  // 0000 0001 001
  { 0x01400000, 11, COEFF_TOKEN(1, 9) },  // 0000 0001 010
  { 0x01600000, 11, COEFF_TOKEN(0, 8) },  // 0000 0001 011
  { 0x01800000, 11, COEFF_TOKEN(3,10) },  // 0000 0001 100
  { 0x01A00000, 11, COEFF_TOKEN(2, 8) },  // 0000 0001 101
  { 0x01C00000, 11, COEFF_TOKEN(1, 8) },  // 0000 0001 110
  { 0x01E00000, 11, COEFF_TOKEN(0, 7) },  // 0000 0001 111
  { 0x02000000,  9, COEFF_TOKEN(3, 9) },  // 0000 0010 0
  { 0x02800000,  9, COEFF_TOKEN(2, 7) },  // 0000 0010 1
  { 0x03000000,  9, COEFF_TOKEN(1, 7) },  // 0000 0011 0
  { 0x03800000,  9, COEFF_TOKEN(0, 6) },  // 0000 0011 1
  { 0x04000000,  8, COEFF_TOKEN(0, 5) },  // 0000 0100
  { 0x05000000,  8, COEFF_TOKEN(2, 6) },  // 0000 0101
  { 0x06000000,  8, COEFF_TOKEN(1, 6) },  // 0000 0110
  { 0x07000000,  8, COEFF_TOKEN(0, 4) },  // 0000 0111
  { 0x08000000,  7, COEFF_TOKEN(3, 8) },  // 0000 100
  { 0x0A000000,  7, COEFF_TOKEN(2, 5) },  // 0000 101
  { 0x0C000000,  7, COEFF_TOKEN(1, 5) },  // 0000 110
  { 0x0E000000,  7, COEFF_TOKEN(0, 3) },  // 0000 111
  { 0x10000000,  6, COEFF_TOKEN(3, 7) },  // 0001 00
  { 0x14000000,  6, COEFF_TOKEN(2, 4) },  // 0001 01
  { 0x18000000,  6, COEFF_TOKEN(1, 4) },  // 0001 10
  { 0x1C000000,  6, COEFF_TOKEN(0, 2) },  // 0001 11
  { 0x20000000,  6, COEFF_TOKEN(3, 6) },  // 0010 00
  { 0x24000000,  6, COEFF_TOKEN(2, 3) },  // 0010 01
  { 0x28000000,  6, COEFF_TOKEN(1, 3) },  // 0010 10
  { 0x2C000000,  6, COEFF_TOKEN(0, 1) },  // 0010 11
  { 0x30000000,  5, COEFF_TOKEN(3, 5) },  // 0011 0
  { 0x38000000,  5, COEFF_TOKEN(1, 2) },  // 0011 1
  { 0x40000000,  4, COEFF_TOKEN(3, 4) },  // 0100
  { 0x50000000,  4, COEFF_TOKEN(3, 3) },  // 0101
  { 0x60000000,  3, COEFF_TOKEN(2, 2) },  // 011
  { 0x80000000,  2, COEFF_TOKEN(1, 1) },  // 10
  { 0xC0000000,  2, COEFF_TOKEN(0, 0) },  // 11
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 4  <=  nC  <  8 /////
  { 0x00000000,  0, 0 },  // BOT
  { 0x00400000, 10, COEFF_TOKEN(0,16) },  // 0000 0000 01
  { 0x00800000, 10, COEFF_TOKEN(3,16) },  // 0000 0000 10
  { 0x00C00000, 10, COEFF_TOKEN(2,16) },  // 0000 0000 11
  { 0x01000000, 10, COEFF_TOKEN(1,16) },  // 0000 0001 00
  { 0x01400000, 10, COEFF_TOKEN(0,15) },  // 0000 0001 01
  { 0x01800000, 10, COEFF_TOKEN(3,15) },  // 0000 0001 10
  { 0x01C00000, 10, COEFF_TOKEN(2,15) },  // 0000 0001 11
  { 0x02000000, 10, COEFF_TOKEN(1,15) },  // 0000 0010 00
  { 0x02400000, 10, COEFF_TOKEN(0,14) },  // 0000 0010 01
  { 0x02800000, 10, COEFF_TOKEN(3,14) },  // 0000 0010 10
  { 0x02C00000, 10, COEFF_TOKEN(2,14) },  // 0000 0010 11
  { 0x03000000, 10, COEFF_TOKEN(1,14) },  // 0000 0011 00
  { 0x03400000, 10, COEFF_TOKEN(0,13) },  // 0000 0011 01
  { 0x03800000,  9, COEFF_TOKEN(1,13) },  // 0000 0011 1
  { 0x04000000,  9, COEFF_TOKEN(0,12) },  // 0000 0100 0
  { 0x04800000,  9, COEFF_TOKEN(2,13) },  // 0000 0100 1
  { 0x05000000,  9, COEFF_TOKEN(1,12) },  // 0000 0101 0
  { 0x05800000,  9, COEFF_TOKEN(0,11) },  // 0000 0101 1
  { 0x06000000,  9, COEFF_TOKEN(3,13) },  // 0000 0110 0
  { 0x06800000,  9, COEFF_TOKEN(2,12) },  // 0000 0110 1
  { 0x07000000,  9, COEFF_TOKEN(1,11) },  // 0000 0111 0
  { 0x07800000,  9, COEFF_TOKEN(0,10) },  // 0000 0111 1
  { 0x08000000,  8, COEFF_TOKEN(3,12) },  // 0000 1000
  { 0x09000000,  8, COEFF_TOKEN(2,11) },  // 0000 1001
  { 0x0A000000,  8, COEFF_TOKEN(1,10) },  // 0000 1010
  { 0x0B000000,  8, COEFF_TOKEN(0, 9) },  // 0000 1011
  { 0x0C000000,  8, COEFF_TOKEN(3,11) },  // 0000 1100
  { 0x0D000000,  8, COEFF_TOKEN(2,10) },  // 0000 1101
  { 0x0E000000,  8, COEFF_TOKEN(1, 9) },  // 0000 1110
  { 0x0F000000,  8, COEFF_TOKEN(0, 8) },  // 0000 1111
  { 0x10000000,  7, COEFF_TOKEN(0, 7) },  // 0001 000
  { 0x12000000,  7, COEFF_TOKEN(0, 6) },  // 0001 001
  { 0x14000000,  7, COEFF_TOKEN(2, 9) },  // 0001 010
  { 0x16000000,  7, COEFF_TOKEN(0, 5) },  // 0001 011
  { 0x18000000,  7, COEFF_TOKEN(3,10) },  // 0001 100
  { 0x1A000000,  7, COEFF_TOKEN(2, 8) },  // 0001 101
  { 0x1C000000,  7, COEFF_TOKEN(1, 8) },  // 0001 110
  { 0x1E000000,  7, COEFF_TOKEN(0, 4) },  // 0001 111
  { 0x20000000,  6, COEFF_TOKEN(0, 3) },  // 0010 00
  { 0x24000000,  6, COEFF_TOKEN(2, 7) },  // 0010 01
  { 0x28000000,  6, COEFF_TOKEN(1, 7) },  // 0010 10
  { 0x2C000000,  6, COEFF_TOKEN(0, 2) },  // 0010 11
  { 0x30000000,  6, COEFF_TOKEN(3, 9) },  // 0011 00
  { 0x34000000,  6, COEFF_TOKEN(2, 6) },  // 0011 01
  { 0x38000000,  6, COEFF_TOKEN(1, 6) },  // 0011 10
  { 0x3C000000,  6, COEFF_TOKEN(0, 1) },  // 0011 11
  { 0x40000000,  5, COEFF_TOKEN(1, 5) },  // 0100 0
  { 0x48000000,  5, COEFF_TOKEN(2, 5) },  // 0100 1
  { 0x50000000,  5, COEFF_TOKEN(1, 4) },  // 0101 0
  { 0x58000000,  5, COEFF_TOKEN(2, 4) },  // 0101 1
  { 0x60000000,  5, COEFF_TOKEN(1, 3) },  // 0110 0
  { 0x68000000,  5, COEFF_TOKEN(3, 8) },  // 0110 1
  { 0x70000000,  5, COEFF_TOKEN(2, 3) },  // 0111 0
  { 0x78000000,  5, COEFF_TOKEN(1, 2) },  // 0111 1
  { 0x80000000,  4, COEFF_TOKEN(3, 7) },  // 1000
  { 0x90000000,  4, COEFF_TOKEN(3, 6) },  // 1001
  { 0xA0000000,  4, COEFF_TOKEN(3, 5) },  // 1010
  { 0xB0000000,  4, COEFF_TOKEN(3, 4) },  // 1011
  { 0xC0000000,  4, COEFF_TOKEN(3, 3) },  // 1100
  { 0xD0000000,  4, COEFF_TOKEN(2, 2) },  // 1101
  { 0xE0000000,  4, COEFF_TOKEN(1, 1) },  // 1110
  { 0xF0000000,  4, COEFF_TOKEN(0, 0) },  // 1111
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 8  <=  nC /////
  { 0x00000000,  6, COEFF_TOKEN(0, 1) },  // 0000 00
  { 0x04000000,  6, COEFF_TOKEN(1, 1) },  // 0000 01
  { 0x0C000000,  6, COEFF_TOKEN(0, 0) },  // 0000 11
  { 0x10000000,  6, COEFF_TOKEN(0, 2) },  // 0001 00
  { 0x14000000,  6, COEFF_TOKEN(1, 2) },  // 0001 01
  { 0x18000000,  6, COEFF_TOKEN(2, 2) },  // 0001 10
  { 0x20000000,  6, COEFF_TOKEN(0, 3) },  // 0010 00
  { 0x24000000,  6, COEFF_TOKEN(1, 3) },  // 0010 01
  { 0x28000000,  6, COEFF_TOKEN(2, 3) },  // 0010 10
  { 0x2C000000,  6, COEFF_TOKEN(3, 3) },  // 0010 11
  { 0x30000000,  6, COEFF_TOKEN(0, 4) },  // 0011 00
  { 0x34000000,  6, COEFF_TOKEN(1, 4) },  // 0011 01
  { 0x38000000,  6, COEFF_TOKEN(2, 4) },  // 0011 10
  { 0x3C000000,  6, COEFF_TOKEN(3, 4) },  // 0011 11
  { 0x40000000,  6, COEFF_TOKEN(0, 5) },  // 0100 00
  { 0x44000000,  6, COEFF_TOKEN(1, 5) },  // 0100 01
  { 0x48000000,  6, COEFF_TOKEN(2, 5) },  // 0100 10
  { 0x4C000000,  6, COEFF_TOKEN(3, 5) },  // 0100 11
  { 0x50000000,  6, COEFF_TOKEN(0, 6) },  // 0101 00
  { 0x54000000,  6, COEFF_TOKEN(1, 6) },  // 0101 01
  { 0x58000000,  6, COEFF_TOKEN(2, 6) },  // 0101 10
  { 0x5C000000,  6, COEFF_TOKEN(3, 6) },  // 0101 11
  { 0x60000000,  6, COEFF_TOKEN(0, 7) },  // 0110 00
  { 0x64000000,  6, COEFF_TOKEN(1, 7) },  // 0110 01
  { 0x68000000,  6, COEFF_TOKEN(2, 7) },  // 0110 10
  { 0x6C000000,  6, COEFF_TOKEN(3, 7) },  // 0110 11
  { 0x70000000,  6, COEFF_TOKEN(0, 8) },  // 0111 00
  { 0x74000000,  6, COEFF_TOKEN(1, 8) },  // 0111 01
  { 0x78000000,  6, COEFF_TOKEN(2, 8) },  // 0111 10
  { 0x7C000000,  6, COEFF_TOKEN(3, 8) },  // 0111 11
  { 0x80000000,  6, COEFF_TOKEN(0, 9) },  // 1000 00
  { 0x84000000,  6, COEFF_TOKEN(1, 9) },  // 1000 01
  { 0x88000000,  6, COEFF_TOKEN(2, 9) },  // 1000 10
  { 0x8C000000,  6, COEFF_TOKEN(3, 9) },  // 1000 11
  { 0x90000000,  6, COEFF_TOKEN(0,10) },  // 1001 00
  { 0x94000000,  6, COEFF_TOKEN(1,10) },  // 1001 01
  { 0x98000000,  6, COEFF_TOKEN(2,10) },  // 1001 10
  { 0x9C000000,  6, COEFF_TOKEN(3,10) },  // 1001 11
  { 0xA0000000,  6, COEFF_TOKEN(0,11) },  // 1010 00
  { 0xA4000000,  6, COEFF_TOKEN(1,11) },  // 1010 01
  { 0xA8000000,  6, COEFF_TOKEN(2,11) },  // 1010 10
  { 0xAC000000,  6, COEFF_TOKEN(3,11) },  // 1010 11
  { 0xB0000000,  6, COEFF_TOKEN(0,12) },  // 1011 00
  { 0xB4000000,  6, COEFF_TOKEN(1,12) },  // 1011 01
  { 0xB8000000,  6, COEFF_TOKEN(2,12) },  // 1011 10
  { 0xBC000000,  6, COEFF_TOKEN(3,12) },  // 1011 11
  { 0xC0000000,  6, COEFF_TOKEN(0,13) },  // 1100 00
  { 0xC4000000,  6, COEFF_TOKEN(1,13) },  // 1100 01
  { 0xC8000000,  6, COEFF_TOKEN(2,13) },  // 1100 10
  { 0xCC000000,  6, COEFF_TOKEN(3,13) },  // 1100 11
  { 0xD0000000,  6, COEFF_TOKEN(0,14) },  // 1101 00
  { 0xD4000000,  6, COEFF_TOKEN(1,14) },  // 1101 01
  { 0xD8000000,  6, COEFF_TOKEN(2,14) },  // 1101 10
  { 0xDC000000,  6, COEFF_TOKEN(3,14) },  // 1101 11
  { 0xE0000000,  6, COEFF_TOKEN(0,15) },  // 1110 00
  { 0xE4000000,  6, COEFF_TOKEN(1,15) },  // 1110 01
  { 0xE8000000,  6, COEFF_TOKEN(2,15) },  // 1110 10
  { 0xEC000000,  6, COEFF_TOKEN(3,15) },  // 1110 11
  { 0xF0000000,  6, COEFF_TOKEN(0,16) },  // 1111 00
  { 0xF4000000,  6, COEFF_TOKEN(1,16) },  // 1111 01
  { 0xF8000000,  6, COEFF_TOKEN(2,16) },  // 1111 10
  { 0xFC000000,  6, COEFF_TOKEN(3,16) },  // 1111 11
  { 0xFFFFFFFF,  0, 0 }  // EOT
} };


int CoeffTokenCodes_ChromaDC[15][3]={
  { 0x00000000,  7, COEFF_TOKEN(3, 4) },  // 0000 000
  { 0x02000000,  8, COEFF_TOKEN(2, 4) },  // 0000 0010
  { 0x03000000,  8, COEFF_TOKEN(1, 4) },  // 0000 0011
  { 0x04000000,  7, COEFF_TOKEN(2, 3) },  // 0000 010
  { 0x06000000,  7, COEFF_TOKEN(1, 3) },  // 0000 011
  { 0x08000000,  6, COEFF_TOKEN(0, 4) },  // 0000 10
  { 0x0C000000,  6, COEFF_TOKEN(0, 3) },  // 0000 11
  { 0x10000000,  6, COEFF_TOKEN(0, 2) },  // 0001 00
  { 0x14000000,  6, COEFF_TOKEN(3, 3) },  // 0001 01
  { 0x18000000,  6, COEFF_TOKEN(1, 2) },  // 0001 10
  { 0x1C000000,  6, COEFF_TOKEN(0, 1) },  // 0001 11
  { 0x20000000,  3, COEFF_TOKEN(2, 2) },  // 001
  { 0x40000000,  2, COEFF_TOKEN(0, 0) },  // 01
  { 0x80000000,  1, COEFF_TOKEN(1, 1) },  // 1
  { 0xFFFFFFFF,  0, 0 }  // EOT
};


int TotalZerosCodes_4x4[15][18][3]={ {
///// 1 /////
  { 0x00000000,  0, 0 },  // BOT
  { 0x00800000,  9, 15 },  // 0000 0000 1
  { 0x01000000,  9, 14 },  // 0000 0001 0
  { 0x01800000,  9, 13 },  // 0000 0001 1
  { 0x02000000,  8, 12 },  // 0000 0010
  { 0x03000000,  8, 11 },  // 0000 0011
  { 0x04000000,  7, 10 },  // 0000 010
  { 0x06000000,  7,  9 },  // 0000 011
  { 0x08000000,  6,  8 },  // 0000 10
  { 0x0C000000,  6,  7 },  // 0000 11
  { 0x10000000,  5,  6 },  // 0001 0
  { 0x18000000,  5,  5 },  // 0001 1
  { 0x20000000,  4,  4 },  // 0010
  { 0x30000000,  4,  3 },  // 0011
  { 0x40000000,  3,  2 },  // 010
  { 0x60000000,  3,  1 },  // 011
  { 0x80000000,  1,  0 },  // 1
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 2 /////
  { 0x00000000,  6, 14 },  // 0000 00
  { 0x04000000,  6, 13 },  // 0000 01
  { 0x08000000,  6, 12 },  // 0000 10
  { 0x0C000000,  6, 11 },  // 0000 11
  { 0x10000000,  5, 10 },  // 0001 0
  { 0x18000000,  5,  9 },  // 0001 1
  { 0x20000000,  4,  8 },  // 0010
  { 0x30000000,  4,  7 },  // 0011
  { 0x40000000,  4,  6 },  // 0100
  { 0x50000000,  4,  5 },  // 0101
  { 0x60000000,  3,  4 },  // 011
  { 0x80000000,  3,  3 },  // 100
  { 0xA0000000,  3,  2 },  // 101
  { 0xC0000000,  3,  1 },  // 110
  { 0xE0000000,  3,  0 },  // 111
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 3 /////
  { 0x00000000,  6, 13 },  // 0000 00
  { 0x04000000,  6, 11 },  // 0000 01
  { 0x08000000,  5, 12 },  // 0000 1
  { 0x10000000,  5, 10 },  // 0001 0
  { 0x18000000,  5,  9 },  // 0001 1
  { 0x20000000,  4,  8 },  // 0010
  { 0x30000000,  4,  5 },  // 0011
  { 0x40000000,  4,  4 },  // 0100
  { 0x50000000,  4,  0 },  // 0101
  { 0x60000000,  3,  7 },  // 011
  { 0x80000000,  3,  6 },  // 100
  { 0xA0000000,  3,  3 },  // 101
  { 0xC0000000,  3,  2 },  // 110
  { 0xE0000000,  3,  1 },  // 111
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 4 /////
  { 0x00000000,  5, 12 },  // 0000 0
  { 0x08000000,  5, 11 },  // 0000 1
  { 0x10000000,  5, 10 },  // 0001 0
  { 0x18000000,  5,  0 },  // 0001 1
  { 0x20000000,  4,  9 },  // 0010
  { 0x30000000,  4,  7 },  // 0011
  { 0x40000000,  4,  3 },  // 0100
  { 0x50000000,  4,  2 },  // 0101
  { 0x60000000,  3,  8 },  // 011
  { 0x80000000,  3,  6 },  // 100
  { 0xA0000000,  3,  5 },  // 101
  { 0xC0000000,  3,  4 },  // 110
  { 0xE0000000,  3,  1 },  // 111
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 5 /////
  { 0x00000000,  5, 11 },  // 0000 0
  { 0x08000000,  5,  9 },  // 0000 1
  { 0x10000000,  4, 10 },  // 0001
  { 0x20000000,  4,  8 },  // 0010
  { 0x30000000,  4,  2 },  // 0011
  { 0x40000000,  4,  1 },  // 0100
  { 0x50000000,  4,  0 },  // 0101
  { 0x60000000,  3,  7 },  // 011
  { 0x80000000,  3,  6 },  // 100
  { 0xA0000000,  3,  5 },  // 101
  { 0xC0000000,  3,  4 },  // 110
  { 0xE0000000,  3,  3 },  // 111
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 6 /////
  { 0x00000000,  6, 10 },  // 0000 00
  { 0x04000000,  6,  0 },  // 0000 01
  { 0x08000000,  5,  1 },  // 0000 1
  { 0x10000000,  4,  8 },  // 0001
  { 0x20000000,  3,  9 },  // 001
  { 0x40000000,  3,  7 },  // 010
  { 0x60000000,  3,  6 },  // 011
  { 0x80000000,  3,  5 },  // 100
  { 0xA0000000,  3,  4 },  // 101
  { 0xC0000000,  3,  3 },  // 110
  { 0xE0000000,  3,  2 },  // 111
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 7 /////
  { 0x00000000,  6, 9 },  // 0000 00
  { 0x04000000,  6, 0 },  // 0000 01
  { 0x08000000,  5, 1 },  // 0000 1
  { 0x10000000,  4, 7 },  // 0001
  { 0x20000000,  3, 8 },  // 001
  { 0x40000000,  3, 6 },  // 010
  { 0x60000000,  3, 4 },  // 011
  { 0x80000000,  3, 3 },  // 100
  { 0xA0000000,  3, 2 },  // 101
  { 0xC0000000,  2, 5 },  // 11
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 8 /////
  { 0x00000000,  6, 8 },  // 0000 00
  { 0x04000000,  6, 0 },  // 0000 01
  { 0x08000000,  5, 2 },  // 0000 1
  { 0x10000000,  4, 1 },  // 0001
  { 0x20000000,  3, 7 },  // 001
  { 0x40000000,  3, 6 },  // 010
  { 0x60000000,  3, 3 },  // 011
  { 0x80000000,  2, 5 },  // 10
  { 0xC0000000,  2, 4 },  // 11
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 9 /////
  { 0x00000000,  6, 1 },  // 0000 00
  { 0x04000000,  6, 0 },  // 0000 01
  { 0x08000000,  5, 7 },  // 0000 1
  { 0x10000000,  4, 2 },  // 0001
  { 0x20000000,  3, 5 },  // 001
  { 0x40000000,  2, 6 },  // 01
  { 0x80000000,  2, 4 },  // 10
  { 0xC0000000,  2, 3 },  // 11
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 10 /////
  { 0x00000000,  5, 1 },  // 0000 0
  { 0x08000000,  5, 0 },  // 0000 1
  { 0x10000000,  4, 6 },  // 0001
  { 0x20000000,  3, 2 },  // 001
  { 0x40000000,  2, 5 },  // 01
  { 0x80000000,  2, 4 },  // 10
  { 0xC0000000,  2, 3 },  // 11
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 11 /////
  { 0x00000000,  4, 0 },  // 0000
  { 0x10000000,  4, 1 },  // 0001
  { 0x20000000,  3, 2 },  // 001
  { 0x40000000,  3, 3 },  // 010
  { 0x60000000,  3, 5 },  // 011
  { 0x80000000,  1, 4 },  // 1
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 12 /////
  { 0x00000000,  4, 0 },  // 0000
  { 0x10000000,  4, 1 },  // 0001
  { 0x20000000,  3, 4 },  // 001
  { 0x40000000,  2, 2 },  // 01
  { 0x80000000,  1, 3 },  // 1
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 13 /////
  { 0x00000000,  3, 0 },  // 000
  { 0x20000000,  3, 1 },  // 001
  { 0x40000000,  2, 3 },  // 01
  { 0x80000000,  1, 2 },  // 1
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 14 /////
  { 0x00000000,  2, 0 },  // 00
  { 0x40000000,  2, 1 },  // 01
  { 0x80000000,  1, 2 },  // 1
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 15 /////
  { 0x00000000,  1, 0 },  // 0
  { 0x80000000,  1, 1 },  // 1
  { 0xFFFFFFFF,  0, 0 }  // EOT
} };


int TotalZerosCodes_ChromaDC[3][5][3]={ {
///// 1 /////
  { 0x00000000,  3, 3 },  // 000
  { 0x20000000,  3, 2 },  // 001
  { 0x40000000,  2, 1 },  // 01
  { 0x80000000,  1, 0 },  // 1
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 2 /////
  { 0x00000000,  2, 2 },  // 00
  { 0x40000000,  2, 1 },  // 01
  { 0x80000000,  1, 0 },  // 1
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 3 /////
  { 0x00000000,  1, 1 },  // 0
  { 0x80000000,  1, 0 },  // 1
  { 0xFFFFFFFF,  0, 0 }  // EOT
} };


int RunBeforeCodes[6][17][3]={ {
///// 1 /////
  { 0x00000000,  1, 1 },  // 0
  { 0x80000000,  1, 0 },  // 1
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 2 /////
  { 0x00000000,  2, 2 },  // 00
  { 0x40000000,  2, 1 },  // 01
  { 0x80000000,  1, 0 },  // 1
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 3 /////
  { 0x00000000,  2, 3 },  // 00
  { 0x40000000,  2, 2 },  // 01
  { 0x80000000,  2, 1 },  // 10
  { 0xC0000000,  2, 0 },  // 11
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 4 /////
  { 0x00000000,  3, 4 },  // 000
  { 0x20000000,  3, 3 },  // 001
  { 0x40000000,  2, 2 },  // 01
  { 0x80000000,  2, 1 },  // 10
  { 0xC0000000,  2, 0 },  // 11
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 5 /////
  { 0x00000000,  3, 5 },  // 000
  { 0x20000000,  3, 4 },  // 001
  { 0x40000000,  3, 3 },  // 010
  { 0x60000000,  3, 2 },  // 011
  { 0x80000000,  2, 1 },  // 10
  { 0xC0000000,  2, 0 },  // 11
  { 0xFFFFFFFF,  0, 0 }  // EOT
},{
///// 6 /////
  { 0x00000000,  3, 1 },  // 000
  { 0x20000000,  3, 2 },  // 001
  { 0x40000000,  3, 4 },  // 010
  { 0x60000000,  3, 3 },  // 011
  { 0x80000000,  3, 6 },  // 100
  { 0xA0000000,  3, 5 },  // 101
  { 0xC0000000,  2, 0 },  // 11
  { 0xFFFFFFFF,  0, 0 }  // EOT
} };

//CAVLC table definition, generation, management and initialization
//These tables are used in residual decoding

int cavlc_table_decode(struct cavlc_table *table)
{
  unsigned int code=peekRawBits(24)<<8;
  int min=0, max=table->count;
  while(max-min>1) {
    int mid=(min+max)>>1;
    if(code>=table->items[mid].code) min=mid; else max=mid;
  }
  skipRawBits(table->items[min].bits);
  return table->items[min].data;
}

struct cavlc_table *init_cavlc_table(int *items)
{
  struct cavlc_table *res=new cavlc_table;
  struct cavlc_table_item *tableContent;
  int *pos;
  int count=0;
  for(pos=items; (*pos)!=0xFFFFFFFF; pos+=3) ++count;

  pos=items;

  tableContent = new cavlc_table_item[count];
  
  for (int i=0;i<count;i++)
  {
	  tableContent[i].code=*(pos);
	  tableContent[i].bits=*(pos+1);
	  tableContent[i].data=*(pos+2);
	  pos+=3;
  }

  res->items=tableContent;
  res->count=count;
  return res;
}

void init_cavlc_tables()
{

	ChromaDCLevel_active=false;
 
  for(int i=0; i<4; ++i)
  {
    CoeffTokenCodeTable[i]=init_cavlc_table(&CoeffTokenCodes[i][0][0]);
  }
  
  CoeffTokenCodeTable_ChromaDC=init_cavlc_table(&CoeffTokenCodes_ChromaDC[0][0]);
  
  for(int i=0; i<15; ++i)
  {
    TotalZerosCodeTable_4x4[i]=init_cavlc_table(&TotalZerosCodes_4x4[i][0][0]);
  }

  for(int i=0; i<3; ++i)
  {
    TotalZerosCodeTable_ChromaDC[i]=init_cavlc_table(&TotalZerosCodes_ChromaDC[i][0][0]);
  }

  for(int i=0; i<6; ++i)
  {
    RunBeforeCodeTable[i]=init_cavlc_table(&RunBeforeCodes[i][0][0]);
  }
}

//Calculate the nC parameter as required when decoding residual data.
//TODO: Cb/Cr channel ambiguity with "luma_or_select_chroma"

int get_nC(int x, int y, int luma_or_select_chroma)
{
	int nA, nB;

	if (luma_or_select_chroma==0)
	{
		nA=((x-4)<0 || y<0)?-1:TotalCoeff_luma_array[x-4][y];
		nB=((x)<0 || (y-4)<0)?-1:TotalCoeff_luma_array[x][y-4];
	}
	else
	{
		nA=(((x-4)<0 || y<0))?-1:TotalCoeff_chroma_array[luma_or_select_chroma][x-4][y];
		nB=(((x)<0 || (y-4)<0))?-1:TotalCoeff_chroma_array[luma_or_select_chroma][x][y-4];
	}

	if(nA<0 && nB<0)
	{
		return 0;
	}
	
	if(nA>=0 && nB>=0)
	{
		return (nA+nB+1)/2;
	}
  
	if(nA>=0)
	{
		return nA;
	}
    else
	{
		return nB;
	}
}

//Residual decoding by the norm

void residual(int startIdx, int endIdx)
{
	residual_luma(i16x16DClevel, i16x16AClevel, level, startIdx, endIdx);

	for (int i=0;i<16;i++)
	{
		Intra16x16DCLevel[i]=i16x16DClevel[i];
		for (int j=0;j<16;j++)
		{
			LumaLevel[i][j] = level[i][j];
			Intra16x16ACLevel[i][j]=i16x16AClevel[i][j];
		}
	}



	//if( ChromaArrayType = = 1 | | ChromaArrayType = = 2 )
	//The only supported ChromaArrayType is 1

	NumC8x8 = 4 / (SubWidthC * SubHeightC );
	for (int iCbCr=0; iCbCr<2; iCbCr++)
	{
		//Chroma DC residual present
		if ((CodedBlockPatternChroma & 3) && startIdx==0)
		{
			ChromaDCLevel_active=true;
			residual_block_cavlc(ChromaDCLevel[iCbCr],0,4*NumC8x8-1, 4*NumC8x8, CHROMA);
			ChromaDCLevel_active=false;
		}
		else
		{
			for (int i=0;i<4*NumC8x8;i++)
			{
				ChromaDCLevel[iCbCr][i]=0;
			}
		}
	}

	for (int iCbCr=0;iCbCr<2;iCbCr++)
	{
		for (int i8x8=0;i8x8<NumC8x8;i8x8++)
		{
			for (int i4x4=0; i4x4<4; i4x4++)
			{
				//Chroma AC residual present
				if ((CodedBlockPatternChroma & 2) && endIdx>0)
				{
					residual_block_cavlc(ChromaACLevel[iCbCr][i8x8*4+i4x4],((startIdx-1)>0?(startIdx-1):0),endIdx-1,15,CHROMA);
				}
				else
				{
					for (int i=0;i<15;i++)
					{
						ChromaACLevel[iCbCr][i8x8*4+i4x4][i]=0;
					}
				}
			}
		}
	}
}

void residual_luma(int i16x16DClevel[16], int i16x16AClevel[16][16], int level[16][16], int startIdx, int endIdx)
{
	if (startIdx == 0 && MbPartPredMode(mb_type, 0) == Intra_16x16)
	{
		residual_block_cavlc(i16x16DClevel, 0, 15, 16, LUMA);
	}

	for (int i8x8=0;i8x8<4;i8x8++)
	{
		for (int i4x4=0;i4x4<4;i4x4++)
		{
			if (CodedBlockPatternLuma & (1<<i8x8))
			{
				if (endIdx>0 && MbPartPredMode(mb_type, 0) == Intra_16x16)
				{
					residual_block_cavlc(i16x16AClevel[i8x8*4+i4x4],(((startIdx-1)>0)?(startIdx-1):0),endIdx-1,15,LUMA);
				}
				else
				{
					residual_block_cavlc(level[i8x8*4 + i4x4],startIdx,endIdx,16,LUMA);
				}
			}
			else if (MbPartPredMode(mb_type, 0)==Intra_16x16)
			{
				for (int i=0;i<15;i++)
				{
					i16x16AClevel[i8x8*4 + i4x4][i] = 0;
				}
			}
			else
			{
				for (int i=0;i<16;i++)
				{
					level[i8x8*4 + i4x4][i] = 0;
				}
			}
		}
	}
}

void residual_block_cavlc(int coeffLevel[16], int startIdx, int endIdx, int maxNumCoeff, int luma_or_chroma)
{

	int TotalCoeff, TrailingOnes, level_suffix, levelCode, zerosLeft;
	int coeff_token,suffixLength, trailing_ones_sign_flag, level[16],run_before, run[16], coeffNum, nC;

	for (int i=0;i<maxNumCoeff;i++)
	{
		coeffLevel[i]=0;
	}

	if (ChromaDCLevel_active==true)
	{
		nC=-1;
	}
	else
	{
		nC=get_nC(mb_pos_x,mb_pos_y,luma_or_chroma);
	}

	//nC Value as defined by the h264_vlc.pdf document:
	/*
		If blocks U and L are available (i.e. in the same coded slice), N = (Nu + NL)/2
		If only block U is available, N=NU ; if only block L is available, N=NL ; if neither is available, N=0.
	*/
	//We are using nA and nB (symbols used in norm, page 240) for Nu and NL, and nC for N.
	//The following code decodes the coeff_token symbol using CAVLC tables which were generated at the beginning of the program.

	if (nC==-1)
	{
		coeff_token=cavlc_table_decode(CoeffTokenCodeTable_ChromaDC);
	}
	else if (nC==0 || nC==1)
	{
		coeff_token=cavlc_table_decode(CoeffTokenCodeTable[0]); 
	}
	else if (nC==2 || nC==3)
	{
		coeff_token=cavlc_table_decode(CoeffTokenCodeTable[1]);
	}
	else if (nC>3 && nC<8)
	{
		coeff_token=cavlc_table_decode(CoeffTokenCodeTable[2]);
	}
	else
	{
		coeff_token=cavlc_table_decode(CoeffTokenCodeTable[3]);
	}

	//Extracting TotalCoeff and TrailingOnes from coeff_token symbol.
	//This operation is defined by the norm.
	TotalCoeff=coeff_token/2;
	TrailingOnes=coeff_token&3;

	if (TotalCoeff>0)
	{
		if (TotalCoeff>10 && TrailingOnes<3)
		{
			suffixLength=1;
		}
		else
		{
			suffixLength=0;
		}

		for (int i=0;i<TotalCoeff;i++)
		{
			if (i<TrailingOnes)
			{
				trailing_ones_sign_flag=getRawBits(1);
				level[i]=1-2*trailing_ones_sign_flag;
			}
			else
			{
				int level_prefix;
				for(level_prefix=0; getRawBits(1)==0; ++level_prefix);
				
				int levelSuffixSize;

				if (level_prefix==14 && suffixLength==0)
				{
					levelSuffixSize=4;
				}
				else if (level_prefix>=15)
				{
					levelSuffixSize=(level_prefix-3);
				}
				else
				{
					levelSuffixSize=suffixLength;
				}

				//3. Decoding level_suffix

				if (levelSuffixSize>0)
				{
					level_suffix=getRawBits(levelSuffixSize);
				}
				else
				{
					level_suffix=0;
				}

				//4. Calculating levelCode
				
				levelCode=(((level_prefix<15)?level_prefix:15)<<suffixLength)+level_suffix;

				//5. , 6. , 7. levelCode corrections

				if (level_prefix>=15 && suffixLength==0)
				{
					levelCode=levelCode+15;
				}

				if (level_prefix>=16)
				{
					levelCode=levelCode+(1<<(level_prefix-3))-4096;
				}

				if (i==TrailingOnes && TrailingOnes<3)
				{
					levelCode=levelCode+2;
				}

				//8. Calculating level[i]

				if ((levelCode%2)==0)
				{
					level[i]=(levelCode+2)>>1;
				}
				else
				{
					level[i]=(-levelCode-1)>>1;
				}

				//9. , 10. suffixLength corrections

				if (suffixLength==0)
				{
					suffixLength=1;
				}

				if (ABS(level[i])>(3<<(suffixLength-1)) && suffixLength<6)
				{
					suffixLength=suffixLength+1;
				}
			}
		}

		if(TotalCoeff < (endIdx - startIdx + 1))
		{
			int total_zeros;
			
			if (ChromaDCLevel_active==false)
			{
				total_zeros=cavlc_table_decode(TotalZerosCodeTable_4x4[TotalCoeff-1]);
			}
			else
			{
				total_zeros=cavlc_table_decode(TotalZerosCodeTable_ChromaDC[TotalCoeff-1]);
			}
			zerosLeft=total_zeros;
		}
		else
		{
			zerosLeft=0;
		}


	int run_before_index;	
	for(int j=0; j<TotalCoeff-1; j++)
	{
		if (zerosLeft>5)
		{
			run_before_index=5;
		}
		else
		{
			run_before_index=zerosLeft-1;
		}

	  if (zerosLeft>0)
	  {
		  run_before=cavlc_table_decode(RunBeforeCodeTable[run_before_index]);
		  run[j]=run_before;
	  }
	  else
	  {
		  run[j]=0;
	  }

	  zerosLeft=zerosLeft-run[j];
	}

  run[TotalCoeff-1]=zerosLeft;

  coeffNum=-1;
  for(int i=TotalCoeff-1; i>=0; i--)
  {
    coeffNum+=run[i]+1;
    coeffLevel[startIdx + coeffNum]=level[i];
  }
}
}


//Obsolete residual decoding!

//Actual residual decoding takes place here.
//This part of the code strays heavily from the norm, TODO: explain the helper functions and macros
/*

int residual_block(int *coeffLevel, int maxNumCoeff, int nC)
{
	int coeff_token, TotalCoeff, TrailingOnes;
	int i, suffixLength, zerosLeft, coeffNum;
	int level[16],run[16];

	//nC Value as defined by the h264_vlc.pdf document:
	//
	//	If blocks U and L are available (i.e. in the same coded slice), N = (Nu + NL)/2
	//	If only block U is available, N=NU ; if only block L is available, N=NL ; if neither is available, N=0.
	//
	//We are using nA and nB (symbols used in norm, page 240) for Nu and NL, and nC for N.
	//The following code decodes the coeff_token symbol using CAVLC tables which were generated at the beginning of the program.

	if (nC==-1)
	{
		coeff_token=cavlc_table_decode(CoeffTokenCodeTable_ChromaDC);
	}
	else if (nC==0 || nC==1)
	{
		coeff_token=cavlc_table_decode(CoeffTokenCodeTable[0]); 
	}
	else if (nC==2 || nC==3)
	{
		coeff_token=cavlc_table_decode(CoeffTokenCodeTable[1]);
	}
	else if (nC>3 && nC<8)
	{
		coeff_token=cavlc_table_decode(CoeffTokenCodeTable[2]);
	}
	else
	{
		coeff_token=cavlc_table_decode(CoeffTokenCodeTable[3]);
	}

	//Extracting TotalCoeff and TrailingOnes from coeff_token symbol.
	//This operation is defined by the norm.
	TotalCoeff=coeff_token/2;
	TrailingOnes=coeff_token&3;

  if(TotalCoeff>10 && TrailingOnes<3) suffixLength=1; else suffixLength=0;

  if(!TotalCoeff) return 0;

  for(i=0; i<TotalCoeff; ++i)
    if(i<TrailingOnes)
      level[i]=1-2*getRawBits(1);
    else {
      int level_prefix;
      int levelSuffixSize=suffixLength;
      int levelCode;
      for(level_prefix=0; !getRawBits(1); ++level_prefix);
      levelCode=level_prefix<<suffixLength;
      if(level_prefix==14 && suffixLength==0) levelSuffixSize=4;
      else if(level_prefix==15) levelSuffixSize=12;
      if(levelSuffixSize)
        levelCode+=getRawBits(levelSuffixSize);
      if(level_prefix==15 && suffixLength==0)
        levelCode+=15;
      if(i==TrailingOnes && TrailingOnes<3)
        levelCode+=2;
      if(levelCode&1) level[i]=(-levelCode-1)>>1;
                 else level[i]=( levelCode+2)>>1;
      if(suffixLength==0) suffixLength=1;
      if(ABS(level[i])>(3<<(suffixLength-1)) && suffixLength<6)
        ++suffixLength;
    }

  if(TotalCoeff<maxNumCoeff) {
    if(nC<0) zerosLeft=cavlc_table_decode(TotalZerosCodeTable_ChromaDC[TotalCoeff-1]);
        else zerosLeft=cavlc_table_decode(TotalZerosCodeTable_4x4[TotalCoeff-1]);
  } else
    zerosLeft=0;

  for(i=0; i<TotalCoeff-1; ++i) {
    if(zerosLeft>6) {
      int run_before=7-getRawBits(3);
      if(run_before==7)
        while(!getRawBits(1)) ++run_before;
      run[i]=run_before;
    } else if(zerosLeft>0)
      run[i]=cavlc_table_decode(RunBeforeCodeTable[zerosLeft-1]);
    else
      run[i]=0;
    zerosLeft-=run[i];
  }
  run[TotalCoeff-1]=zerosLeft;

  coeffNum=-1;
  for(i=TotalCoeff-1; i>=0; --i) {
    coeffNum+=run[i]+1;
    coeffLevel[coeffNum]=level[i];
  }

  return TotalCoeff;
}
*/