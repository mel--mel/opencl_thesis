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

int SimpleImage::createBuffer(cl_mem &bufferName, pixelStruct *arrayName){

	int status = 0;

	/*arrayName = (pixelStruct*)calloc(width * height, sizeof(pixelStruct));
	CHECK_ALLOCATION(arrayName,
                     "Failed to allocate memory!");
	*/
	
	bufferName = clCreateBuffer(context, 
		                       CL_MEM_READ_WRITE, 
							   width * height * sizeof(pixelStruct), 
							   NULL, 
							   &status);
	CHECK_OPENCL_ERROR(status,"clCreateBuffer failed. (redBuffer)");

	status = clEnqueueWriteBuffer(commandQueue,
 								 bufferName,
 								 1,
 								 0,
 								 width * height * sizeof(pixelStruct),
								 arrayName,
 								 0, 0, 0);
	CHECK_OPENCL_ERROR(status,"clEnqueueWriteBuffer failed. (redBuffer)");

	return CL_SUCCESS;
}

int SimpleImage::getInputImage(std::string imageName, cl_image_desc *imageDesc, cl_uchar4 **imageData)
{
	// Allocate host memoryF and read input image
    //std::string filePath = getPath() + std::string(INPUT_IMAGE);
	CHECK_OPENCL_ERROR(readInputImage(imageName, imageDesc, imageData), "Read Input Image failed");

	//enable timing for this SimpleImage
	sampleArgs->timing = 1;

	return CL_SUCCESS;
}

int SimpleImage::readInputImage(std::string imageName, cl_image_desc *imageDesc, cl_uchar4 **imageData)
{

	//SDKBitMap inputBitmap;
    // load input bitmap image
    inputBitmap.load(imageName.c_str());

    // error if image did not load
    if(!inputBitmap.isLoaded()){
        error("Failed to load input image!");
        return SDK_FAILURE;
    }

    // get width and height of input image
    height = inputBitmap.getHeight();
    width = inputBitmap.getWidth();

    // allocate memory for input & outputimage data
    *imageData = (cl_uchar4*)malloc(width * height * sizeof(cl_uchar4));
    CHECK_ALLOCATION(imageData,"Failed to allocate memory! (inputImageData)");

    // get the pointer to pixel data
    pixelData = inputBitmap.getPixels();
    CHECK_ALLOCATION(pixelData,"Failed to read pixel Data!");

    // Copy pixel data into inputImageData
    memcpy(*imageData, pixelData, width * height * pixelSize);

    // allocate memory for verification output
    verificationOutput = (cl_uchar*)malloc(width * height * pixelSize);
    CHECK_ALLOCATION(pixelData,"verificationOutput heap allocation failed!");

    // initialize the data to NULL
    //memset(verificationOutput, 0, width * height * pixelSize);
    memcpy(verificationOutput, *imageData, width * height * pixelSize);

	//parameter imageDesc needed for clGreateImage
    memset(imageDesc, '\0', sizeof(cl_image_desc));
    imageDesc->image_type = CL_MEM_OBJECT_IMAGE2D;
    imageDesc->image_width = width;
    imageDesc->image_height = height;

    return SDK_SUCCESS;

}

int SimpleImage::createColorArrays()
{
	// allocate memory for 1D pixel struct array
	redArray = (pixelStruct*)calloc(width * height, sizeof(pixelStruct));
	CHECK_ALLOCATION(redArray, "Failed to allocate memory! (redArray)");

	greenArray = (pixelStruct*)calloc(width * height, sizeof(pixelStruct));
	CHECK_ALLOCATION(greenArray, "Failed to allocate memory! (greenArray)");
	
	blueArray = (pixelStruct*)calloc(width * height, sizeof(pixelStruct));
	CHECK_ALLOCATION(blueArray, "Failed to allocate memory! (blueArray)");

	return SDK_SUCCESS;
}

int SimpleImage::writeOutputImage(std::string outputImageName)
{
    // copy output image data back to original pixel data
    memcpy(pixelData, outputImageData2D, width * height * pixelSize);

    // write the output bmp file
    if(!inputBitmap.write(outputImageName.c_str()))
    {
        std::cout << "Failed to write output image!";
        return SDK_FAILURE;
    }
    return SDK_SUCCESS;
}

int SimpleImage::genBinaryImage()
{
    bifData binaryData;
    binaryData.kernelName = std::string("SimpleImage_Kernels.cl");
    binaryData.flagsStr = std::string("");
    if(sampleArgs->isComplierFlagsSpecified())
    {
        binaryData.flagsFileName = std::string(sampleArgs->flags.c_str());
    }

    binaryData.binaryName = std::string(sampleArgs->dumpBinary.c_str());
    int status = generateBinaryImage(binaryData);
    return status;
}

