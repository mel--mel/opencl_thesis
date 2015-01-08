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


#include "SimpleImage.hpp"
#include "myImage.hpp"
#include "HelpingFunctions.h"
#include "Templates.cpp"
#include <cmath>
#include <vector>
#include "opencv2\ocl\ocl.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv/cv.h"
#include "opencv2/core/core.hpp"


#define OUTPUT_IMAGE "L_Out.bmp"

void giveMelOpenCL::setupCL()
{
    cl_int status = CL_SUCCESS;
    cl_device_type dType;

	//find device type
    if(sampleArgs->deviceType.compare("cpu") == 0) {
		dType = CL_DEVICE_TYPE_CPU;
		std::cout << "Device type = CPU" << std::endl;} //deviceType = "cpu"
    else{  
        dType = CL_DEVICE_TYPE_GPU; //deviceType = "gpu"
        if(sampleArgs->isThereGPU() == false) {
            std::cout << "GPU not found. Falling back to CPU device" << std::endl;
            dType = CL_DEVICE_TYPE_CPU;
			std::cout << "Device type = CPU" << std::endl; //deviceType = "cpu"
        }
		std::cout << "Device type = GPU" << std::endl;
    }

    /* Have a look at the available platforms and pick either
    * the AMD one if available or a reasonable default.*/
    cl_platform_id platform = NULL;
    int retValue = getPlatform(platform, sampleArgs->platformId, sampleArgs->isPlatformEnabled());
    if (retValue != CL_SUCCESS) throw "getPlatform() failed";

    // Display available devices.
    retValue = displayDevices(platform, dType);
	if (retValue != CL_SUCCESS) throw "displayDevices() failed";

    // If we could find our platform, use it. Otherwise use just available platform.
    cl_context_properties cps[3] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0};

	//create context analoga me deviceType kai platform
    context = clCreateContextFromType(cps, dType, NULL, NULL, &status);
    if (status != CL_SUCCESS) throw "clCreateContextFromType failed.";

    //get device on which to run the sample
	status = getDevices(context, &devices, sampleArgs->deviceId, sampleArgs->isDeviceIdEnabled());
    if (status != CL_SUCCESS) throw "getDevices() failed";

    status = deviceInfo.setDeviceInfo(devices[sampleArgs->deviceId]);
    if (status != CL_SUCCESS) throw "deviceInfo.setDeviceInfo failed";

	//Check whether device doesn t support images
    if(!deviceInfo.imageSupport) throw "Expected Error: Device does not support Images";

	//Create command queue
    commandQueue = clCreateCommandQueue(context, devices[sampleArgs->deviceId], 0, &status);
    if (status != CL_SUCCESS) throw "clCreateCommandQueue failed.";
}

void giveMelOpenCL::compileKernels(std::string kernelsFileName)
{
	//create a CL program using the kernel source
	buildProgramData buildData;
	buildData.kernelName = kernelsFileName;
    buildData.devices = devices;
    buildData.deviceId = sampleArgs->deviceId;
    buildData.flagsStr = std::string("");
    if(sampleArgs->isLoadBinaryEnabled()) buildData.binaryName = std::string(sampleArgs->loadBinary.c_str());
    if(sampleArgs->isComplierFlagsSpecified()) buildData.flagsFileName = std::string(sampleArgs->flags.c_str());
    int retValue = buildOpenCLProgram(program, context, buildData);
    if (retValue != CL_SUCCESS) throw "buildOpenCLProgram() failed";
}

