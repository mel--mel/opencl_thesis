#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "CLUtil.hpp"
#include "SDKBitMap.hpp"
#include "SimpleImage.hpp"


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

void histogramEqualization(pixelStruct *colorArray, cl_uint height, cl_uint width){

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
							//cl_mem buffer1, pixelStruct* array1, cl_mem buffer2, pixelStruct* array2, cl_mem buffer3, pixelStruct* array3
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