int SimpleImage::setupCL()
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
    CHECK_OPENCL_ERROR(retValue, "getPlatform() failed");

    // Display available devices.
    retValue = displayDevices(platform, dType);
    CHECK_OPENCL_ERROR(retValue, "displayDevices() failed");

    // If we could find our platform, use it. Otherwise use just available platform.
    cl_context_properties cps[3] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0};

	//create context analoga me deviceType kai platform
    context = clCreateContextFromType(cps, dType, NULL, NULL, &status);
    CHECK_OPENCL_ERROR(status, "clCreateContextFromType failed.");

    //get device on which to run the sample
	status = getDevices(context, &devices, sampleArgs->deviceId, sampleArgs->isDeviceIdEnabled());
    CHECK_OPENCL_ERROR(status, "getDevices() failed");

    status = deviceInfo.setDeviceInfo(devices[sampleArgs->deviceId]);
    CHECK_OPENCL_ERROR(status, "deviceInfo.setDeviceInfo failed");

	//Check whether device doesn t support images
    if(!deviceInfo.imageSupport) { OPENCL_EXPECTED_ERROR(" Expected Error: Device does not support Images"); }

	//Create command queue
    commandQueue = clCreateCommandQueue(context, devices[sampleArgs->deviceId], 0, &status);
    CHECK_OPENCL_ERROR(status,"clCreateCommandQueue failed.");

    //create a CL program using the kernel source
	buildProgramData buildData;
    buildData.kernelName = std::string("SimpleImage_Kernels.cl");
    buildData.devices = devices;
    buildData.deviceId = sampleArgs->deviceId;
    buildData.flagsStr = std::string("");
    if(sampleArgs->isLoadBinaryEnabled()) buildData.binaryName = std::string(sampleArgs->loadBinary.c_str());
    if(sampleArgs->isComplierFlagsSpecified()) buildData.flagsFileName = std::string(sampleArgs->flags.c_str());
    retValue = buildOpenCLProgram(program, context, buildData);
    CHECK_ERROR(retValue, SDK_SUCCESS, "buildOpenCLProgram() failed");

	return SDK_SUCCESS;

}

int SimpleImage::checkResources(int numOfKernels, cl_kernel *kernelNames){ // Check group size against group size returned by kernel

	cl_device_id devId = devices[sampleArgs->deviceId];
	
	size_t minWorkGroupSize = findMinWorkGroupSize(numOfKernels, kernelNames, devId);

    if((blockSizeX * blockSizeY) > minWorkGroupSize) {
        if(!sampleArgs->quiet){
            std::cout << "Out of Resources!" << std::endl;
            std::cout << "Group Size specified : "<< blockSizeX * blockSizeY << std::endl;
            std::cout << "Max Group Size supported on the kernel(s) : "<< minWorkGroupSize << std::endl;
            std::cout << "Falling back to " << minWorkGroupSize << std::endl;
        }
		if(blockSizeX > minWorkGroupSize){
            blockSizeX = minWorkGroupSize;
            blockSizeY = 1;
        }
    }

	return CL_SUCCESS;
}

int SimpleImage::runThisKernel(const char* kernelFileName, size_t *globalThreads, size_t *localThreads, 
							   cl_mem &buffer1, cl_mem &buffer2, cl_mem &buffer3,
							   cl_mem &buffer4, cl_mem &buffer5, cl_mem &buffer6,
							   cl_uint &parameter)
{
	int status = 0;

	cl_kernel kernelName;

	//create kernel
	kernelName = clCreateKernel(program, kernelFileName, &status);// "createColorArrays", &status);
    CHECK_OPENCL_ERROR(status,"clCreateKernel failed.(colorArraysKernel)");

	//push arguments
	pushArguments(kernelName, &buffer1, &buffer2, &buffer3, &buffer4, &buffer5, &buffer6, &parameter);

	//run kernel
	status = clEnqueueNDRangeKernel(commandQueue, kernelName, 2, NULL, globalThreads, localThreads, 0, NULL, 0);
    CHECK_OPENCL_ERROR(status,"clEnqueueNDRangeKernel failed.");

	return CL_SUCCESS;

}



