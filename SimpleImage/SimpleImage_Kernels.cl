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

__constant sampler_t imageSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST; 

/* Copy input 2D image to output 2D image */
__kernel void image2dCopy(__read_only image2d_t input, __write_only image2d_t output)
{
	int2 coord = (int2)(get_global_id(0), get_global_id(1));

	int2 coord1 = (int2)(coord.x + 1, coord.y);
	int2 coord2 = (int2)(coord.x - 1, coord.y);
	int2 coord3 = (int2)(coord.x, coord.y + 1);
	int2 coord4 = (int2)(coord.x, coord.y - 1);
	uint4 temp1 = read_imageui(input, imageSampler, coord1);
	uint4 temp2 = read_imageui(input, imageSampler, coord2);
	uint4 temp3 = read_imageui(input, imageSampler, coord3);
	uint4 temp4 = read_imageui(input, imageSampler, coord4);
	uint4 temp = (temp1 + temp2 + temp3 + temp4) / 4;
	
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

