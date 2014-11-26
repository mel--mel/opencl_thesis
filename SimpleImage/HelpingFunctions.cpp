#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "CLUtil.hpp"
#include "SDKBitMap.hpp"
#include "SimpleImage.hpp"
#include "myImage.hpp"


using namespace appsdk;


int compfunc(const void *pa, const void *pb){

    pixelStruct a, b;

    a = *(const pixelStruct*)pa;
    b = *(const pixelStruct*)pb;

    if (a.pxlValue == b.pxlValue && a.mo == b.mo){
        return 0;
    } else if ((a.pxlValue < b.pxlValue) || (a.pxlValue == b.pxlValue && a.mo < b.mo) ){
        return -1;
    } else {
        return 1;
    }
}

int getKernelWorkGroupSize(cl_device_id devId, cl_kernel kernelName, size_t &workGroupSize){

	int status = 0;

	    // Check group size against group size returned by kernel
    status = clGetKernelWorkGroupInfo(kernelName,
                                      devId,
								      //devices[sampleArgs->deviceId],
                                      CL_KERNEL_WORK_GROUP_SIZE,
                                      sizeof(size_t),
                                      &workGroupSize,
                                      0);
    CHECK_OPENCL_ERROR(status,"clGetKernelWorkGroupInfo  failed.");

	return CL_SUCCESS;
}

size_t findMinWorkGroupSize(int numOfKernels, cl_kernel *kernelNames, cl_device_id devId){

	size_t *kernelWorkGroupSize = (size_t*)calloc(numOfKernels, sizeof(size_t));
	CHECK_ALLOCATION(kernelWorkGroupSize, "Failed to allocate memory! (outputImageData)");

	for (int i = 0; i < numOfKernels; i++){ getKernelWorkGroupSize(devId, kernelNames[i], kernelWorkGroupSize[i]);}
	
	size_t tempMin = kernelWorkGroupSize[0];
	for (int i = 1; i < numOfKernels; i++){ tempMin = min(tempMin, kernelWorkGroupSize[i]); }

	return tempMin;
}

void histEqualization(pixelStruct *colorArray, cl_uint height, cl_uint width){

    cl_uint i, j; 
	int value, count, acc255;
    float mo;
    double acc;
	
	//sort
	qsort(colorArray, (width * height), sizeof(pixelStruct), compfunc);

    //equalize
    acc = 0;
    count = 1;
    value = colorArray[0].pxlValue;
    mo = colorArray[0].mo;
    for (i = 1; i < (width * height); i++){
        if ((colorArray[i].pxlValue == value) && (colorArray[i].mo == mo)){
            //count how many values are the same
            count++;
        }
        else {
            //find equalized value
            acc += (double)count/((double)(width * height));
            acc255 = (int)(acc*255);

            //for every same pixel
            //save transformation + change pixel to equalized value
            for (j = (i - count); j < i; j++){
                colorArray[j].trsfrm = acc255 - colorArray[j].pxlValue;
                if (acc255 < 255){
                    colorArray[j].pxlValue = acc255;
                }else{
                    colorArray[j].pxlValue = 255;
                }
            }

            //start counting again
            count = 1;
            value = colorArray[i].pxlValue;
            mo = colorArray[i].mo;
        }

    }
}

void setZero(pixelStruct *array, cl_uint height, cl_uint width)
{
	for (cl_uint i = 0; i < height; i++){
		for (cl_uint j = 0; j < width; j++){
			array[i*width + j].mo = 0;
			array[i*width + j].pxlValue = 0;}}
}

int copyFromArraysToBuffers(cl_command_queue cmdQueue,  cl_uint width, cl_uint height, 
							int numOfBuffers, cl_mem *buffers, pixelStruct **arrays){

	int status = 0;

	for (int i = 0; i < numOfBuffers; i++){
		status = clEnqueueWriteBuffer(cmdQueue,
 								 buffers[i],
 								 1,
 								 0,
 								 width * height * sizeof(pixelStruct),
								 arrays[i],
 								 0, 0, 0);
		CHECK_OPENCL_ERROR(status,"clEnqueueWriteBuffer failed. (copyFromArraysToBuffers)");
	}

	status = clFinish(cmdQueue);
    CHECK_OPENCL_ERROR(status,"clFinish failed.(cmdQueue)");

	return CL_SUCCESS;

}

