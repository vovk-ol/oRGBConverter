#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

/* To Include OpenCV:

Define environment path OPENCV_DIR

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
	Mat& image;

    /* oRGB space container
	*  [image row][image column][channel index: 0,1,2]
	*  remark: double could be replaced by float for efficiency
	*/
	double*** oRGB;

	/* container of angles, it is neccessary to get LCC from oRGB
	*  [image row][image column]
	*  remark: double could be replaced by float for efficiency
	*/
	double** oRGBTheta;
protected:
	void ScaleORGBChannel(double, uint);
public:
	ORGBImage(Mat&);
	Mat& GetOriginImage();
	Mat GetImageFromORGB();
	void ScaleBlueYellowChannel(double);
	void ScaleGreenRedChannel(double);
};