void giveMelOpenCL::runThisKernel(const char* kernelNameInFile, size_t *globalThreads, 
							   cl_mem &buffer1, cl_mem &buffer2, cl_mem &buffer3,
							   cl_mem &buffer4, cl_mem &buffer5, cl_mem &buffer6,
							   cl_uint &parameter)
{
	int status = 0;

	cl_kernel kernelName;

	//create kernel
	kernelName = clCreateKernel(program, kernelNameInFile, &status);// "createColorArrays", &status);
    if (status != CL_SUCCESS) throw "clCreateKernel failed.(colorArraysKernel)";

	//push arguments
	pushArguments(kernelName, &buffer1, &buffer2, &buffer3, &buffer4, &buffer5, &buffer6, &parameter);

	//run kernel
	status = clEnqueueNDRangeKernel(commandQueue, kernelName, 2, NULL, globalThreads, NULL, 0, NULL, 0);
    if (status != CL_SUCCESS) throw "clEnqueueNDRangeKernel failed.";

}

void giveMelOpenCL::runThisKernel(const char* kernelNameInFile, size_t *globalThreads, 
							   cl_mem &buffer1, cl_mem &buffer2, cl_mem &buffer3,
							   cl_mem &imageName)
{
	int status = CL_SUCCESS;
	cl_kernel kernelName;

	//create kernel
	kernelName = clCreateKernel(program, kernelNameInFile, &status);// "createColorArrays", &status);
    if (status != CL_SUCCESS) throw "clCreateKernel failed.(colorArraysKernel)";

	//push arguments
	pushArguments(kernelName, &buffer1, &buffer2, &buffer3, &imageName);

	//run kernel
	status = clEnqueueNDRangeKernel(commandQueue, kernelName, 2, NULL, globalThreads, NULL, 0, NULL, 0);
    if (status != CL_SUCCESS) throw "clEnqueueNDRangeKernel failed.";
}

int giveMelOpenCL::setTimer()
{
	// create and initialize timers
    int timer = sampleTimer->createTimer();
    sampleTimer->resetTimer(timer);
    sampleTimer->startTimer(timer);

	return timer;
}

void giveMelOpenCL::stopTimer(int timer)
{
	sampleTimer->stopTimer(timer);
    cl_double time = (double)(sampleTimer->readTimer(timer));

	std::cout << "Total time: " << time << " sec." << std::endl << std:: endl;
}

int giveMelOpenCL::cleanup()
{
    if(!byteRWSupport)
    {
        return SDK_SUCCESS;
    }

    if (clReleaseProgram(program) != CL_SUCCESS) throw "clReleaseProgram failed.(program)";
    if (clReleaseCommandQueue(commandQueue) != CL_SUCCESS) throw "clReleaseCommandQueue failed.(commandQueue)";
    if (clReleaseContext(context) != CL_SUCCESS) throw "clReleaseContext failed.(context)";
    FREE(devices);

    return SDK_SUCCESS;
}

/*void SimpleImage::printStats()
{
	if(sampleArgs->timing)
    {
        std::string strArray[4] =
        {
            "Width",
            "Height",
            "Time(sec)",
            "kernelTime(sec)"
        };
        std::string stats[4];

        sampleTimer->totalTime = setupTime + kernelTime;

        stats[0] = toString(width, std::dec);
        stats[1] = toString(height, std::dec);
        stats[2] = toString(sampleTimer->totalTime, std::dec);
        stats[3] = toString(kernelTime, std::dec);

        printStatistics(strArray, stats, 4);
    }
}*/

