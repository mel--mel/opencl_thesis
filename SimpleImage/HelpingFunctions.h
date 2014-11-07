#ifndef HELPINGFUNCTIONS_H
#define HELPINGFUNCTIONS_H
 
int compfunc(const void *pa, const void *pb);

int getKernelWorkGroupSize(cl_device_id devId, cl_kernel kernelName, size_t workGroupSize);

void histogramEqualization(pixelStruct *colorArray, cl_uint height, cl_uint width);

void setZero(pixelStruct *array, cl_uint height, cl_uint width);

 
#endif