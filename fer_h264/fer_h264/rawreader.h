#pragma once

void initRawReader(unsigned char *RBSP,unsigned int size);

bool more_rbsp_data();

void skipRawBits(int N);

unsigned int peekRawBits(int N);

unsigned int getRawBits(int N);

unsigned int RBSPtoUINT(unsigned char *rbsp, int N);

extern unsigned int RBSP_current_byte;
extern unsigned int RBSP_current_bit;
extern unsigned int RBSP_total_size;
extern unsigned char *RBSP_data;