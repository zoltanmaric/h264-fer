#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <CL/cl.h>

#include "h264_globals.h"

cl_mem frame_mem;
cl_mem frameInt_mem;
cl_mem dpb_mem;
cl_mem ans_mem;		// the result array

cl_mem refFrameKar_mem;
cl_mem refFrameInterpolatedL_mem;

cl_mem predModes_mem;

enum Kernel
{
	absDiff = 0,
	CharToInt,
	FillRefFrameKar,
	Intra
};

cl_program program[1];
cl_kernel kernel[4];

cl_command_queue cmd_queue;
cl_context context;

cl_device_id cpu = NULL, device = NULL;

cl_platform_id platform[1];

bool OpenCLEnabled = true;

int *predModes;

char * load_program_source(const char *filename)
{ 
	
	struct stat statbuf;
	FILE *fh; 
	char *source; 
	
	fh = fopen(filename, "rb");
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
	if (err != CL_SUCCESS)
	{
		device = cpu;
		OpenCLEnabled = false;
	}
	assert(device);

	// Get some information about the returned device
	cl_char vendor_name[1024] = {0};
	cl_char device_name[1024] = {0};
	err = clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(vendor_name), 
						  vendor_name, &returned_size);
	err |= clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), 
						  device_name, &returned_size);
	assert(err == CL_SUCCESS);
	printf("Found OpenCL compatible device:\n%s %s...\n", vendor_name, device_name);
	

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
	printf("%s\n", buildLog);
	assert(err == CL_SUCCESS);
	
	// Now create the kernel "objects" that we want to use in the example file 
	kernel[absDiff] = clCreateKernel(program[0], "absDiff", &err);
	assert(err == CL_SUCCESS);

	kernel[CharToInt] = clCreateKernel(program[0], "CharToInt", &err);
	assert(err == CL_SUCCESS);

	kernel[FillRefFrameKar] = clCreateKernel(program[0], "FillRefFrameKar", &err);
	assert(err == CL_SUCCESS);

	kernel[Intra] = clCreateKernel(program[0], "IntraPrediction", &err);
	assert(err == CL_SUCCESS);
}

void AllocateFrameBuffers()
{
	size_t frameBufferSize = frame.Lwidth*frame.Lheight;
	size_t frameIntBufferSize = frame.Lwidth*frame.Lheight * sizeof(int);

	frame_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, frameBufferSize, NULL, NULL);
	frameInt_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, frameIntBufferSize, NULL, NULL);
	dpb_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, frameBufferSize, NULL, NULL);
	ans_mem	= clCreateBuffer(context, CL_MEM_READ_WRITE, frameBufferSize, NULL, NULL);

	size_t predModesBufferSize = (frame.Lwidth >> 4)*(frame.Lheight >> 4) * sizeof(int);
	predModes_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, predModesBufferSize, NULL, NULL);
	predModes = new int[predModesBufferSize / sizeof(int)];
	
	size_t refFrameKarBufferSize = (frame.Lwidth+8)*(frame.Lheight+8)*16*6*sizeof(int);
	size_t refFrameInterpolatedBufferSize = frameBufferSize*16*sizeof(int);
	refFrameKar_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, refFrameKarBufferSize, NULL, NULL);
	refFrameInterpolatedL_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, refFrameInterpolatedBufferSize, NULL, NULL);


}

void CloseCL()
{
	clReleaseMemObject(frame_mem);
	clReleaseMemObject(frameInt_mem);
	clReleaseMemObject(dpb_mem);
	clReleaseMemObject(ans_mem);
	clReleaseMemObject(predModes_mem);
	clReleaseCommandQueue(cmd_queue);
	clReleaseContext(context);
	delete [] predModes;
}

