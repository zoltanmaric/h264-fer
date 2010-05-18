#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <CL/cl.h>

#include "h264_globals.h"

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
	kernel[0] = clCreateKernel(program[0], "absDiff", &err);
}

void CloseCL()
{
	clReleaseCommandQueue(cmd_queue);
	clReleaseContext(context);
}

int RunCL(unsigned char **a, unsigned char **b, int *results)
{
	cl_int err = 0;
	size_t returned_size = 0;
	
	cl_mem a_mem, b_mem, ans_mem;

	int *a_buff, *b_buff;

	a_buff = new int[frame.Lwidth*frame.Lheight];
	b_buff = new int[frame.Lwidth*frame.Lheight];

	for (int i = 0; i < frame.Lheight; i++)
	{
		for (int j = 0; j < frame.Lwidth; j++)
		{
			a_buff[i*frame.Lwidth+j] = a[i][j];
			b_buff[i*frame.Lwidth+j] = b[i][j];
		}
	}
		

	// MEMORY ALLOCATION
	// Allocate memory on the device to hold our data and store the results into
	size_t buffer_size = frame.Lwidth*frame.Lheight * sizeof(int);
	cl_event event_wait_list[3];
	cl_uint num_events = 0;

	// Input array a
	a_mem = clCreateBuffer(context, CL_MEM_READ_ONLY, buffer_size, NULL, NULL);
	err = clEnqueueWriteBuffer(cmd_queue, a_mem, CL_FALSE, 0, buffer_size,		// TEST: currently non-blocking, may cause errors
							   (void*)a_buff, 0, NULL, &event_wait_list[num_events++]);
	
	// Input array b
	b_mem = clCreateBuffer(context, CL_MEM_READ_ONLY, buffer_size, NULL, NULL);
	err |= clEnqueueWriteBuffer(cmd_queue, b_mem, CL_FALSE, 0, buffer_size,
								(void*)b_buff, 0, NULL, &event_wait_list[num_events++]);
	assert(err == CL_SUCCESS);
	
	// Results array
	ans_mem	= clCreateBuffer(context, CL_MEM_READ_WRITE, buffer_size, NULL, NULL);
	
	// Get all of the stuff written and allocated 
	//clFinish(cmd_queue);
	
	
	// KERNEL ARGUMENTS
	// Now setup the arguments to our kernel
	err  = clSetKernelArg(kernel[0],  0, sizeof(cl_mem), &a_mem);
	err |= clSetKernelArg(kernel[0],  1, sizeof(cl_mem), &b_mem);
	err |= clSetKernelArg(kernel[0],  2, sizeof(cl_mem), &ans_mem);
	assert(err == CL_SUCCESS);
	
	// EXECUTION AND READ
	// Run the calculation by enqueuing it and forcing the 
	// command queue to complete the task
	size_t global_work_size = frame.Lwidth*frame.Lheight;
	cl_event evnt;
	err = clEnqueueNDRangeKernel(cmd_queue, kernel[0], 1, NULL, 
								 &global_work_size, NULL, num_events, event_wait_list, &evnt);
	assert(err == CL_SUCCESS);
	event_wait_list[0] = evnt;
	num_events = 1;
	
	// Once finished read back the results from the answer 
	// array into the results array
	err = clEnqueueReadBuffer(cmd_queue, ans_mem, CL_TRUE, 0, buffer_size, 
							  results, num_events, event_wait_list, NULL);
	assert(err == CL_SUCCESS);
	//clFinish(cmd_queue);
	
	// TEARDOWN
	clReleaseMemObject(a_mem);
	clReleaseMemObject(b_mem);
	clReleaseMemObject(ans_mem);
	
	return CL_SUCCESS;
}