int SimpleImage::runThisKernel(const char* kernelFileName, size_t *globalThreads, size_t *localThreads, 
							   cl_mem &buffer1, cl_mem &buffer2, cl_mem &buffer3,
							   cl_mem &imageName)
{
	int status = 0;

	cl_kernel kernelName;

	//create kernel
	kernelName = clCreateKernel(program, kernelFileName, &status);// "createColorArrays", &status);
    CHECK_OPENCL_ERROR(status,"clCreateKernel failed.(colorArraysKernel)");

	//push arguments
	pushArguments(kernelName, &buffer1, &buffer2, &buffer3, &imageName);

	//run kernel
	status = clEnqueueNDRangeKernel(commandQueue, kernelName, 2, NULL, globalThreads, localThreads, 0, NULL, 0);
    CHECK_OPENCL_ERROR(status,"clEnqueueNDRangeKernel failed.");

	return CL_SUCCESS;

}

int SimpleImage::runCLKernels()
{
    cl_int status;

	// Enqueue a kernel run call.
    size_t globalThreads[] = {width, height};
    size_t localThreads[] = {blockSizeX, blockSizeY};

	//Create args
	inputImage2D = clCreateImage(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &imageFormat, &imageDesc1, imageData1, &status);
    CHECK_OPENCL_ERROR(status,"clCreateImage failed. (inputImage2D)");

	CHECK_OPENCL_ERROR(createColorArrays(), "createColorArrays() failed");

	createBuffer(redBuffer, redArray);
	createBuffer(greenBuffer, greenArray);
	createBuffer(blueBuffer, blueArray);

	status = runThisKernel("createColorArrays", globalThreads, localThreads, redBuffer, greenBuffer, blueBuffer, inputImage2D);
	CHECK_OPENCL_ERROR(status,"runThisKernel failed.");

	//Copy from buffers to color arrays
	cl_mem buffers[] = {redBuffer, greenBuffer, blueBuffer};
	pixelStruct *arrays[] = {redArray, greenArray, blueArray};
	copyFromBuffersToArrays(commandQueue, width, height, 3, buffers, arrays);

	//equalize color arrays
	histogramEqualization(redArray, height, width);
	histogramEqualization(greenArray, height, width);
	histogramEqualization(blueArray, height, width);

	//Copy from arrays to buffers
	copyFromArraysToBuffers(commandQueue,  width, height, 3, buffers, arrays);

	//Create args and run createPixelArray
	createBuffer(redSortedBuffer, redArray);
	createBuffer(greenSortedBuffer, greenArray);
	createBuffer(blueSortedBuffer, blueArray);

	status = runThisKernel("createPixelArray", globalThreads, localThreads, 
						   redBuffer, greenBuffer, blueBuffer,
		                   redSortedBuffer, greenSortedBuffer, blueSortedBuffer,
						   width);
	CHECK_OPENCL_ERROR(status,"runThisKernel failed.");

	//create args and run CreateOutputImage
	outputImage2D = clCreateImage(context, CL_MEM_WRITE_ONLY, &imageFormat, &imageDesc1, 0, &status);
    CHECK_OPENCL_ERROR(status,"clCreateImage failed. (outputImage2D)");

	status = runThisKernel("createOutputImage", globalThreads, localThreads, 
		                   redSortedBuffer, greenSortedBuffer, blueSortedBuffer,
						   outputImage2D);
	CHECK_OPENCL_ERROR(status,"runThisKernel failed.");

    //Read Output Image to output data
    size_t origin[] = {0, 0, 0};
    size_t region[] = {width, height, 1};

	// allocate memory for 2D-copy output image data
    outputImageData2D = (cl_uchar4*)calloc(width * height, sizeof(cl_uchar4));
    CHECK_ALLOCATION(outputImageData2D, "Failed to allocate memory! (outputImageData)");

    status = clEnqueueReadImage(commandQueue, outputImage2D, 1, origin, region, 0, 0, outputImageData2D, 0, 0, 0);
    CHECK_OPENCL_ERROR(status,"clEnqueueReadImage failed.");

	status = clFinish(commandQueue);
    CHECK_OPENCL_ERROR(status,"clFinish failed.(commandQueue)");

	/*cl_kernel kernelNames[] = {colorArraysKernel, pixelArrayKernel, outputImageKernel};
	checkResources(3, kernelNames);
	*/
    return SDK_SUCCESS;
}

