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
} pixelStruct;

__constant sampler_t imageSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST; 

/* Copy input 2D image to output 2D image */
__kernel void createColorArrays(__read_only image2d_t input, __write_only image2d_t output, __global pixelStruct* red, 
                         __global pixelStruct* green, __global pixelStruct* blue)
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
	float4 mo = (float4)(temp1 + temp2 + temp3 + temp4) / 4;
	red[index].mo = mo.x;
	green[index].mo = mo.y;
	blue[index].mo = mo.z;

	red[index].trsfrm = 0;
	green[index].trsfrm = 0;
	blue[index].trsfrm = 0;

	red[index].row = coord.y;
	red[index].col = coord.x;
	green[index].row = coord.y;
	green[index].col = coord.x;
	blue[index].row = coord.y;
	blue[index].col = coord.x;

	/*uint4 temp = read_imageui(input, imageSampler, coord);
	write_imageui(output, coord, temp);*/
}

/*create 2D output image according to color buffers)*/
__kernel void createOutputImage(__write_only image2d_t output, __global pixelStruct* red, 
                         __global pixelStruct* green, __global pixelStruct* blue)
{
	//get image coordinates
	int2 coord = (int2)(get_global_id(0), get_global_id(1));

	//get image dimensions
	int2 dim = get_image_dim(output);
	
	//calculate buffer index
	int index = (coord.y)*(dim.x) + coord.x; 

	uint4 temp = (uint4)(red[index].pxlValue, green[index].pxlValue, blue[index].pxlValue, 255);
	
	write_imageui(output, coord, temp);
}

/* Copy input 3D image to 2D image */
__kernel void image3dCopy(__read_only image3d_t input, __write_only image2d_t output)
{
	int2 coord = (int2)(get_global_id(0), get_global_id(1));

	/* Read first slice into lower half */
	uint4 temp0 = read_imageui(input, imageSampler, (int4)(coord, 0, 0));

	/* Read second slice into upper half */
	uint4 temp1 = read_imageui(input, imageSampler, (int4)((int2)(get_global_id(0), get_global_id(1) - get_global_size(1)/2), 1, 0));
	
	write_imageui(output, coord, temp0 + temp1);
}

