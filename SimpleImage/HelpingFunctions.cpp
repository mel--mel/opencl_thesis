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

void createArrays(pixelStruct*** arrays, cl_uint width, cl_uint height, int num)
{
	// allocate memory for 1D pixel struct array
	for (int i = 0; i < num; i++){
		*arrays[i] = (pixelStruct*)calloc(width * height, sizeof(pixelStruct));
		if (arrays[i] == NULL) throw "Failed to allocate memory for some array!";
	}

}

void createBuffer(cl_mem &bufferName, pixelStruct *arrayName, cl_context context, cl_command_queue commandQueue, cl_uint width, cl_uint height){

	int status = 0;
	
	bufferName = clCreateBuffer(context, 
		                       CL_MEM_READ_WRITE, 
							   width * height * sizeof(pixelStruct), 
							   NULL, 
							   &status);
	if (status != CL_SUCCESS) throw "clCreateBuffer failed";

	status = clEnqueueWriteBuffer(commandQueue,
 								 bufferName,
 								 1,
 								 0,
 								 width * height * sizeof(pixelStruct),
								 arrayName,
 								 0, 0, 0);
	if (status != CL_SUCCESS) throw "clEnqueueWriteBuffer failed. (redBuffer)";
}

void createBuffers(cl_mem **buffers, pixelStruct **arrays, int num, cl_context context, cl_command_queue commandQueue, cl_uint width, cl_uint height){

	for (int i = 0; i < num; i++){
		createBuffer(*buffers[i], arrays[i], context, commandQueue, width, height);
	}
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

void MyImage::save(std::string imageName){

	// copy output image data back to original pixel data
    memcpy(pixelData, imageDataOut, width * height * sizeof(uchar4));

    // write the output bmp file
    if(!imageBitmap.write(imageName.c_str()))
    {
        throw "Failed to write output image!";
    }
}

void MyImage::imageToColorBuffers(giveMelOpenCL *clProvider){

	cl_int status;
	size_t globalThreads[] = {width, height}; /**< Work-group size in x and y direction */

	imageIn = clCreateImage(clProvider->context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &imageFormat, &imageDesc, imageDataIn, &status);
    if (status != CL_SUCCESS) throw "clCreateImage failed. (imageIn)";
	
	pixelStruct** arrays[] = {&redArray, &greenArray, &blueArray};
	createArrays(arrays, width, height, 3); 

	cl_mem *buffers[] = {&redBuffer, &greenBuffer, &blueBuffer};
	pixelStruct* arrayPointers[] = {redArray, greenArray, blueArray};
	createBuffers(buffers, arrayPointers, 3, clProvider->context, clProvider->commandQueue, width, height);
	
	clProvider->runThisKernel("createColorBuffers", globalThreads, redBuffer, greenBuffer, blueBuffer, imageIn);

	status = clFinish(clProvider->commandQueue);
    if (status != CL_SUCCESS) throw "clFinish failed.(commandQueue)";

}

void MyImage::putPixelsInRightPos(giveMelOpenCL *clProvider){

	int status = CL_SUCCESS;
	size_t globalThreads[] = {width, height}; /**< Work-group size in x and y direction */

	//Create args and run createPixelArray
	cl_mem _redBuffer, _greenBuffer, _blueBuffer;
	pixelStruct* arrayPointers[] = {redArray, greenArray, blueArray};
	cl_mem *sortedBuffers[] = {&_redBuffer, &_greenBuffer, &_blueBuffer};
	createBuffers(sortedBuffers, arrayPointers, 3, clProvider->context, clProvider->commandQueue, width, height);

    clProvider->runThisKernel("createPixelArray", globalThreads, redBuffer, greenBuffer, blueBuffer,
		                      _redBuffer, _greenBuffer, _blueBuffer, width);

	status = clFinish(clProvider->commandQueue);
    if (status != CL_SUCCESS) throw "clFinish failed.(commandQueue)";

	redBuffer = _redBuffer;
	greenBuffer = _greenBuffer;
	blueBuffer = _blueBuffer;
}

void MyImage::buffersToOutputImage(giveMelOpenCL *clProvider)
{
	int status = CL_SUCCESS;
	size_t globalThreads[] = {width, height}; /**< Work-group size in x and y direction */

	//create args and run CreateOutputImage
	imageOut = clCreateImage(clProvider->context, CL_MEM_WRITE_ONLY, &imageFormat, &imageDesc, 0, &status);
    if (status != CL_SUCCESS) throw "clCreateImage failed. (imageOut)";

	clProvider->runThisKernel("createOutputImage", globalThreads, redBuffer, greenBuffer, blueBuffer, imageOut);
	
	//Read Output Image to output data
    size_t origin[] = {0, 0, 0};
    size_t region[] = {width, height, 1};

	// allocate memory for 2D-copy output image data
    imageDataOut = (cl_uchar4*)calloc(width * height, sizeof(cl_uchar4));
    if (imageDataOut == NULL) throw "Failed to allocate memory! (imageDataOut)";

    status = clEnqueueReadImage(clProvider->commandQueue, imageOut, 1, origin, region, 0, 0, imageDataOut, 0, 0, 0);
    if (status != CL_SUCCESS) throw "clEnqueueReadImage failed.";
	
	status = clFinish(clProvider->commandQueue);
    if (status != CL_SUCCESS) throw "clFinish failed.(commandQueue)";
}

void MyImage::histEqualizeColors(giveMelOpenCL *clProvider){

	cl_int status;
	size_t globalThreads[] = {width, height}; /**< Work-group size in x and y direction */
	
	//Copy from buffers to color arrays
	pixelStruct* arrayPointers[] = {redArray, greenArray, blueArray};
	cl_mem bufferPointers[] = {redBuffer, greenBuffer, blueBuffer};
	copyFromBuffersToArrays(clProvider->commandQueue, width, height, 3, bufferPointers, arrayPointers);

	//equalize color arrays
	histEqualization(redArray, height, width);
	histEqualization(greenArray, height, width);
	histEqualization(blueArray, height, width);

	//Copy from arrays to buffers
	copyFromArraysToBuffers(clProvider->commandQueue,  width, height, 3, bufferPointers, arrayPointers);

	status = clFinish(clProvider->commandQueue);
    if (status != CL_SUCCESS) throw "clFinish failed.(commandQueue)";
}

void MyImage::matchHistograms(giveMelOpenCL *clProvider, MyImage *imageRef){

	cl_int status;
	size_t globalThreads[] = {width, height}; /**< Work-group size in x and y direction */
	
	clProvider->runThisKernel("histMatching", globalThreads, redBuffer, greenBuffer, blueBuffer,
						   imageRef->redBuffer, imageRef->greenBuffer, imageRef->blueBuffer, width);
	
	status = clFinish(clProvider->commandQueue);
    if (status != CL_SUCCESS) throw "clFinish failed.(commandQueue)";

}

void MyImage::histogramMatching(giveMelOpenCL *clProvider, MyImage *imageRef){

	histEqualizeColors(clProvider);
	imageRef->histEqualizeColors(clProvider);

	matchHistograms(clProvider, imageRef);

	putPixelsInRightPos(clProvider);
	imageRef->putPixelsInRightPos(clProvider);

}

void MyImage::cleanup(){

    FREE(imageDataIn);
	FREE(imageDataOut);
	FREE(redArray);
	FREE(greenArray);
	FREE(blueArray);
}