void oclTest()
{
	cv::initModule_nonfree();

	cv::Mat L, R, leftImageGrey, rightImageGrey;

	L = cv::imread("myOutL.bmp");
	R = cv::imread("myOutR.bmp");
	
	cv::cvtColor(L, leftImageGrey, CV_RGB2GRAY);
	cv::cvtColor(R, rightImageGrey, CV_RGB2GRAY);

	cv::ocl::oclMat clL, edgesL, clR, edgesR;

	clL.upload(leftImageGrey);
	clR.upload(rightImageGrey);

	cv::ocl::Canny(clL, edgesL, 10, 150, 3, false);
	cv::ocl::Canny(clR, edgesR, 10, 150, 3, false);

	std::vector<cv::DMatch> matches;
	cv::ocl::BruteForceMatcher_OCL_base matcher;
	matcher.match(edgesL, edgesR, matches);

	for (size_t i = 0; i < int(matches.size()); ++i){
		cv::Point from = matches[i].queryIdx;
		cv::Point to = matches[i].trainIdx;

		//calculate local distance for each possible match
		double dist = sqrt((from.x - to.x) * (from.x - to.x) + (from.y - to.y) * (from.y - to.y));
		//double dist = abs(from.x-to.x);

		cv::circle(L, from, 1, cv::Scalar(255-(dist*8), 0, dist*8), -1);}


	//cv::Rect roi(30, 0,  L.size().width);
	cv::Rect roi(33, 0, L.size().width-35, L.size().height);
	cv::Mat croppedDepthMap = L(roi);

	cv::imshow("depthmap", croppedDepthMap);
    cv::waitKey(0);

	/*edgesL.download(leftImageGrey);
	edgesR.download(rightImageGrey);

	cv::imwrite("edgesL.bmp", leftImageGrey);
	cv::imwrite("edgesR.bmp", rightImageGrey);

	cv::imshow("left bmp", leftImageGrey);
	cv::imshow("right bmp", rightImageGrey);

	cv::waitKey(0);
	*/
}

