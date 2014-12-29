/**********************************************************************
Copyright ©2013 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

•	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
•	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/
#pragma OPENCL EXTENSION cl_amd_printf : enable

typedef struct __attribute__((packed)) pixelStruct{
    uint pxlValue;
    float mo;
    int trsfrm;
    uint row;
    uint col;
	int indx;
} pixelStruct;

__constant sampler_t imageSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST; 

/* According to the input image create 3 color buffers */
__kernel void createColorBuffers(__global pixelStruct* red, __global pixelStruct* green, __global pixelStruct* blue, 
                                __read_only image2d_t input)
{
	//get image coordinates
	int2 coord = (int2)(get_global_id(0), get_global_id(1));
	
	//get image dimensions
	int2 dim = get_image_dim(input);
	
	//calculate buffer index
	int index = (coord.y)*(dim.x) + coord.x; 
	
	//fill struct
	uint4 pxlValue = read_imageui(input, imageSampler, coord);
	red[index].pxlValue = pxlValue.x;
	green[index].pxlValue = pxlValue.y;
	blue[index].pxlValue = pxlValue.z;


	int2 coord1 = (int2)(coord.x + 1, coord.y);
	int2 coord2 = (int2)(coord.x - 1, coord.y);
	int2 coord3 = (int2)(coord.x, coord.y + 1);
	int2 coord4 = (int2)(coord.x, coord.y - 1);
	float4 temp1 = convert_float4(read_imageui(input, imageSampler, coord1));
	float4 temp2 = convert_float4(read_imageui(input, imageSampler, coord2));
	float4 temp3 = convert_float4(read_imageui(input, imageSampler, coord3));
	float4 temp4 = convert_float4(read_imageui(input, imageSampler, coord4));
	float4 mo = (float4)(temp1 + temp2 + temp3 + temp4) / 4.0;
	red[index].mo = mo.x;
	green[index].mo = mo.y;
	blue[index].mo = mo.z;

	red[index].trsfrm = 0;
	green[index].trsfrm = 0;
	blue[index].trsfrm = 0;

	red[index].indx = index;
	red[index].row = coord.y;
	red[index].col = coord.x;
	green[index].indx = index;
	green[index].row = coord.y;
	green[index].col = coord.x;
	blue[index].indx = index;
	blue[index].row = coord.y;
	blue[index].col = coord.x;

	/*uint4 temp = read_imageui(input, imageSampler, coord);
	write_imageui(output, coord, temp);*/
}

/*create a pixel buffer according to color buffers)*/
__kernel void createPixelArray(__global pixelStruct* redIn, __global pixelStruct* greenIn, __global pixelStruct* blueIn, 
                               __global pixelStruct* redOut, __global pixelStruct* greenOut, __global pixelStruct* blueOut, 
							   uint width)
{
	//get image coordinates
	int2 coord = (int2)(get_global_id(0), get_global_id(1));

	//calculate buffer index
	int index = (coord.y)*width + coord.x; 

	int indexRed = redIn[index].indx;
	int indexGreen = greenIn[index].indx;
	int indexBlue = blueIn[index].indx;

	redOut[indexRed] = redIn[index];
	greenOut[indexGreen] = greenIn[index];
	blueOut[indexBlue] = blueIn[index];

}

/*Match image histograms*/
__kernel void histMatching(__global pixelStruct* red, __global pixelStruct* green, __global pixelStruct* blue, 
                           __global pixelStruct* refRed, __global pixelStruct* refGreen, __global pixelStruct* refBlue, 
						   uint width)
{
	//get image coordinates
	int2 coord = (int2)(get_global_id(0), get_global_id(1));

	//calculate buffer index
	int index = (coord.y)*width + coord.x; 

	/*referee image*/
	//red
	int x = red[index].pxlValue - refRed[index].trsfrm;
	if ((x > 0) && (x < 256)) red[index].pxlValue = x;
    else if (x <= 0) red[index].pxlValue = 1;
    else red[index].pxlValue = 255;

	//green
	x = green[index].pxlValue - refGreen[index].trsfrm;
	if ((x > 0) && (x < 256)) green[index].pxlValue = x;
    else if (x <= 0) green[index].pxlValue = 1;
    else green[index].pxlValue = 255;

	//blue
	x = blue[index].pxlValue - refBlue[index].trsfrm;
	if ((x > 0) && (x < 256)) blue[index].pxlValue = x;
    else if (x <= 0) blue[index].pxlValue = 1;
    else blue[index].pxlValue = 255;

	/*reference image*/
	refRed[index].pxlValue -= refRed[index].trsfrm;
	refGreen[index].pxlValue -= refGreen[index].trsfrm;
	refBlue[index].pxlValue -= refBlue[index].trsfrm;
}

/*create 2D output image according to color buffers)*/
__kernel void createOutputImage(__global pixelStruct* redOut, __global pixelStruct* greenOut, __global pixelStruct* blueOut, 
                                __write_only image2d_t output)
{
	//get image coordinates
	int2 coord = (int2)(get_global_id(0), get_global_id(1));

	//get image dimensions
	int2 dim = get_image_dim(output);
	
	//calculate buffer index
	int index = (coord.y)*(dim.x) + coord.x; 
	
	uint4 pixel;
	pixel.x = redOut[index].pxlValue;
	pixel.y = greenOut[index].pxlValue;
	pixel.z = blueOut[index].pxlValue;
	pixel.w = 255;

	write_imageui(output, coord, pixel);
}



