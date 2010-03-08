#pragma once

#define COEFF_TOKEN(TrailingOnes,TotalCoeff) (((TotalCoeff)<<2)|(TrailingOnes))

void derivation_process_for_4x4_chroma_block_indices(int x, int y, int *chroma4x4BlkIdx);

//Coefficient levels

extern int i16x16DClevel[16];
extern int i16x16AClevel[16][16];
extern int Intra16x16DCLevel[16];
extern int Intra16x16ACLevel[16][16];
extern int level[16][16], LumaLevel[16][16];

extern int ChromaDCLevel[2][4];
extern int ChromaACLevel[2][4][16];

extern int NumC8x8;

extern int coeffLevel_luma_DC[16];
extern int coeffLevel_luma_AC[16][16];
extern int coeffLevel_chroma_DC[2][4];
extern int coeffLevel_chroma_AC[2][4][16];

//TODO: Niz tipa mb_width*mb_height*16 integera NETOCNO, izrazeni u transform/colour blokovima
//Number of non-zero coefficients in luma and chroma blocks (integer from 0 to 16)
extern int **TotalCoeff_luma_array;
extern int ***TotalCoeff_chroma_array;

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

extern int CoeffTokenCodes[4][64][3];
extern int CoeffTokenCodes_ChromaDC[15][3];
extern int TotalZerosCodes_4x4[15][18][3];
extern int TotalZerosCodes_ChromaDC[3][5][3];
extern int RunBeforeCodes[6][17][3];


int get_nC(int x, int y, int luma_or_select_chroma);

void init_cavlc_tables();
struct cavlc_table *init_cavlc_table(struct cavlc_table_item *items);
int cavlc_table_decode(struct cavlc_table *table);

//TEST INSERT
int residual_block(int *coeffLevel, int maxNumCoeff, int nC);


void residual(int startIdx, int endIdx);
void residual_luma(int i16x16DClevel[16], int i16x16AClevel[16][16], int level[16][16], int startIdx, int endIdx);
void residual_block_cavlc(int ChromaDCLevel[16], int, int, int);