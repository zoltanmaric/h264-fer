#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <CL/cl.h>

#include "h264_globals.h"

cl_command_queue cmd_queue;
cl_context context;
cl_device_id cpu = NULL, device = NULL;
cl_platform_id platform;

cl_mem frame_mem;
cl_mem dpb_mem;
cl_mem ans_mem;		// the result array

cl_mem predModes16x16_mem;
cl_mem predModes4x4_mem;

cl_program programGeneral, programIntra;
cl_kernel kernelAbsDiff, kernelCharToInt, kernelIntra16, kernelIntra4;

cl_event eventReadPredModes16x16, eventReadPredModes4x4;

bool OpenCLEnabled = true;

int *predModes16x16, *predModes4x4;

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
	err = clGetPlatformIDs(1, &platform, NULL);
	assert(err == CL_SUCCESS);

	// Find the CPU CL device, as a fallback
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &cpu, NULL);
	assert(err == CL_SUCCESS);
	
	// Find the GPU CL device, this is what we really want
	// If there is no GPU device is CL capable, fall back to CPU
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	if (err != CL_SUCCESS)
	{
		device = cpu;
		OpenCLEnabled = false;
	}
	assert(device);

	// TEST:
	device = cpu;
	OpenCLEnabled = false;

	// Do not initalize OpenCL if no compatible GPU is found.
	if (OpenCLEnabled == false) return;

	// Get some information about the returned device
	cl_char vendor_name[1024] = {0};
	cl_char device_name[1024] = {0};
	err = clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(vendor_name), 
						  vendor_name, &returned_size);
	err |= clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), 
						  device_name, &returned_size);
	assert(err == CL_SUCCESS);
	printf("Found OpenCL compatible device:\n%s, %s...\n", vendor_name, device_name);
	

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
	programGeneral = clCreateProgramWithSource(context, 1, (const char**)&program_source,
										   NULL, &err);
	assert(err == CL_SUCCESS);

	program_source = load_program_source("intra_kernels.cl");
	programIntra = clCreateProgramWithSource(context, 1, (const char**)&program_source,
										   NULL, &err);
	assert(err == CL_SUCCESS);
	
	char buildLog[5000];
	printf("Building program h264_kernels.cl ...\n");
	err = clBuildProgram(programGeneral, 0, NULL, NULL, NULL, NULL);
	clGetProgramBuildInfo(programGeneral, device, CL_PROGRAM_BUILD_LOG, 5000, buildLog, NULL);
	printf("%s\n", buildLog);
	assert(err == CL_SUCCESS);

	printf("Building program intra_kernels.cl ...\n");
	err = clBuildProgram(programIntra, 0, NULL, NULL, NULL, NULL);
	clGetProgramBuildInfo(programIntra, device, CL_PROGRAM_BUILD_LOG, 5000, buildLog, NULL);
	printf("%s\n", buildLog);
	assert(err == CL_SUCCESS);
	
	// Now create the kernel "objects" that we want to use in the example file 
	kernelAbsDiff= clCreateKernel(programGeneral, "AbsDiff", &err);
	assert(err == CL_SUCCESS);

	kernelCharToInt = clCreateKernel(programGeneral, "CharToInt", &err);
	assert(err == CL_SUCCESS);

	kernelIntra16 = clCreateKernel(programIntra, "GetIntra16x16PredModes", &err);
	assert(err == CL_SUCCESS);

	kernelIntra4 = clCreateKernel(programIntra, "GetIntra4x4PredModes", &err);
	assert(err == CL_SUCCESS);
}

void AllocateFrameBuffersCL()
{
	if (OpenCLEnabled == false) return;

	size_t frameBufferSize = frame.Lwidth*frame.Lheight;
	size_t frameIntBufferSize = frame.Lwidth*frame.Lheight * sizeof(int);

	frame_mem = clCreateBuffer(context, CL_MEM_READ_ONLY, frameBufferSize, NULL, NULL);
	dpb_mem = clCreateBuffer(context, CL_MEM_READ_ONLY, frameBufferSize, NULL, NULL);
	ans_mem	= clCreateBuffer(context, CL_MEM_WRITE_ONLY, frameBufferSize, NULL, NULL);

	size_t predModes16BufferSize = (frame.Lwidth >> 4)*(frame.Lheight >> 4) * sizeof(int);
	predModes16x16_mem = clCreateBuffer(context, CL_MEM_WRITE_ONLY, predModes16BufferSize, NULL, NULL);
	predModes16x16 = new int[predModes16BufferSize / sizeof(int)];

	size_t predModes4BufferSize = (frame.Lwidth >> 2)*(frame.Lheight >> 2) * sizeof(int);
	predModes4x4_mem = clCreateBuffer(context, CL_MEM_WRITE_ONLY, predModes4BufferSize, NULL, NULL);
	predModes4x4 = new int[predModes4BufferSize / sizeof(int)];
}

void CloseCL()
{
	if (OpenCLEnabled == false) return;

	clReleaseMemObject(frame_mem);
	clReleaseMemObject(dpb_mem);
	clReleaseMemObject(ans_mem);
	clReleaseMemObject(predModes16x16_mem);
	clReleaseMemObject(predModes4x4_mem);

	clReleaseCommandQueue(cmd_queue);
	clReleaseContext(context);

	delete [] predModes16x16;
	delete [] predModes4x4;
}

