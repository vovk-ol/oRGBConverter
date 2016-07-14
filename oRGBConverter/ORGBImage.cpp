#include "oRGBImage.h"

ErrorClass::ErrorClass(int _errorCode, string _msg) :errorCode(_errorCode), msg(_msg) {}

const double ORGBImage::coersionMatrix[3][3] = { { 0.2990, 0.5870, 0.1140 },
												 { 0.5000, 0.5000,-1.000 },
												 { 0.8660, -0.8660, 0.0000 } };

const double ORGBImage::reverseCoersionMatrix[3][3] = { { 1.0000, 0.1140, 0.7436 },
														{ 1.0000, 0.1140, -0.4111 },
														{ 1.0000, -0.8660, 0.1663 } };

//******************************************************************************
// Convert an sRGB color channel to a linear sRGB color channel.
//******************************************************************************
//float GammaExpand_sRGB(float nonlinear)
//{
//	return nonlinear;
//	return   (nonlinear <= 0.04045f)
//		? (nonlinear / 12.92f)
//		: (powf((nonlinear + 0.055f) / 1.055f, 2.4f));
//}

//******************************************************************************
// Convert a linear sRGB color channel to a sRGB color channel.
//******************************************************************************
//float GammaCompress_sRGB(float linear)
//{
//	return linear;
//	return   (linear <= 0.0031308f)
//		? (12.92f * linear)
//		: (1.055f * powf(linear, 1.0f / 2.4f) - 0.055f);
//}

ORGBImage::ORGBImage(Mat& _image) : image(_image) {

#pragma region check input image

	if (image.empty())
		throw ErrorClass(1, "invalid image: image.empty() == true");
	if (image.rows == 0)
		throw ErrorClass(2, "invalid image: mage.rows == 0");
	if (image.cols == 0)
		throw ErrorClass(3, "invalid image: mage.cols == 0");

#pragma endregion

	//static array for scaled intensities
	double scaledOntersity[3];

	this->oRGB = new double**[image.rows];

	for (int i = 0; i < image.rows; i++)
	{
		oRGB[i] = new double*[image.cols];
		for (int j = 0; j < image.cols; j++)
		{
			oRGB[i][j] = new double[3];
			Vec3b  intensity = image.at<Vec3b>(i, j);//get pixel intensity

			for (int ch_i = 0; ch_i < 3; ch_i++)
			{
				//this scaling to range [0,1] isn't neccesary for image processing,
				//but it's needed to get origin oRGB space
				scaledOntersity[ch_i] = intensity[2 - ch_i] / 255.0;
			}

			//get LCC space
			for (int r = 0; r < 3; r++) {
				oRGB[i][j][r] = 0;
				for (int c = 0; c < 3; c++)
				{
					oRGB[i][j][r] += coersionMatrix[r][c] * scaledOntersity[c];
				}
			}

			//rotating
			double theta = atan2(oRGB[i][j][2], oRGB[i][j][1]);
			double oRGBTheta = 0;
			if (theta > 0) {
				if (theta < M_PI / 3.0)
				{
					oRGBTheta = 1.5*theta;
				}
				else
				{
					if (theta <= M_PI)
					{
						oRGBTheta = M_PI_2 + 0.75*(theta - M_PI / 3.0);
					}
					else
					{
						throw ErrorClass(10, "unexpected angle");
					}
				}
			}
			else
			{
				if (theta > -M_PI / 3.0)
				{
					oRGBTheta = 1.5*theta;
				}
				else
				{
					if (theta >= -M_PI)
					{
						oRGBTheta = -(M_PI_2 + 0.75*(-theta - M_PI / 3.0));
					}
					else
					{
						throw ErrorClass(10, "unexpected angle");
					}
				}
			}
			//remark: if angle is less then some small parametr e. g. 1e-9, 
			//we could do no rotating for efficiency
			double angle = oRGBTheta - theta;

			//create rotation matrix
			double R[2][2] = { { cos(angle), -sin(angle) } ,
							   { sin(angle),  cos(angle) } };

			double CybCrgVector[2];

			//multiply by Rotation matrix R
			for (int r = 0; r < 2; r++) {
				CybCrgVector[r] = 0;
				for (int c = 0; c < 2; c++)
				{
					CybCrgVector[r] += R[r][c] * oRGB[i][j][1 + c];
				}
			}

			//renew yellow-blue and red-green channels in oRGB[i][j] pixel
			for (int r = 0; r < 2; r++)
			{
				oRGB[i][j][1 + r] = CybCrgVector[r];
			}
		}
	}
}
void ORGBImage::SetLumaScaleFactor(double lumaScaleFactor) {
	this->lumaScaleFactor = lumaScaleFactor;
}
void  ORGBImage::SetBlueYellowScaleFactor(double blueYellowScaleFactor) {
	this->blueYellowScaleFactor = blueYellowScaleFactor;
}
void  ORGBImage::SetGreenRedScaleFactor(double greenRedScaleFactor) {
	this->greenRedScaleFactor = greenRedScaleFactor;
}

