#pragma once

unsigned int expGolomb_UD();

signed int expGolomb_SD();

void expGolomb_UC(unsigned int codeNum);
void expGolomb_SC(unsigned int codeNum);

void golombRice_SC(int codeNum, unsigned int VLCNum);

unsigned int expGolomb_TD();