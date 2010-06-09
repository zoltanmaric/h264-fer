#pragma once
#include <CL/cl.h>

extern cl_mem frame_mem;
extern cl_mem dpb_mem;
extern cl_mem ans_mem;

extern cl_command_queue cmd_queue;
extern cl_context context;
extern cl_kernel kernel[2];

extern int *predModes16x16, *predModes4x4;

void InitCL();
void CloseCL();
void AllocateFrameBuffersCL();

void IntraCL();
void WaitIntraCL(int subMbSize);
void subtractFramesCL(unsigned char *dpb, unsigned char *result);