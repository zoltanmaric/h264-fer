#pragma once
#include <CL/cl.h>

extern enum Kernel;

extern cl_mem frame_mem;
extern cl_mem dpb_mem;
extern cl_mem ans_mem;

extern cl_command_queue cmd_queue;
extern cl_context context;
extern cl_kernel kernel[2];

void InitCL();
void CloseCL();
void AllocateFrameBuffers();
void getPredictionSamples();
void TestKar(int **refFrameKar[6][16], frame_type refFrameInterpolatedL[16]);