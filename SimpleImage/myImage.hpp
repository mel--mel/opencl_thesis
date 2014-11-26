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
	cl_uchar4* imageData;                 /**< Input bitmap data to device */
	cl_image_desc imageDesc;            /**< Parameter needed for clGreateImage*/


};

#endif // MY_IMAGE_H_