int SimpleImage::initialize()
{

    // Call base class Initialize to get default configuration
    if (sampleArgs->initialize() != SDK_SUCCESS)
    {
        return SDK_FAILURE;
    }

    Option* iteration_option = new Option;
    CHECK_ALLOCATION(iteration_option,
                     "Memory Allocation error. (iteration_option)");

    iteration_option->_sVersion = "i";
    iteration_option->_lVersion = "iterations";
    iteration_option->_description = "Number of iterations to execute kernel";
    iteration_option->_type = CA_ARG_INT;
    iteration_option->_value = &iterations;

    sampleArgs->AddOption(iteration_option);

    delete iteration_option;

    return SDK_SUCCESS;
}

int SimpleImage::setup()
{
    int status = 0;

    // create and initialize timers
    int timer = sampleTimer->createTimer();
    sampleTimer->resetTimer(timer);
    sampleTimer->startTimer(timer);

	CHECK_OPENCL_ERROR(setupCL(), "setupCL() failed");

	CHECK_OPENCL_ERROR(getInputImage("diplo000000-L.bmp", &imageDesc1, &imageData1), "getInputImage1() failed");
	CHECK_OPENCL_ERROR(getInputImage("diplo000000-R.bmp", &imageDesc2, &imageData2), "getInputImage2() failed");

    sampleTimer->stopTimer(timer);
    // Compute setup time
    setupTime = (double)(sampleTimer->readTimer(timer));

    return SDK_SUCCESS;

}

int SimpleImage::run()
{
    if(!byteRWSupport)
    {
        return SDK_SUCCESS;
    }

    // create and initialize timers
    int timer = sampleTimer->createTimer();
    sampleTimer->resetTimer(timer);
    sampleTimer->startTimer(timer);

    // Set kernel arguments and run kernel
    CHECK_OPENCL_ERROR(runCLKernels(), "OpenCL run Kernel failed");

	// Compute kernel time
    sampleTimer->stopTimer(timer);
    kernelTime = (double)(sampleTimer->readTimer(timer)) / iterations;


    return SDK_SUCCESS;
}

int SimpleImage::cleanup()
{
    if(!byteRWSupport)
    {
        return SDK_SUCCESS;
    }

    // Releases OpenCL resources (Context, Memory etc.)
    //CHECK_OPENCL_ERROR(clReleaseKernel(colorArraysKernel),"clReleaseKernel failed.(colorArraysKernel)");
    CHECK_OPENCL_ERROR(clReleaseProgram(program),"clReleaseProgram failed.(program)");
    CHECK_OPENCL_ERROR(clReleaseMemObject(inputImage2D),"clReleaseMemObject failed.(inputImage2D)");
    CHECK_OPENCL_ERROR(clReleaseMemObject(outputImage2D),"clReleaseMemObject failed.(outputImage2D)");
    CHECK_OPENCL_ERROR(clReleaseCommandQueue(commandQueue),"clReleaseCommandQueue failed.(commandQueue)");
    CHECK_OPENCL_ERROR(clReleaseContext(context),"clReleaseContext failed.(context)");

    // release program resources (input memory etc.)
    FREE(imageData1);
	FREE(imageData2);
    FREE(outputImageData2D);
    FREE(verificationOutput);
    FREE(devices);

    return SDK_SUCCESS;
}

void SimpleImage::simpleImageCPUReference()
{

}

int SimpleImage::verifyResults()
{
	if(sampleArgs->verify){
        std::cout << "Verifying 2D copy result - ";
        // compare the results and see if they match
        if(!memcmp(imageData1, outputImageData2D, width * height * 4)) {std::cout << "Passed!\n" << std::endl;}
        else{
            std::cout << "Failed\n" << std::endl;
            return SDK_FAILURE;
        }  
    }
    return SDK_SUCCESS;
}

void SimpleImage::printStats()
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
}

int main(int argc, char * argv[])
{
    int status = 0;
    SimpleImage clSimpleImage;
	MyImage imageL; 
	MyImage imageR;

	imageL.open("diplo000000-L.bmp");
	imageR.open("diplo000000-R.bmp");

	imageL.save("myOutL.bmp");
	imageR.save("myOutR.bmp");

	CHECK_OPENCL_ERROR(clSimpleImage.setup(), "setup() failed");
	
	CHECK_OPENCL_ERROR(clSimpleImage.run(), "run() failed");

    CHECK_OPENCL_ERROR(clSimpleImage.writeOutputImage(OUTPUT_IMAGE), "write Output Image Failed");

	CHECK_OPENCL_ERROR(clSimpleImage.verifyResults(), "verifyResults() failed");

	CHECK_OPENCL_ERROR(clSimpleImage.cleanup(), "cleanup() failed");
	
    clSimpleImage.printStats();

    return SDK_SUCCESS;
}
