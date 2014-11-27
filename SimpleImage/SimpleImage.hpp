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

#ifndef SIMPLE_IMAGE_H_
#define SIMPLE_IMAGE_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "CLUtil.hpp"
#include "SDKBitMap.hpp"

using namespace appsdk;

#define SAMPLE_VERSION "AMD-APP-SDK-v2.9.233.1"

#define GROUP_SIZE 256

#ifndef min
#define min(a, b)            (((a) < (b)) ? (a) : (b))
#endif

typedef struct pixelStruct{
    cl_uint pxlValue;
    cl_float mo;
    cl_int trsfrm;
    cl_uint row;
    cl_uint col;
	cl_int indx;
} pixelStruct;

/**
* SimpleImage
* Class implements OpenCL Simple Image sample
*/

class giveMelOpenCL
{       
        cl_device_id *devices;              /**< CL device list */
        SDKDeviceInfo deviceInfo;                    /**< Structure to store device information*/

		cl_program program;                 /**< CL program  */
		
        cl_bool byteRWSupport;
        cl_bool imageSupport;               /**< Flag to check whether images are supported */
		
        KernelWorkGroupInfo kernelInfo;              /**< Structure to store kernel related info */

        SDKTimer    *sampleTimer;      /**< SDKTimer object */

    public:

		cl_context context;                 /**< CL context */
		cl_command_queue commandQueue;      /**< CL command queue */
        CLCommandArgs   *sampleArgs;   /**< CLCommand argument class */


        /**
        * Constructor
        * Initialize member variables
        */
        giveMelOpenCL()
            : byteRWSupport(true)
        {
            sampleTimer = new SDKTimer();
            sampleArgs = new CLCommandArgs() ;
			sampleArgs->sampleVerStr = SAMPLE_VERSION;
        }

        ~giveMelOpenCL()
        {
			cleanup();
        }

        /**
        * OpenCL related initialisations.
        * Set up Context, Device list, Command Queue, Memory buffers
        * Build CL kernel program executable
        * @return SDK_SUCCESS on success and SDK_FAILURE on failure
        */
        void setupCL();

		void compileKernels(std::string kernelsFileName);

		/**
		** Makes the moves needed to run a kernel
		*/
		void runThisKernel(const char* kernelNameInFile, size_t  *globalThreads, size_t *localThreads, 
						cl_mem &buffer1, cl_mem &buffer2, cl_mem &buffer3,
						cl_mem &imageName);

		void runThisKernel(const char* kernelNameInFile, size_t *globalThreads, size_t *localThreads, 
							   cl_mem &buffer1, cl_mem &buffer2, cl_mem &buffer3,
							   cl_mem &buffer4, cl_mem &buffer5, cl_mem &buffer6,
							   cl_uint &width);

		int setTimer();

		void stopTimer(int timer);

        /**
        * Override from SDKSample. Print sample stats.
        */
        void printStats();

        /**
        * Override from SDKSample
        * Cleanup memory allocations
        * @return  SDK_SUCCESS on success and SDK_FAILURE on failure
        */
        int cleanup();
};

#endif // SIMPLE_IMAGE_H_
