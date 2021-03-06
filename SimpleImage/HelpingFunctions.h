#ifndef HELPINGFUNCTIONS_H
#define HELPINGFUNCTIONS_H
 
int compfunc(const void *pa, const void *pb);

int getKernelWorkGroupSize(cl_device_id devId, cl_kernel kernelName, size_t &workGroupSize);

void histEqualization(pixelStruct *colorArray, cl_uint height, cl_uint width);

void setZero(pixelStruct *array, cl_uint height, cl_uint width);

size_t findMinWorkGroupSize(int numOfKernels, cl_kernel *kernelNames, cl_device_id devId);
 
int copyFromArraysToBuffers(cl_command_queue cmdQueue,  cl_uint width, cl_uint height, int numOfBuffers, cl_mem *buffers, pixelStruct **arrays);//cl_mem buffer1, pixelStruct* array1);

int copyFromBuffersToArrays(cl_command_queue cmdQueue,  cl_uint width, cl_uint height, int numOfBuffers, cl_mem *buffers, pixelStruct **arrays);

void createArrays(pixelStruct*** arrays, cl_uint width, cl_uint height, int num);

void createBuffer(cl_mem &bufferName, pixelStruct *arrayName, cl_context context, cl_command_queue commandQueue, cl_uint width, cl_uint height);

void createBuffers(cl_mem **buffers, pixelStruct **arrays, int num, cl_context context, cl_command_queue commandQueue, cl_uint width, cl_uint height);

#endif