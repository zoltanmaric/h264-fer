#pragma once

extern unsigned int expgolomb_UC_codes[10000][2];
void init_expgolomb_UC_codes();
unsigned int SC_to_UC(int codeNum);

unsigned int expGolomb_UD();

signed int expGolomb_SD();

void expGolomb_UC(unsigned int codeNum);
void expGolomb_SC(int codeNum);

void golombRice_SC(int codeNum, unsigned int VLCNum);

unsigned int expGolomb_TD();