#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

/* To Include OpenCV:

Define environment path %OPENCV_DIR%

Project->properties->Configuration Properties->C/C++->General->Additional Include Directories:
add value	$(OPENCV_DIR)\..\..\include

Project->properties->Configuration Properties->Linker->General->Additional Library Directories:
add value	$(OPENCV_DIR)\lib

Project->properties->Configuration Properties->Linker->Input->Additional Dependencies^
add value	opencv_world310d.lib;
*/
#include <opencv2/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>

using namespace cv;
using namespace std;

class ErrorClass {
	int errorCode;
	string msg;
public:
	ErrorClass(int _errorCode, string _msg);
};
class ORGBImage
{
private:
	
	const static double coersionMatrix[3][3];
	const static double reverseCoersionMatrix[3][3];
	
	//source image
	Mat image;

    /* oRGB space container
	*  [image row][image column][channel index: 0,1,2]
	*  remark: double could be replaced by float for efficiency
	*/
	double*** oRGB;
protected:
	double lumaScaleFactor = 1.0;
	double blueYellowScaleFactor = 1.0;
	double greenRedScaleFactor = 1.0;

	double lumaShiftingFactor = 0.0;
	double blueYellowShiftingFactor = 0.0;
	double greenRedShiftingFactor = 0.0;

	void DrawORGBImageWithLinearTransformation(Mat&,
		double, double, double,
		double, double, double);
	double AngleTransform(double);
	double AngleReverseTransform(double);
public:
	ORGBImage();
	ORGBImage(Mat);
	ORGBImage(const ORGBImage&);
	ORGBImage& operator=(const ORGBImage&);
	Mat& GetOriginImage();
	Mat GetImageFromORGB();
	Mat GetTestImage(double factor);

	void SetLumaScaleFactor(double);
	void SetBlueYellowScaleFactor(double);
	void SetGreenRedScaleFactor(double);

	void SetLumaShiftingFactor(double);
	void SetBlueYellowShiftingFactor(double);
	void SetGreenRedShiftingFactor(double);

	~ORGBImage();
};