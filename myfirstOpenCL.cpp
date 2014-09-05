/**********************************************************************
Copyright ©2013 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

•	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
•	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/

// For clarity,error checking has been omitted.

//includes and defines
#include <CL/cl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

//opencv
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#define SUCCESS 0
#define FAILURE 1

void CheckError (cl_int error)
{
	if (error != SUCCESS) {
		std::cerr << "OpenCL call failed with error " << error << std::endl;
		std::exit (1);
	}
}


//reads a whole file (name) into memory and places it into the string "result" \
//generally inefficient method when opening large files
std::string LoadKernel (const char* name)
{
	std::ifstream in (name);
	std::string result ((std::istreambuf_iterator<char> (in)), std::istreambuf_iterator<char> ());
	return result;
}

int main()
{
	//error variable
	cl_int status;

	//opencl platforms available?
	cl_uint platformIdCount = 0;
	clGetPlatformIDs (0, NULL, &platformIdCount);

	if (platformIdCount == 0) {
			std::cerr << "No OpenCL platform found" << std::endl;
			return 1;
	} else {
			std::cout << "Found " << platformIdCount << " platform(s)" << std::endl;
	}

	//get platform ids
	cl_platform_id* platforms = (cl_platform_id* )malloc(platformIdCount* sizeof(cl_platform_id));
	clGetPlatformIDs (platformIdCount, platforms, NULL);

	char name[10240];
	for (cl_uint i = 0; i < platformIdCount; ++i) {
		clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 10240, name, NULL);
		std::cout << "Platform " << i << " ID is: " << name << std::endl;
	}

	//choose platform
	cl_platform_id platform = platforms[1];
	std::cout << std::endl << "I choose platform 1" << std::endl;
	free(platforms);

	//search for GPU in chosen platform
	cl_uint				numDevices = 0;
	cl_device_id        *devices;
	clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices); //briskw poses devices (numdevices) yparxoyn sthn 
	                                                                             //platforma poy symfwnoyn me to cl_device_type
	if (numDevices == 0)	//no GPU available. 
	{
		std::cout << "No GPU device available." << std::endl;
		std::cout << "Choose CPU as default device." << std::endl;
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL, &numDevices);//poses CPU diathesimes sthn platforma
		CheckError(status);
		devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id)); //kanw xwro gia ta id toys
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, numDevices, devices, NULL); //briskw ta id toys
	}
	else //GPU is available
	{
		std::cout << numDevices << " GPU device(s) available in this platform." << std::endl << std::endl;
		devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id)); //kanw xwro gia ta id twn GPU poy brhka parapanw
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devices, NULL); //briskw ta id toys
		CheckError(status);
	}

	//create context (first initialize context properties)
	const cl_context_properties contextProperties [] ={CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties> (platform), 0, 0};
	cl_context context = clCreateContext(contextProperties, numDevices, devices, NULL, NULL, &status);
	CheckError(status);

	//create command queue
	cl_command_queue commandQueue = clCreateCommandQueue(context, devices[0], 0, &status);
	CheckError(status);

	//DO YOUR MAGIC =)

	//create program: takes .cl file, reads it into a string, and makes a program with it using the previously created context
	std::string kernelString = LoadKernel ("HelloWorld_Kernel.cl");
	const char* sources [1] = { kernelString.data () };//allocate array of constant size 1 - giati h createprogrwithsource zhtaei array
	size_t lengths [1] = { kernelString.size () };
	cl_program program = clCreateProgramWithSource (context, 1, sources, lengths, &status);
	CheckError (status);

	//build (compile and link) program
	CheckError(clBuildProgram (program, numDevices, devices, "-D FILTER_SIZE=1", NULL, NULL));

	/*create kernel (o kernel me onoma "Filter" brisketai sto arxeio poy molis kaname build)
	cl_kernel kernel = clCreateKernel (program, "Filter", &status);
	CheckError (status);*/
	cl_kernel kernel = clCreateKernel (program, "CopyImage", &status);
	CheckError (status);

	//open image using openCV and load into Mat
	cv::Mat ImgIn = cv::imread("img.png",-1);
	cv::Mat ImgOut = cv::imread("img.png",-1);

	//apo to mat sto opoio exoyme ta data ths eikonas mas ftiaxnoume mia 2d image poy mporei na perasei ston kernel
	static const cl_image_format format = { CL_RGBA, CL_UNORM_INT8 };
	cl_mem inputImage = clCreateImage2D (context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &format,
		ImgIn.rows, ImgIn.cols, 0, ImgIn.data, &status);
	CheckError (status);
	//kai kanoume xwro antistoixa gia to apotelesma
	cl_mem outputImage = clCreateImage2D (context, CL_MEM_WRITE_ONLY, &format, ImgIn.rows, ImgIn.cols, 0, NULL, &status);
	CheckError (status);

	/* Simple Gaussian blur filter
	float filter [] = {
		1, 2, 1,
		2, 4, 2,
		1, 2, 1
	};*/

	/* Normalize the filter
	for (int i = 0; i < 9; ++i) {
		filter [i] /= 16.0f;
	}*/

	/* Create a buffer for the filter weights
	cl_mem filterWeightsBuffer = clCreateBuffer (context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof (float) * 9, filter, &status);
	CheckError (status);*/

	/*set kernel args - defined above
	clSetKernelArg (kernel, 0, sizeof (cl_mem), &inputImage);
	clSetKernelArg (kernel, 1, sizeof (cl_mem), &filterWeightsBuffer);
	clSetKernelArg (kernel, 2, sizeof (cl_mem), &outputImage);*/
	clSetKernelArg (kernel, 0, sizeof (cl_mem), &inputImage);
	clSetKernelArg (kernel, 1, sizeof (cl_mem), &outputImage);

	//execute kernel EPI TELOUS
	cl_uint work_dim = 2; //work_dim -> o arithmos diastasewn stis opoies tha doylepsoyme
	std::size_t size [3] = { ImgIn.rows, ImgIn.cols, 1 }; //size -> o arithmos twn twork items pou tha xrhsimopoihsoyme
	CheckError (clEnqueueNDRangeKernel (commandQueue, kernel, work_dim, NULL, size, NULL, 0, NULL, NULL));
	
	std::size_t origin [3] = { 0 };
	clEnqueueReadImage (commandQueue, outputImage, CL_TRUE, origin, size, 0, 0, ImgOut.data, 0, nullptr, nullptr);

	 cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );// Create a window for display.
     cv::imshow( "Display window", ImgOut );        
	 cv::waitKey(0);            

	 cv::imwrite( "blured.png", ImgOut);

	//cleanup
	status = clReleaseCommandQueue(commandQueue);
	status = clReleaseContext(context);
	status = clReleaseProgram (program);
	//status = clReleaseMemObject(filterWeightsBuffer);
	status = clReleaseMemObject(inputImage);
	status = clReleaseMemObject(outputImage);
	CheckError (status);

}