void subtractFramesCL(unsigned char *dpb, unsigned char *result)
{
	cl_int err = 0;
	size_t returned_size = 0;

	size_t buffer_size = frame.Lwidth*frame.Lheight;
	cl_event event_wait_list[3];
	cl_uint num_events = 0;

	err = clEnqueueWriteBuffer(cmd_queue, frame_mem, CL_FALSE, 0, buffer_size,		// TEST: currently non-blocking, may cause errors
							   (void*)frame.L, 0, NULL, &event_wait_list[num_events++]);
	
	err |= clEnqueueWriteBuffer(cmd_queue, dpb_mem, CL_FALSE, 0, buffer_size,
								(void*)dpb, 0, NULL, &event_wait_list[num_events++]);
	assert(err == CL_SUCCESS);

	
	// KERNEL ARGUMENTS
	// Now setup the arguments to our kernel
	err  = clSetKernelArg(kernelAbsDiff,  0, sizeof(cl_mem), &frame_mem);
	err |= clSetKernelArg(kernelAbsDiff,  1, sizeof(cl_mem), &dpb_mem);
	err |= clSetKernelArg(kernelAbsDiff,  2, sizeof(cl_mem), &ans_mem);
	assert(err == CL_SUCCESS);
	
	// EXECUTION AND READ
	// Run the calculation by enqueuing it and forcing the 
	// command queue to complete the task
	size_t global_work_size = frame.Lwidth*frame.Lheight;
	cl_event evnt;
	err = clEnqueueNDRangeKernel(cmd_queue, kernelAbsDiff, 1, NULL, 
								 &global_work_size, NULL, num_events, event_wait_list, &evnt);
	assert(err == CL_SUCCESS);
	event_wait_list[0] = evnt;
	num_events = 1;
	
	// Once finished read back the result from the answer 
	// array into the result array
	err = clEnqueueReadBuffer(cmd_queue, ans_mem, CL_TRUE, 0, buffer_size, 
							  result, num_events, event_wait_list, NULL);
	assert(err == CL_SUCCESS);
}

void IntraCL()
{
	size_t global_work_size;

	if (OpenCLEnabled == false) return;

	size_t frameBufferSize = frame.Lwidth*frame.Lheight;
	size_t frameIntBufferSize = frame.Lwidth*frame.Lheight*sizeof(int);
	
	cl_int err = 0;

	// Copy frame_mem to frameInt_mem:
	cl_event eventWriteBuffer; 
	err |= clEnqueueWriteBuffer(cmd_queue, frame_mem, CL_FALSE, 0, frameBufferSize,
		(void*)(frame.L), 0, NULL, &eventWriteBuffer);
	assert(err == CL_SUCCESS);

	// The frameInt_mem buffer is the input to the intra prediction kernel
	err = clSetKernelArg(kernelIntra16, 0, sizeof(cl_mem), &frame_mem);
	err |= clSetKernelArg(kernelIntra16, 1, sizeof(int), &frame.Lwidth);
	err |= clSetKernelArg(kernelIntra16, 2, sizeof(int), &QPy);
	err |= clSetKernelArg(kernelIntra16, 3, sizeof(cl_mem), &predModes16x16_mem);
	assert(err == CL_SUCCESS);

	err = clSetKernelArg(kernelIntra4, 0, sizeof(cl_mem), &frame_mem);
	err |= clSetKernelArg(kernelIntra4, 1, sizeof(int), &frame.Lwidth);
	err |= clSetKernelArg(kernelIntra4, 2, sizeof(int), &QPy);
	err |= clSetKernelArg(kernelIntra4, 3, sizeof(cl_mem), &predModes4x4_mem);
	assert(err == CL_SUCCESS);

	cl_event eventIntra4;
	global_work_size = (frame.Lwidth >> 2)*(frame.Lheight >> 2);
	err = clEnqueueNDRangeKernel(cmd_queue, kernelIntra4, 1, NULL,
		&global_work_size, NULL, 1, &eventWriteBuffer, &eventIntra4);
	assert(err == CL_SUCCESS);

	cl_event eventIntra16;
	global_work_size = (frame.Lwidth >> 4)*(frame.Lheight >> 4);
	err = clEnqueueNDRangeKernel(cmd_queue, kernelIntra16, 1, NULL,
		&global_work_size, NULL, 1, &eventWriteBuffer, &eventIntra16);
	assert(err == CL_SUCCESS);

	size_t predMode16BufferSize = (frame.Lwidth >> 4)*(frame.Lheight >> 4) * sizeof(int);
	err |= clEnqueueReadBuffer(cmd_queue, predModes16x16_mem, CL_FALSE, 0, predMode16BufferSize,
							predModes16x16, 1, &eventIntra16, &eventReadPredModes16x16);

	size_t predMode4BufferSize = (frame.Lwidth >> 2)*(frame.Lheight >> 2) * sizeof(int);
	err |= clEnqueueReadBuffer(cmd_queue, predModes4x4_mem, CL_FALSE, 0, predMode4BufferSize,
							Intra4x4PredMode, 1, &eventIntra4, &eventReadPredModes4x4);
	assert(err == CL_SUCCESS);
}

void WaitIntraCL(bool Intra16x16)
{
	if (Intra16x16 == true)
	{
		clWaitForEvents(1, &eventReadPredModes16x16);
	}
	else
	{
		clWaitForEvents(1, &eventReadPredModes4x4);
	}
}