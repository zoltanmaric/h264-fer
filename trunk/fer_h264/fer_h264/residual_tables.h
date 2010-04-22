#pragma once

#define COEFF_TOKEN(TrailingOnes,TotalCoeff) (((TotalCoeff)<<2)|(TrailingOnes))
#define MAX_LEVELCODE_VALUE 5055+1
#define MAX_SUFFIX_VALUE 4095+1


//CAVLC tables used in residual decoding.

struct cavlc_table_item
{
  unsigned int code;
  int bits;
  int data;
} ;

struct cavlc_table
{
  int count;
  struct cavlc_table_item *items;
} ;


extern struct cavlc_table *CoeffTokenCodeTable[4];
extern struct cavlc_table *CoeffTokenCodeTable_ChromaDC;
extern struct cavlc_table *TotalZerosCodeTable_4x4[15];
extern struct cavlc_table *TotalZerosCodeTable_ChromaDC[3];
extern struct cavlc_table *RunBeforeCodeTable[6];

extern int RunBeforeCodeTableCoder_length[6][7];
extern unsigned char RunBeforeCodeTableCoder_data[6][7][4];
extern int TotalZerosCodeTableCoder_ChromaDC_length[3][4];
extern unsigned char TotalZerosCodeTableCoder_ChromaDC_data[3][4][4];
extern int TotalZerosCodeTableCoder_4x4_length[15][16];
extern unsigned char TotalZerosCodeTableCoder_4x4_data[15][16][4];
extern int CoeffTokenCodeTableCoder_ChromaDC_length[17][4];
extern unsigned char CoeffTokenCodeTableCoder_ChromaDC_data[17][4][4];
extern int CoeffTokenCodesCoder_nC_8_to_max_length[17][4];
extern unsigned char CoeffTokenCodesCoder_nC_8_to_max_data[17][4][4];
extern int CoeffTokenCodesCoder_nC_4_to_8_length[17][4];
extern unsigned char CoeffTokenCodesCoder_nC_4_to_8_data[17][4][4];
extern int CoeffTokenCodesCoder_nC_2_to_4_length[17][4];
extern unsigned char CoeffTokenCodesCoder_nC_2_to_4_data[17][4][4];
extern int CoeffTokenCodesCoder_nC_0_to_2_length[17][4];
extern unsigned char CoeffTokenCodesCoder_nC_0_to_2_data[17][4][4];
extern int CoeffTokenCodes[4][64][3];
extern int CoeffTokenCodes_ChromaDC[15][3];
extern int TotalZerosCodes_4x4[15][18][3];
extern int TotalZerosCodes_ChromaDC[3][5][3];
extern int RunBeforeCodes[6][17][3];


void generate_residual_level_tables();

extern int levelcode_to_outputstream[MAX_LEVELCODE_VALUE][7][4];
extern int inputstream_to_levelcode[16][7][MAX_SUFFIX_VALUE];

void init_cavlc_tables();
struct cavlc_table *init_cavlc_table(struct cavlc_table_item *items);
int cavlc_table_decode(struct cavlc_table *table);