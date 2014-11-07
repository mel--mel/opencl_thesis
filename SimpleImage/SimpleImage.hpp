/**********************************************************************
Copyright �2013 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

�   Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
�   Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
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

class SimpleImage
{
        cl_double setupTime;                /**< time taken to setup OpenCL resources and building kernel */
        cl_double kernelTime;               /**< time taken to run kernel and read result back */
        cl_uchar4* inputImageData;          /**< Input bitmap data to device */
        cl_uchar4* outputImageData2D;       /**< Output from device for 2D copy*/
        cl_uchar4* outputImageData3D;       /**< Output from device for 3D copy*/
        cl_context context;                 /**< CL context */
        cl_device_id *devices;              /**< CL device list */

        cl_mem inputImage2D;                /**< CL image buffer for input Image*/
        cl_mem inputImage3D;                /**< CL image buffer for input Image*/
        cl_mem outputImage2D;               /**< CL image buffer for Output Image*/
        cl_mem outputImage3D;               /**< CL image buffer for Output Image*/
		cl_mem pixelStructBuffer;           /**< CL image buffer for pixelStructArray*/
		cl_mem redBuffer;                   /**< CL image buffer for pixelStructArray*/
		cl_mem greenBuffer;                 /**< CL image buffer for pixelStructArray*/
		cl_mem blueBuffer;                  /**< CL image buffer for pixelStructArray*/
		cl_mem redSortedBuffer;                   /**< CL image buffer for pixelStructArray*/
		cl_mem greenSortedBuffer;                 /**< CL image buffer for pixelStructArray*/
		cl_mem blueSortedBuffer;                  /**< CL image buffer for pixelStructArray*/

        cl_uchar* verificationOutput;       /**< Output array for reference implementation */
        cl_command_queue commandQueue;      /**< CL command queue */
        cl_program program;                 /**< CL program  */

        cl_kernel kernel2D;                 /**< CL kernel */
        cl_kernel kernel3D;                 /**< CL kernel */
		cl_kernel colorArraysKernel;
		cl_kernel outputImageKernel;
		cl_kernel pixelArrayKernel;

        SDKBitMap inputBitmap;   /**< Bitmap class object */
        uchar4* pixelData;       /**< Pointer to image data */
        cl_uint pixelSize;                  /**< Size of a pixel in BMP format> */
        cl_uint width;                      /**< Width of image */
        cl_uint height;                     /**< Height of image */
        cl_bool byteRWSupport;
		cl_uint4* pixelArray; /**< 1D Array to help image reconstruction */
		pixelStruct* redArray; /**< 1D Struct to help equalization */
		pixelStruct* greenArray; /**< 1D Struct to help equalization */
		pixelStruct* blueArray; /**< 1D Struct to help equalization */

        size_t kernel2DWorkGroupSize;         /**< Group Size returned by kernel */
        size_t kernel3DWorkGroupSize;         /**< Group Size returned by kernel */
		size_t colorArraysKernelWorkGroupSize;
		size_t outputImageKernelWorkGroupSize;
		size_t pixelArrayKernelWorkGroupSize;

        size_t blockSizeX;                  /**< Work-group size in x-direction */
        size_t blockSizeY;                  /**< Work-group size in y-direction */

        int iterations;                     /**< Number of iterations for kernel execution */
        cl_bool imageSupport;               /**< Flag to check whether images are supported */
        cl_image_format imageFormat;        /**< Image format descriptor */
        SDKDeviceInfo
        deviceInfo;                    /**< Structure to store device information*/
        KernelWorkGroupInfo
        kernelInfo;              /**< Structure to store kernel related info */

        SDKTimer    *sampleTimer;      /**< SDKTimer object */

    public:

        CLCommandArgs   *sampleArgs;   /**< CLCommand argument class */

        /**
        * Read bitmap image and allocate host memory
        * @param inputImageName name of the input file
        * @return SDK_SUCCESS on success and SDK_FAILURE on failure
        */
        int readInputImage(std::string inputImageName);

        /**
        * Write to an image file
        * @param outputImageName name of the output file
        * @return SDK_SUCCESS on success and SDK_FAILURE on failure
        */
        int writeOutputImage(std::string outputImageName);

        /**
        * Constructor
        * Initialize member variables
        */
        SimpleImage()
            : inputImageData(NULL),
              outputImageData2D(NULL),
              outputImageData3D(NULL),
              verificationOutput(NULL),
              byteRWSupport(true)
        {
            sampleArgs = new CLCommandArgs() ;
            sampleTimer = new SDKTimer();
            sampleArgs->sampleVerStr = SAMPLE_VERSION;
            pixelSize = sizeof(uchar4);
            pixelData = NULL;
            blockSizeX = GROUP_SIZE;
            blockSizeY = 1;
            iterations = 1;
            imageFormat.image_channel_data_type = CL_UNSIGNED_INT8;
            imageFormat.image_channel_order = CL_RGBA;
			pixelArray = NULL;
        }

        ~SimpleImage()
        {
        }

		/*
		* Just a test function to dump things in for a while
		*/
		int dump();

        /**
        * Allocate image memory and Load bitmap file
        * @return SDK_SUCCESS on success and SDK_FAILURE on failure
        */
        int setupSimpleImage();

		/**
		*Create opencl buffer
		* - allocate & initialize
		*/
		int createBuffer(cl_mem &bufferName, pixelStruct *arrayName);//(cl_mem bufferName, pixelStruct *arrayName);


		/**
		*Allocate array memory
		* @return SDK_SUCCESS on success and SDK_FAILURE on failure
        */
		int setupBuffers();

        /**
         * Override from SDKSample, Generate binary image of given kernel
         * and exit application
         */
        int genBinaryImage();

        /**
        * OpenCL related initialisations.
        * Set up Context, Device list, Command Queue, Memory buffers
        * Build CL kernel program executable
        * @return SDK_SUCCESS on success and SDK_FAILURE on failure
        */
        int setupCL();

		/**
		* Check if we get out of resources
		*/
		int checkResources();

        /**
        * Set values for kernels' arguments, enqueue calls to the kernels
        * on to the command queue, wait till end of kernel execution.
        * Get kernel start and end time if timing is enabled
        * @return  SDK_SUCCESS on success and SDK_FAILURE on failure
        */
        int runCLKernels();

        /**
        * Reference CPU implementation of Binomial Option
        * for performance comparison
        */
        void simpleImageCPUReference();

        /**
        * Override from SDKSample. Print sample stats.
        */
        void printStats();

        /**
        * Override from SDKSample. Initialize
        * command line parser, add custom options
        */
        int initialize();

        /**
        * Override from SDKSample, adjust width and height
        * of execution domain, perform all sample setup
        * @return  SDK_SUCCESS on success and SDK_FAILURE on failure
        */
        int setup();

        /**
        * Override from SDKSample
        * Run OpenCL SimpleImage
        * @return  SDK_SUCCESS on success and SDK_FAILURE on failure
        */
        int run();

        /**
        * Override from SDKSample
        * Cleanup memory allocations
        * @return  SDK_SUCCESS on success and SDK_FAILURE on failure
        */
        int cleanup();

        /**
        * Override from SDKSample
        * Verify against reference implementation
        * @return  SDK_SUCCESS on success and SDK_FAILURE on failure
        */
        int verifyResults();
};

#endif // SIMPLE_IMAGE_H_
