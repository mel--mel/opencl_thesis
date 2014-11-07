#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "CLUtil.hpp"
#include "SDKBitMap.hpp"
#include "SimpleImage.hpp"

using namespace appsdk;

template <typename T> 
int pushArgument(cl_kernel kernel, T* a, cl_uint id)
{
	cl_int status = clSetKernelArg(kernel, id, sizeof(T), a);
	CHECK_OPENCL_ERROR(status, "setKernelArg failed.");

	return SDK_SUCCESS;
}
template <typename T1> 
inline void pushArguments(cl_kernel kernel, T1* a1)
{
	pushArgument(kernel, a1, 0);
}
template <typename T1, typename T2> 
inline void pushArguments(cl_kernel kernel, T1* a1, T2* a2)
{
	pushArgument(kernel, a1, 0);
	pushArgument(kernel, a2, 1);
}
template <typename T1, typename T2, typename T3> 
inline void pushArguments(cl_kernel kernel, T1* a1, T2* a2, T3* a3)
{
	pushArgument(kernel, a1, 0);
	pushArgument(kernel, a2, 1);
	pushArgument(kernel, a3, 2);
}
template <typename T1, typename T2, typename T3, typename T4> 
inline void pushArguments(cl_kernel kernel, T1* a1, T2* a2, T3* a3, T4* a4)
{
	pushArgument(kernel, a1, 0);
	pushArgument(kernel, a2, 1);
	pushArgument(kernel, a3, 2);
	pushArgument(kernel, a4, 3);
}
template <typename T1, typename T2, typename T3, typename T4, typename T5> 
inline void pushArguments(cl_kernel kernel, T1* a1, T2* a2, T3* a3, T4* a4, T5* a5)
{
	pushArgument(kernel, a1, 0);
	pushArgument(kernel, a2, 1);
	pushArgument(kernel, a3, 2);
	pushArgument(kernel, a4, 3);
	pushArgument(kernel, a5, 4);
}
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> 
inline void pushArguments(cl_kernel kernel, T1* a1, T2* a2, T3* a3, T4* a4, T5* a5, T6* a6)
{
	pushArgument(kernel, a1, 0);
	pushArgument(kernel, a2, 1);
	pushArgument(kernel, a3, 2);
	pushArgument(kernel, a4, 3);
	pushArgument(kernel, a5, 4);
	pushArgument(kernel, a6, 5);
}
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> 
inline void pushArguments(cl_kernel kernel, T1* a1, T2* a2, T3* a3, T4* a4, T5* a5, T6* a6, T7* a7)
{
	pushArgument(kernel, a1, 0);
	pushArgument(kernel, a2, 1);
	pushArgument(kernel, a3, 2);
	pushArgument(kernel, a4, 3);
	pushArgument(kernel, a5, 4);
	pushArgument(kernel, a6, 5);
	pushArgument(kernel, a7, 6);
}

