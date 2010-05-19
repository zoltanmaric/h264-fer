#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <CL/cl.h>

#include "h264_globals.h"

enum Kernel
{
	absDiff = 0,
	getPred
};

cl_program program[1];
cl_kernel kernel[2];

cl_command_queue cmd_queue;
cl_context context;

cl_device_id cpu = NULL, device = NULL;

cl_platform_id platform[1];

char * load_program_source(const char *filename)
{ 
	
	struct stat statbuf;
	FILE *fh; 
	char *source; 
	
	fh = fopen(filename, "r");
	if (fh == 0)
		return 0; 
	
	stat(filename, &statbuf);
	source = (char *) malloc(statbuf.st_size + 1);
	fread(source, statbuf.st_size, 1, fh);
	source[statbuf.st_size] = '\0'; 
	
	return source; 
} 

void InitCL()
{
	cl_int err = 0;
	size_t returned_size = 0;

	// DEVICE INFORMATION
	// Get the platform ids (clGetDeviceIDs does not accept NULL in this implementation)
	err = clGetPlatformIDs(1, platform, NULL);
	assert(err == CL_SUCCESS);

	// Find the CPU CL device, as a fallback
	err = clGetDeviceIDs(platform[0], CL_DEVICE_TYPE_CPU, 1, &cpu, NULL);
	assert(err == CL_SUCCESS);
	
	// Find the GPU CL device, this is what we really want
	// If there is no GPU device is CL capable, fall back to CPU
	err = clGetDeviceIDs(platform[0], CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	if (err != CL_SUCCESS) device = cpu;
	assert(device);

	// Get some information about the returned device
	cl_char vendor_name[1024] = {0};
	cl_char device_name[1024] = {0};
	err = clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(vendor_name), 
						  vendor_name, &returned_size);
	err |= clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), 
						  device_name, &returned_size);
	assert(err == CL_SUCCESS);
	printf("Connecting to %s %s...\n", vendor_name, device_name);
	

	// CONTEXT AND COMMAND QUEUE
	// Now create a context to perform our calculation with the 
	// specified device 
	context = clCreateContext(0, 1, &device, NULL, NULL, &err);
	assert(err == CL_SUCCESS);
	
	// And also a command queue for the context
	cmd_queue = clCreateCommandQueue(context, device, 0, NULL);


	// PROGRAM AND KERNEL CREATION
	// Load the program source from disk
	// The kernel/program is the project directory and in Xcode the executable
	// is set to launch from that directory hence we use a relative path
	char *program_source = load_program_source("h264_kernels.cl");
	program[0] = clCreateProgramWithSource(context, 1, (const char**)&program_source,
										   NULL, &err);
	assert(err == CL_SUCCESS);
	
	char buildLog[5000];
	err = clBuildProgram(program[0], 0, NULL, NULL, NULL, NULL);
	clGetProgramBuildInfo(program[0], device, CL_PROGRAM_BUILD_LOG, 5000, buildLog, NULL);
	printf("%s", buildLog);
	assert(err == CL_SUCCESS);
	
	// Now create the kernel "objects" that we want to use in the example file 
	kernel[absDiff] = clCreateKernel(program[0], "absDiff", &err);
	assert(err == CL_SUCCESS);
	kernel[getPred] = clCreateKernel(program[0], "fetchPredictionSamples16", &err);
	assert(err == CL_SUCCESS);
}

void CloseCL()
{
	clReleaseCommandQueue(cmd_queue);
	clReleaseContext(context);
	delete [] predSamples;
}

void getPredictionSamples()
{
	size_t frameBufferSize = frame.Lwidth*frame.Lheight;
	size_t predSamplesBufferSize = PicWidthInMbs*PicHeightInMbs*33 * sizeof(int);

	cl_mem frame_mem = clCreateBuffer(context, CL_MEM_READ_ONLY, frameBufferSize, NULL, NULL);
	cl_int err = clEnqueueWriteBuffer(cmd_queue, frame_mem, CL_FALSE, 0, frameBufferSize, (void*)frame.L, 0, NULL, NULL);
	assert(err == CL_SUCCESS);

	cl_mem predSamples_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, predSamplesBufferSize, NULL, NULL);
	
	// KERNEL ARGUMENTS
	err = clSetKernelArg(kernel[getPred], 0, sizeof(cl_mem), &frame_mem);
	err = clSetKernelArg(kernel[getPred], 1, sizeof(int), &frame.Lwidth);
	err |= clSetKernelArg(kernel[getPred], 2, sizeof(cl_mem), &predSamples_mem);
	assert(err == CL_SUCCESS);

	clFinish(cmd_queue);

	size_t global_work_size = PicWidthInMbs*PicHeightInMbs*33;	// one for each prediction sample
	err = clEnqueueNDRangeKernel(cmd_queue, kernel[getPred], 1, NULL, &global_work_size, NULL, 0, NULL, NULL);
	assert(err == CL_SUCCESS);

	clFinish(cmd_queue);

	// TEST:
	for (int i = 0; i < predSamplesBufferSize/4; i++)
	{
		predSamples[i] = i;
	}

	err = clEnqueueReadBuffer(cmd_queue, predSamples_mem, CL_TRUE, 0, predSamplesBufferSize, predSamples, 0, NULL, NULL);
	assert(err == CL_SUCCESS);

	clReleaseMemObject(frame_mem);
	clReleaseMemObject(predSamples_mem);
}
