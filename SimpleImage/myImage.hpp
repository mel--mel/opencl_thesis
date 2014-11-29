#ifndef MY_IMAGE_H_
#define MY_IMAGE_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "CLUtil.hpp"
#include "SDKBitMap.hpp"

using namespace appsdk;

class MyImage
{
	//variables initialized in "open"
	SDKBitMap imageBitmap;              /**< Bitmap class object */
	uchar4* pixelData;                  /**< Pointer to image data */
	cl_uchar4* imageDataIn;             /**< Input bitmap data to device */
	cl_uchar4* imageDataOut;            /**< Output bitmap data to device */

    cl_uint width;                      /**< Width of image */
    cl_uint height;                     /**< Height of image */
	
	cl_image_desc imageDesc;            /**< Parameter needed for clGreateImage*/
	cl_image_format imageFormat;        /**< Image format descriptor */

	//variables initialized in ____________________
	cl_mem imageIn;
	cl_mem imageOut;

	pixelStruct* redArray; /**< 1D Struct to help equalization */
	pixelStruct* greenArray; /**< 1D Struct to help equalization */
	pixelStruct* blueArray; /**< 1D Struct to help equalization */

public:

	cl_mem redBuffer;                   /**< CL image buffer for pixelStructArray*/
	cl_mem greenBuffer;                 /**< CL image buffer for pixelStructArray*/
	cl_mem blueBuffer;                  /**< CL image buffer for pixelStructArray*/

	void open(std::string imageName);  /*Open (load) image*/

	void save(std::string imageName);  /*Save image*/

	void imageToColorBuffers(giveMelOpenCL *clProvider);

	void buffersToOutputImage(giveMelOpenCL *clProvider);

	void histEqualizeColors(giveMelOpenCL *clProvider);

	void matchHistograms(giveMelOpenCL *clProvider, MyImage *imageRef);

	void histogramMatching(giveMelOpenCL *clProvider, MyImage *imageRef);

	void histogramEqualization(giveMelOpenCL *clProvider);

	void putPixelsInRightPos(giveMelOpenCL *clProvider);

	void cleanup();

	MyImage()   /*Constructor*/
		: imageDataIn(NULL),
		  imageDataOut(NULL)
	{
		 imageFormat.image_channel_data_type = CL_UNSIGNED_INT8;
         imageFormat.image_channel_order = CL_RGBA;
		 redArray = NULL;
	     greenArray = NULL;
	     blueArray = NULL;
	}

	~MyImage()
	{
		cleanup();
	}
};

#endif // MY_IMAGE_H_