void ORGBImage::SetLumaShiftingFactor(double lumaShiftingFactor) {
	this->lumaShiftingFactor = lumaShiftingFactor;
}
void  ORGBImage::SetBlueYellowShiftingFactor(double blueYellowShiftingFactor) {
	this->blueYellowShiftingFactor = blueYellowShiftingFactor;
}
void  ORGBImage::SetGreenRedShiftingFactor(double greenRedShiftingFactor) {
	this->greenRedShiftingFactor = greenRedShiftingFactor;
}

Mat& ORGBImage::GetOriginImage() { return image; }

Mat ORGBImage::GetImageFromORGB() {
	if (lumaScaleFactor == 1.0
		&&blueYellowScaleFactor == 1.0
		&&greenRedScaleFactor == 1.0
		&& lumaShiftingFactor == 0.0
		&& blueYellowShiftingFactor == 0.0
		&& greenRedShiftingFactor == 0.0)
	{
		return this->image;
	}

	Mat resImage(image.rows, image.cols, image.type());
	DrawORGBImageWithLinearTransformation(resImage,
		this->lumaScaleFactor, this->blueYellowScaleFactor, this->greenRedScaleFactor,
		this->lumaShiftingFactor, this->blueYellowShiftingFactor, this->greenRedShiftingFactor);
	return resImage;
}
void ORGBImage::DrawORGBImageWithLinearTransformation(Mat& resImage,
	double lumaScaleFactor, double blueYellowScaleFactor, double greenRedScaleFactor,
	double lumaShiftingFactor, double blueYellowShiftingFactor, double greenRedShiftingFactor) {
	for (int i = 0; i < image.rows; i++)
	{
		for (int j = 0; j < image.cols; j++)
		{
			double LCbyCgrScaledVector[3];
			LCbyCgrScaledVector[0] = lumaScaleFactor*oRGB[i][j][0] + lumaShiftingFactor;
			LCbyCgrScaledVector[1] = blueYellowScaleFactor*oRGB[i][j][1] + blueYellowShiftingFactor;
			LCbyCgrScaledVector[2] = greenRedScaleFactor*oRGB[i][j][2] + greenRedShiftingFactor;

#pragma region undo rotation
			double oRGBTheta_2 = atan2(LCbyCgrScaledVector[2], LCbyCgrScaledVector[1]);
			double theta;
			if (oRGBTheta_2 > 0) {
				if (oRGBTheta_2 < M_PI_2)
				{
					theta = (2.0*oRGBTheta_2) / 3.0;
				}
				else
				{
					if (oRGBTheta_2 <= M_PI)
					{
						theta = M_PI / 3.0 + (4.0 / 3.0)*(oRGBTheta_2 - M_PI_2);
					}
					else
					{
						throw ErrorClass(11, "unexpected angle");
					}

				}
			}
			else
			{
				if (oRGBTheta_2 > -M_PI_2)
				{
					theta = (2.0*oRGBTheta_2) / 3.0;
				}
				else
				{
					if (oRGBTheta_2 >= -M_PI)
					{
						theta = -(M_PI / 3.0 + (4.0 / 3.0)*(-oRGBTheta_2 - M_PI_2));
					}
					else
					{
						throw ErrorClass(11, "unexpected angle");
					}

				}
			}

			double angle = (oRGBTheta_2 - theta);
			double inverseR[2][2] = { { cos(angle), sin(angle) } ,
									  { -sin(angle),cos(angle) } };
			double CbyCgrTmpVector[2];

			for (int r = 0; r < 2; r++) {
				CbyCgrTmpVector[r] = 0;
				for (int c = 0; c < 2; c++)
				{
					CbyCgrTmpVector[r] += inverseR[r][c] * LCbyCgrScaledVector[1 + c];
				}
			}

			for (int r = 0; r < 2; r++)
			{
				LCbyCgrScaledVector[1 + r] = CbyCgrTmpVector[r];
			}

#pragma endregion

#pragma region undo LCC converting

			Vec3b  intensity;
			for (int r = 0; r < 3; r++) {
				double value = 0;
				for (int c = 0; c < 3; c++)
				{
					value += reverseCoersionMatrix[r][c] * LCbyCgrScaledVector[c];
				}

				double scaledIntensity = value *255.0;

				//to avoid overflowing 
				//as consequence of rounding in calculations
				if (scaledIntensity > 255)
				{
					scaledIntensity = 255;
				}
				if (scaledIntensity < 0)
				{
					scaledIntensity = 0;
				}

				intensity[2 - r] = scaledIntensity;
			}
			resImage.at<Vec3b>(i, j) = intensity;
#pragma endregion
		}
	}
}

Mat ORGBImage::GetTestImage(double shiftingFactor) {
	Mat resImage(image.rows * 3, image.cols * 3, image.type());
	double g_r_shiftingFactor = shiftingFactor;
	for (int r = 0; r < 3; r++)
	{
		double b_y_ShiftingFactor = -shiftingFactor;
		for (int c = 0; c < 3; c++) {
			DrawORGBImageWithLinearTransformation(Mat(resImage, Rect(image.cols*c, image.rows*r, image.cols, image.rows)),
				1.0, 1.0, 1.0, 0.0, b_y_ShiftingFactor, g_r_shiftingFactor);
			b_y_ShiftingFactor += shiftingFactor;
		}
		g_r_shiftingFactor -= shiftingFactor;
	}
	//Use Return Value Optimization to avoid copying
	return resImage;
}