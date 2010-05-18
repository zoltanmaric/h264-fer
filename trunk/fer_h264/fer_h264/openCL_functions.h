#pragma once
#include <CL/cl.h>

extern cl_command_queue cmd_queue;
extern cl_context context;
extern cl_kernel kernel[2];

void InitCL();
void CloseCL();
int RunCL(unsigned char **a, unsigned char **b, int *results);