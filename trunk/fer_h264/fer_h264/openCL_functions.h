#pragma once
#include <CL/cl.h>

extern enum Kernels;

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
void getPredictionSamples();
void TestKar(int **refFrameKar[6][16], frame_type refFrameInterpolatedL[16]);
void IntraCL();