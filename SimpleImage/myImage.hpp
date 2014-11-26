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
	SDKBitMap imageBitmap;   /**< Bitmap class object */
	uchar4* pixelData;       /**< Pointer to image data */
    cl_uint width;                      /**< Width of image */
    cl_uint height;                     /**< Height of image */

	cl_uchar4* imageData;               /**< Input bitmap data to device */
	cl_image_desc imageDesc;            /**< Parameter needed for clGreateImage*/
	

public:

	int open(std::string imageName);  /*Open input image*/

};

#endif // MY_IMAGE_H_