int copyFromBuffersToArrays(cl_command_queue cmdQueue,  cl_uint width, cl_uint height, 
							int numOfBuffers, cl_mem *buffers, pixelStruct **arrays){

	int status = 0;

	for (int i = 0; i < numOfBuffers; i++){
		status = clEnqueueReadBuffer(cmdQueue,
 								 buffers[i],
 								 1,
 								 0,
 								 width * height * sizeof(pixelStruct),
								 arrays[i],
 								 0, 0, 0);
		CHECK_OPENCL_ERROR(status,"clEnqueueWriteBuffer failed. (copyFromArraysToBuffers)");
	}

	status = clFinish(cmdQueue);
    CHECK_OPENCL_ERROR(status,"clFinish failed.(cmdQueue)");

	return CL_SUCCESS;

}

int createArrays(pixelStruct*** arrays, cl_uint width, cl_uint height, int num)
{
	// allocate memory for 1D pixel struct array
	for (int i = 0; i < num; i++){
		*arrays[i] = (pixelStruct*)calloc(width * height, sizeof(pixelStruct));
		CHECK_ALLOCATION(arrays[i], "Failed to allocate memory! (redArray)");
	}

	return SDK_SUCCESS;
}

int createBuffer(cl_mem &bufferName, pixelStruct *arrayName, cl_context context, cl_command_queue commandQueue, cl_uint width, cl_uint height){

	int status = 0;
	
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

int createBuffers(cl_mem **buffers, pixelStruct **arrays, int num, cl_context context, cl_command_queue commandQueue, cl_uint width, cl_uint height){

	for (int i = 0; i < num; i++){
		createBuffer(*buffers[i], arrays[i], context, commandQueue, width, height);
	}

	return CL_SUCCESS;
}

void MyImage::open(std::string imageName){

	//SDKBitMap inputBitmap;
    // load input bitmap image
    imageBitmap.load(imageName.c_str());

    // error if image did not load
    if(!imageBitmap.isLoaded()){
        error("Failed to load input image!");
        throw "Failed to load input image!";
    }

    // get width and height of input image
    height = imageBitmap.getHeight();
    width = imageBitmap.getWidth();

    // allocate memory for input & outputimage data
    imageDataIn = (cl_uchar4*)malloc(width * height * sizeof(cl_uchar4));
    if (imageDataIn == NULL) throw "ERROR: Failed to allocate memory! (inputImageData)";

    // get the pointer to pixel data
    pixelData = imageBitmap.getPixels();
    if (pixelData == NULL) throw "ERROR: Failed to read pixel Data!";

    // Copy pixel data into inputImageData
	memcpy(imageDataIn, pixelData, width * height * sizeof(uchar4));

	//parameter imageDesc needed for clGreateImage
    memset(&imageDesc, '\0', sizeof(cl_image_desc));
    imageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
    imageDesc.image_width = width;
    imageDesc.image_height = height;

	std::cout << imageName << " opened" << std::endl << std::endl;

}

int MyImage::save(std::string imageName){


	// copy output image data back to original pixel data
    memcpy(pixelData, imageDataOut, width * height * sizeof(uchar4));

    // write the output bmp file
    if(!imageBitmap.write(imageName.c_str()))
    {
        std::cout << "Failed to write output image!";
        return SDK_FAILURE;
    }
    
	std::cout << imageName << " saved" << std::endl << std::endl;

    return SDK_SUCCESS;

}

int MyImage::histogramEqualization(SimpleImage *clSimpleImage){

	cl_int status;

	size_t globalThreads[] = {width, height};
    size_t localThreads[] = {256, 1}; /**< Work-group size in x and y direction */

	imageIn = clCreateImage(clSimpleImage->context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &imageFormat, &imageDesc, imageDataIn, &status);
    CHECK_OPENCL_ERROR(status,"clCreateImage failed. (imageIn)");
	
	pixelStruct** arrays[] = {&redArray, &greenArray, &blueArray};
	CHECK_OPENCL_ERROR(createArrays(arrays, width, height, 3), "createArrays() failed");

	cl_mem *buffers[] = {&redBuffer, &greenBuffer, &blueBuffer};
	pixelStruct* arrayPointers[] = {redArray, greenArray, blueArray};
	CHECK_OPENCL_ERROR(createBuffers(buffers, arrayPointers, 3, clSimpleImage->context, clSimpleImage->commandQueue, width, height), "createBuffers() failed");
	
	status = clSimpleImage->runThisKernel("createColorArrays", globalThreads, localThreads, redBuffer, greenBuffer, blueBuffer, imageIn);
	CHECK_OPENCL_ERROR(status,"runThisKernel-createColorArrays failed.");
	
	//Copy from buffers to color arrays
	cl_mem bufferPointers[] = {redBuffer, greenBuffer, blueBuffer};
	copyFromBuffersToArrays(clSimpleImage->commandQueue, width, height, 3, bufferPointers, arrayPointers);

	//equalize color arrays
	histEqualization(redArray, height, width);
	histEqualization(greenArray, height, width);
	histEqualization(blueArray, height, width);

	//Copy from arrays to buffers
	copyFromArraysToBuffers(clSimpleImage->commandQueue,  width, height, 3, bufferPointers, arrayPointers);
	
	//Create args and run createPixelArray
	cl_mem *sortedBuffers[] = {&redSortedBuffer, &greenSortedBuffer, &blueSortedBuffer};
	CHECK_OPENCL_ERROR(createBuffers(sortedBuffers, arrayPointers, 3, clSimpleImage->context, clSimpleImage->commandQueue, width, height), "createBuffers() failed");

	status = clSimpleImage->runThisKernel("createPixelArray", globalThreads, localThreads, 
						   redBuffer, greenBuffer, blueBuffer,
		                   redSortedBuffer, greenSortedBuffer, blueSortedBuffer,
						   width);
	CHECK_OPENCL_ERROR(status,"runThisKernel failed.");

		//create args and run CreateOutputImage
	imageOut = clCreateImage(clSimpleImage->context, CL_MEM_WRITE_ONLY, &imageFormat, &imageDesc, 0, &status);
    CHECK_OPENCL_ERROR(status,"clCreateImage failed. (outputImage2D)");

	status = clSimpleImage->runThisKernel("createOutputImage", globalThreads, localThreads, 
		                   redSortedBuffer, greenSortedBuffer, blueSortedBuffer,
						   imageOut);
	CHECK_OPENCL_ERROR(status,"runThisKernel failed.");
	
	//Read Output Image to output data
    size_t origin[] = {0, 0, 0};
    size_t region[] = {width, height, 1};

	// allocate memory for 2D-copy output image data
    imageDataOut = (cl_uchar4*)calloc(width * height, sizeof(cl_uchar4));
    CHECK_ALLOCATION(imageDataOut, "Failed to allocate memory! (outputImageData)");

    status = clEnqueueReadImage(clSimpleImage->commandQueue, imageOut, 1, origin, region, 0, 0, imageDataOut, 0, 0, 0);
    CHECK_OPENCL_ERROR(status,"clEnqueueReadImage failed.");

	status = clFinish(clSimpleImage->commandQueue);
    CHECK_OPENCL_ERROR(status,"clFinish failed.(commandQueue)");
	
	return SDK_SUCCESS;
}

int MyImage::cleanup(){

    FREE(imageDataIn);
	FREE(imageDataOut);
	FREE(redArray);
	FREE(greenArray);
	FREE(blueArray);

    return SDK_SUCCESS;

}