void TestKar(int **refFrameKar[6][16], frame_type refFrameInterpolated[16])
{
	int *tempInterpolated[16];

	for (int k = 0; k < 16; k++)
	{
		tempInterpolated[k] = new int[frame.Lwidth*frame.Lheight];
		for (int i = 0; i < frame.Lheight; i++)
		{
			for (int j = 0; j < frame.Lwidth; j++)
			{
				tempInterpolated[k][i*frame.Lwidth+j] = refFrameInterpolated[k].L[i*frame.Lwidth+j];
			}
		}
	}

	size_t frameBufferSize = frame.Lwidth*frame.Lheight;
	size_t interpolBufferPartSize = frameBufferSize*sizeof(int);
	size_t karBufferPartSize = (frame.Lwidth+8)*sizeof(int);
	
	cl_int err = 0;

	size_t offset = 0;
	for (int i = 0; i < 16; i++)
	{
		err |= clEnqueueWriteBuffer(cmd_queue, refFrameInterpolatedL_mem, CL_FALSE, offset, interpolBufferPartSize,
			(void*)(tempInterpolated[i]), 0, NULL, NULL);
		offset += interpolBufferPartSize;
	}
	assert(err == CL_SUCCESS);

	
	// KERNEL ARGUMENTS
	err = clSetKernelArg(kernel[FillRefFrameKar],  0, sizeof(cl_mem), &refFrameKar_mem);
	err |= clSetKernelArg(kernel[FillRefFrameKar], 1, sizeof(cl_mem), &refFrameInterpolatedL_mem);
	err |= clSetKernelArg(kernel[FillRefFrameKar], 2, sizeof(int), &frame.Lheight);
	err |= clSetKernelArg(kernel[FillRefFrameKar], 3, sizeof(int), &frame.Lwidth);
	assert(err == CL_SUCCESS);

	clFinish(cmd_queue);

	size_t global_work_size = (frame.Lwidth+8)*(frame.Lheight+8);
	err = clEnqueueNDRangeKernel(cmd_queue, kernel[FillRefFrameKar], 1, NULL,
		&global_work_size, NULL, 0, NULL, NULL);
	assert(err == CL_SUCCESS);

	clFinish(cmd_queue);

	offset = 0;
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			for (int y = 0; y < frame.Lheight+8; y++)
			{
				err |= clEnqueueReadBuffer(cmd_queue, refFrameKar_mem, CL_TRUE, offset, karBufferPartSize,
					refFrameKar[i][j][y], 0, NULL, NULL);
				offset += karBufferPartSize;
			}
		}
	}	

	for (int k = 0; k < 16; k++)
	{
		delete [] tempInterpolated[k];
	}

	clFinish(cmd_queue);
	int test = 0;
}


void IntraCL()
{
	size_t frameBufferSize = frame.Lwidth*frame.Lheight;
	size_t frameIntBufferSize = frame.Lwidth*frame.Lheight*sizeof(int);
	
	cl_int err = 0;

	// Copy frame_mem to frameInt_mem:
	err |= clEnqueueWriteBuffer(cmd_queue, frame_mem, CL_FALSE, 0, frameBufferSize,
		(void*)(frame.L), 0, NULL, NULL);
	assert(err == CL_SUCCESS);

	err = clSetKernelArg(kernel[CharToInt], 0, sizeof(cl_mem), &frame_mem);
	err |= clSetKernelArg(kernel[CharToInt], 1, sizeof(cl_mem), &frameInt_mem);
	assert(err == CL_SUCCESS);

	clFinish(cmd_queue);

	size_t global_work_size = frame.Lwidth*frame.Lheight / sizeof(cl_int);
	err = clEnqueueNDRangeKernel(cmd_queue, kernel[CharToInt], 1, NULL,
		&global_work_size, NULL, 0, NULL, NULL);
	assert(err == CL_SUCCESS);

	clFinish(cmd_queue);

	// The frameInt_mem buffer is the input to the intra prediction kernel
	err = clSetKernelArg(kernel[Intra], 0, sizeof(cl_mem), &frameInt_mem);
	err |= clSetKernelArg(kernel[Intra], 1, sizeof(int), &frame.Lwidth);
	err |= clSetKernelArg(kernel[Intra], 2, sizeof(cl_mem), &predModes_mem);

	clFinish(cmd_queue);

	global_work_size = (frame.Lwidth >> 4)*(frame.Lheight >> 4);
	err = clEnqueueNDRangeKernel(cmd_queue, kernel[Intra], 1, NULL,
		&global_work_size, NULL, 0, NULL, NULL);
	assert(err == CL_SUCCESS);

	clFinish(cmd_queue);

	size_t predModeBufferSize = (frame.Lwidth >> 4)*(frame.Lheight >> 4) * sizeof(int);
	err |= clEnqueueReadBuffer(cmd_queue, predModes_mem, CL_TRUE, 0, predModeBufferSize,
							predModes, 0, NULL, NULL);

	clFinish(cmd_queue);
	int test = 0;
}