void createDepthMap()
{
	cv::initModule_nonfree();

	cv::Mat L, R, leftImageGrey, rightImageGrey;

	L = cv::imread("myOutL.bmp");
	R = cv::imread("myOutR.bmp");
	
	cv::cvtColor(L, leftImageGrey, CV_RGB2GRAY);
	cv::cvtColor(R, rightImageGrey, CV_RGB2GRAY);

	std::vector<cv::KeyPoint> keypoints1, keypoints2; 
	cv::Mat descriptors1, descriptors2;
	const cv::Mat mask;

	//anixneyw ta features (FAST)
	cv::Ptr<cv::FeatureDetector> detector;
	detector = new cv::DynamicAdaptedFeatureDetector ( new cv::FastAdjuster(10,true), 5000, 10000, 10);
	detector->detect(leftImageGrey, keypoints1);
	detector->detect(rightImageGrey, keypoints2);

	//briskw descriptors gia ta features (SIFT)
	cv::Ptr<cv::DescriptorExtractor> extractor;
	extractor = cv::DescriptorExtractor::create("SIFT");
	extractor->compute( leftImageGrey, keypoints1, descriptors1);
	extractor->compute( rightImageGrey, keypoints2, descriptors2);

	//antistoixw toyw descriptors twn features
	/*std::vector <std::vector <cv::DMatch>> matches;
	cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create("BruteForce");
    matcher->knnMatch(descriptors_1, descriptors_2, matches, 500);*/

	cv::ocl::oclMat oclDescriptors1, oclDescriptors2;

	oclDescriptors1.upload(descriptors1);
	oclDescriptors2.upload(descriptors2);

	//std::vector<cv::DMatch> matches;
	std::vector <std::vector <cv::DMatch>> matches;
	cv::ocl::BruteForceMatcher_OCL_base matcher;
	matcher.knnMatch(oclDescriptors1, oclDescriptors2, matches, 500);

	//look whether the match is inside a defined area of the image
	//only 25% of maximum of possible distance
	double tresholdDist = 0.25 * sqrt(double(leftImageGrey.size().height*leftImageGrey.size().height + 
											leftImageGrey.size().width*leftImageGrey.size().width));
	std::vector <cv::DMatch> good_matches2;
	good_matches2.reserve(matches.size());
	for (size_t i = 0; i < int(matches.size()); ++i){
		for (int j = 0; j < int(matches[i].size()); j++){
			cv::Point2f from = keypoints1[matches[i][j].queryIdx].pt;
			cv::Point2f to = keypoints2[matches[i][j].trainIdx].pt;

	        //calculate local distance for each possible match
		    double dist = sqrt((from.x - to.x) * (from.x - to.x) + (from.y - to.y) * (from.y - to.y));
	
	        //save as best match if local distance is in specified area and on same height
	        if (dist < tresholdDist && abs(from.y-to.y)<8 && abs(from.x-to.x)<30){
	            good_matches2.push_back(matches[i][j]);
	            j = matches[i].size();}
		}}

	//combine2images
	cv::Size sz1 = L.size();
    cv::Size sz2 = R.size();
	cv::Mat showMatches(sz1.height, sz1.width+sz2.width, L.type());
    cv::Mat leftHalf(showMatches, cv::Rect(0, 0, sz1.width, sz1.height));
    L.copyTo(leftHalf);
    cv::Mat rightHalf(showMatches, cv::Rect(sz1.width, 0, sz2.width, sz2.height));
    R.copyTo(rightHalf);

	cv::Point2f rightPoint;
	//for (size_t i = 0; i < int(good_matches2.size()); ++i){
	for (size_t i = 0; i < int(good_matches2.size()); i+=15){
		rightPoint = keypoints2[good_matches2[i].trainIdx].pt;
		rightPoint.x += sz1.width;
		cv::circle(showMatches, keypoints1[good_matches2[i].queryIdx].pt, 2, cv::Scalar(255, 0, 0));
		cv::circle(showMatches, rightPoint, 2, cv::Scalar(0, 0, 255));
		cv::line(showMatches, keypoints1[good_matches2[i].queryIdx].pt,rightPoint, cv::Scalar((i*20)%255, (i*50)%255, (i*130)%255));}

    cv::imshow("matches", showMatches);
    cv::waitKey(0);

	cv::imwrite("matches.bmp", showMatches);

	for (size_t i = 0; i < int(good_matches2.size()); ++i){
		cv::Point2f from = keypoints1[good_matches2[i].queryIdx].pt;
		cv::Point2f to = keypoints2[good_matches2[i].trainIdx].pt;

		//calculate local distance for each possible match
		double dist = sqrt((from.x - to.x) * (from.x - to.x) + (from.y - to.y) * (from.y - to.y));
		//double dist = abs(from.x-to.x);

		cv::circle(L, keypoints1[good_matches2[i].queryIdx].pt, 1, cv::Scalar(255-(dist*8), 0, dist*8), -1);}


	//cv::Rect roi(30, 0,  L.size().width);
	cv::Rect roi(33, 0, L.size().width-35, L.size().height);
	cv::Mat croppedDepthMap = L(roi);

	cv::imshow("depthmap", croppedDepthMap);
    cv::waitKey(0);

	cv::imwrite("myDepthmap.bmp", croppedDepthMap);
}

void matchImageColors()
{
	giveMelOpenCL clProvider;
	MyImage imageL; 
	MyImage imageR;
	int timer;
	
	imageL.open("diplo000000-L.bmp");//imageL.open("cemeteryL.bmp");//
	imageR.open("diplo000000-R.bmp");//imageR.open("cemeteryR.bmp");//
	
	timer = clProvider.setTimer();

	clProvider.setupCL();
	clProvider.compileKernels("SimpleImage_Kernels.cl");

	imageL.imageToColorBuffers(&clProvider);
	imageR.imageToColorBuffers(&clProvider);

	imageL.histogramMatching(&clProvider, &imageR);

	imageL.buffersToOutputImage(&clProvider);
	imageR.buffersToOutputImage(&clProvider);

	clProvider.stopTimer(timer);

	imageL.save("myOutL.bmp");//("equalizedL.bmp");//
	imageR.save("myOutR.bmp");//("equalizedR.bmp");//

}

int main(int argc, char * argv[])
{
    try
	{
	    matchImageColors();
		createDepthMap();	
		//oclTest();
	}

	catch(char* expn){
		std::cout << "EXITING ERROR: " << expn << std::endl << std::endl;
	}

    return SDK_SUCCESS;
}
