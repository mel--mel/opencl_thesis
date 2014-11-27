/**********************************************************************
Copyright ©2013 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

•   Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
•   Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/


#include "SimpleImage.hpp"
#include "myImage.hpp"
#include "HelpingFunctions.h"
#include "Templates.cpp"
#include <cmath>
#include <vector>

#define OUTPUT_IMAGE "L_Out.bmp"

void giveMelOpenCL::setupCL(std::string kernelsFileName)
{
    cl_int status = CL_SUCCESS;
    cl_device_type dType;

	//find device type
    if(sampleArgs->deviceType.compare("cpu") == 0) {
		dType = CL_DEVICE_TYPE_CPU;
		std::cout << "Device type = CPU" << std::endl;} //deviceType = "cpu"
    else{  
        dType = CL_DEVICE_TYPE_GPU; //deviceType = "gpu"
        if(sampleArgs->isThereGPU() == false) {
            std::cout << "GPU not found. Falling back to CPU device" << std::endl;
            dType = CL_DEVICE_TYPE_CPU;
			std::cout << "Device type = CPU" << std::endl; //deviceType = "cpu"
        }
		std::cout << "Device type = GPU" << std::endl;
    }

    /* Have a look at the available platforms and pick either
    * the AMD one if available or a reasonable default.*/
    cl_platform_id platform = NULL;
    int retValue = getPlatform(platform, sampleArgs->platformId, sampleArgs->isPlatformEnabled());
    if (retValue != CL_SUCCESS) throw "getPlatform() failed";

    // Display available devices.
    retValue = displayDevices(platform, dType);
	if (retValue != CL_SUCCESS) throw "displayDevices() failed";

    // If we could find our platform, use it. Otherwise use just available platform.
    cl_context_properties cps[3] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0};

	//create context analoga me deviceType kai platform
    context = clCreateContextFromType(cps, dType, NULL, NULL, &status);
    if (status != CL_SUCCESS) throw "clCreateContextFromType failed.";

    //get device on which to run the sample
	status = getDevices(context, &devices, sampleArgs->deviceId, sampleArgs->isDeviceIdEnabled());
    if (status != CL_SUCCESS) throw "getDevices() failed";

    status = deviceInfo.setDeviceInfo(devices[sampleArgs->deviceId]);
    if (status != CL_SUCCESS) throw "deviceInfo.setDeviceInfo failed";

	//Check whether device doesn t support images
    if(!deviceInfo.imageSupport) throw "Expected Error: Device does not support Images";

	//Create command queue
    commandQueue = clCreateCommandQueue(context, devices[sampleArgs->deviceId], 0, &status);
    if (status != CL_SUCCESS) throw "clCreateCommandQueue failed.";

    //create a CL program using the kernel source
	buildProgramData buildData;
	buildData.kernelName = kernelsFileName;
    buildData.devices = devices;
    buildData.deviceId = sampleArgs->deviceId;
    buildData.flagsStr = std::string("");
    if(sampleArgs->isLoadBinaryEnabled()) buildData.binaryName = std::string(sampleArgs->loadBinary.c_str());
    if(sampleArgs->isComplierFlagsSpecified()) buildData.flagsFileName = std::string(sampleArgs->flags.c_str());
    retValue = buildOpenCLProgram(program, context, buildData);
    if (retValue != CL_SUCCESS) throw "buildOpenCLProgram() failed";

}


void giveMelOpenCL::runThisKernel(const char* kernelFileName, size_t *globalThreads, size_t *localThreads, 
							   cl_mem &buffer1, cl_mem &buffer2, cl_mem &buffer3,
							   cl_mem &buffer4, cl_mem &buffer5, cl_mem &buffer6,
							   cl_uint &parameter)
{
	int status = 0;

	cl_kernel kernelName;

	//create kernel
	kernelName = clCreateKernel(program, kernelFileName, &status);// "createColorArrays", &status);
    if (status != CL_SUCCESS) throw "clCreateKernel failed.(colorArraysKernel)";

	//push arguments
	pushArguments(kernelName, &buffer1, &buffer2, &buffer3, &buffer4, &buffer5, &buffer6, &parameter);

	//run kernel
	status = clEnqueueNDRangeKernel(commandQueue, kernelName, 2, NULL, globalThreads, localThreads, 0, NULL, 0);
    if (status != CL_SUCCESS) throw "clEnqueueNDRangeKernel failed.";

}

void giveMelOpenCL::runThisKernel(const char* kernelFileName, size_t *globalThreads, size_t *localThreads, 
							   cl_mem &buffer1, cl_mem &buffer2, cl_mem &buffer3,
							   cl_mem &imageName)
{
	int status = CL_SUCCESS;
	cl_kernel kernelName;

	//create kernel
	kernelName = clCreateKernel(program, kernelFileName, &status);// "createColorArrays", &status);
    if (status != CL_SUCCESS) throw "clCreateKernel failed.(colorArraysKernel)";

	//push arguments
	pushArguments(kernelName, &buffer1, &buffer2, &buffer3, &imageName);

	//run kernel
	status = clEnqueueNDRangeKernel(commandQueue, kernelName, 2, NULL, globalThreads, localThreads, 0, NULL, 0);
    if (status != CL_SUCCESS) throw "clEnqueueNDRangeKernel failed.";
}

int giveMelOpenCL::setTimer()
{
	// create and initialize timers
    int timer = sampleTimer->createTimer();
    sampleTimer->resetTimer(timer);
    sampleTimer->startTimer(timer);

	return timer;
}

void giveMelOpenCL::stopTimer(int timer)
{
	sampleTimer->stopTimer(timer);
    cl_double time = (double)(sampleTimer->readTimer(timer));

	std::cout << "Total time: " << time << " sec." << std::endl << std:: endl;
}

int giveMelOpenCL::cleanup()
{
    if(!byteRWSupport)
    {
        return SDK_SUCCESS;
    }

    if (clReleaseProgram(program) != CL_SUCCESS) throw "clReleaseProgram failed.(program)";
    if (clReleaseCommandQueue(commandQueue) != CL_SUCCESS) throw "clReleaseCommandQueue failed.(commandQueue)";
    if (clReleaseContext(context) != CL_SUCCESS) throw "clReleaseContext failed.(context)";
    FREE(devices);

    return SDK_SUCCESS;
}

/*void SimpleImage::printStats()
{
	if(sampleArgs->timing)
    {
        std::string strArray[4] =
        {
            "Width",
            "Height",
            "Time(sec)",
            "kernelTime(sec)"
        };
        std::string stats[4];

        sampleTimer->totalTime = setupTime + kernelTime;

        stats[0] = toString(width, std::dec);
        stats[1] = toString(height, std::dec);
        stats[2] = toString(sampleTimer->totalTime, std::dec);
        stats[3] = toString(kernelTime, std::dec);

        printStatistics(strArray, stats, 4);
    }
}*/

int main(int argc, char * argv[])
{
    try
	{
	    giveMelOpenCL clProvider;
		MyImage imageL; 
		MyImage imageR;
		int timer;
	
		imageL.open("diplo000000-L.bmp");
		imageR.open("diplo000000-R.bmp");

		timer = clProvider.setTimer();

		clProvider.setupCL("SimpleImage_Kernels.cl");
	
		imageL.histogramEqualization(&clProvider);
		imageR.histogramEqualization(&clProvider);

		clProvider.stopTimer(timer);

		imageL.save("myOutL.bmp");
		imageR.save("myOutR.bmp");
	}

	catch(char* expn){
		std::cout << "EXITING ERROR: " << expn << std::endl << std::endl;
	}

    return SDK_SUCCESS;
}
