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
float GammaExpand_sRGB(float nonlinear)
{
	return nonlinear;
	return   (nonlinear <= 0.04045f)
		? (nonlinear / 12.92f)
		: (powf((nonlinear + 0.055f) / 1.055f, 2.4f));
}

//******************************************************************************
// Convert a linear sRGB color channel to a sRGB color channel.
//******************************************************************************
float GammaCompress_sRGB(float linear)
{
	return linear;
	return   (linear <= 0.0031308f)
		? (12.92f * linear)
		: (1.055f * powf(linear, 1.0f / 2.4f) - 0.055f);
}

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
	this->oRGBTheta = new double*[image.rows];

	for (int i = 0; i < image.rows; i++)
	{
		oRGB[i] = new double*[image.cols];
		oRGBTheta[i] = new double[image.cols];
		for (int j = 0; j < image.cols; j++)
		{
			oRGB[i][j] = new double[3];
			Vec3b  intensity = image.at<Vec3b>(i, j);//get pixel intensity

			for (int ch_i = 0; ch_i < 3; ch_i++)
			{
				//this scaling to range [0,1] isn't neccesary for image processing,
				//but it's needed to get origin oRGB space
				scaledOntersity[ch_i] = GammaCompress_sRGB(intensity[2-ch_i] / 255.0);
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
			if (theta > 0) {
				if (theta < M_PI / 3.0)
				{
					oRGBTheta[i][j] = 1.5*theta;
				}
				else
				{
					if (theta <= M_PI)
					{
						oRGBTheta[i][j] = M_PI_2 + 0.75*(theta - M_PI / 3.0);
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
					oRGBTheta[i][j] = 1.5*theta;
				}
				else
				{
					if (theta >= -M_PI)
					{
						oRGBTheta[i][j] = -(M_PI_2 + 0.75*(-theta - M_PI / 3.0));
					}
					else
					{
						throw ErrorClass(10, "unexpected angle");
					}
				}
			}
			//remark: if angle is less then some small parametr e. g. 1e-9, 
			//we could do no rotating for efficiency
			double angle = oRGBTheta[i][j] - theta;

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

			/*double oRGBTheta_2 = atan2(oRGB[i][j][2], oRGB[i][j][1]);
			if (abs(oRGBTheta[i][j] - oRGBTheta_2) > 1e-3)
			{
				oRGBTheta[i][j] = oRGBTheta_2;
			}*/
		}
	}
}
void ORGBImage::ScaleORGBChannel(double alpha, uint channelIndex) {
	if (channelIndex > 2)
		throw ErrorClass(20, "wrong channel index");
	for (int i = 0; i < image.rows; i++)
		for (int j = 0; j < image.cols; j++)
			oRGB[i][j][channelIndex] *= alpha;
}

void ORGBImage::ScaleBlueYellowChannel(double alpha)
{
	ScaleORGBChannel(alpha, 1);
}

void ORGBImage::ScaleGreenRedChannel(double alpha)
{
	ScaleORGBChannel(alpha, 2);
}

Mat& ORGBImage::GetOriginImage() { return image; }

Mat ORGBImage::GetImageFromORGB() {
	Mat resImage(image.rows, image.cols, image.type());// , Scalar(0, 0, 255));
	//return resImage;
	for (int i = 0; i < image.rows; i++)
	{
		for (int j = 0; j < image.cols; j++)
		{
#pragma region undo rotation
			double oRGBTheta_2 = atan2(oRGB[i][j][2], oRGB[i][j][1]);
			/*if (abs(oRGBTheta[i][j] - oRGBTheta_2) > 1e-5)
			{
				oRGBTheta[i][j] = oRGBTheta_2;
			}*/
			//oRGBTheta[i][j] = oRGBTheta_2;
			double theta;
			if (oRGBTheta[i][j] > 0) {
				if (oRGBTheta[i][j] < M_PI_2)
				{
					theta = (2.0*oRGBTheta[i][j]) / 3.0;
				}
				else
				{
					if (oRGBTheta[i][j] <= M_PI)
					{
						theta = M_PI / 3.0 + (4.0 / 3.0)*(oRGBTheta[i][j] - M_PI_2);
					}
					else
					{
						throw ErrorClass(11, "unexpected angle");
					}

				}
			}
			else
			{
				if (oRGBTheta[i][j] > -M_PI_2)
				{
					theta = (2.0*oRGBTheta[i][j]) / 3.0;
				}
				else
				{
					if (oRGBTheta[i][j] >= -M_PI)
					{
						theta =-( M_PI / 3.0 + (4.0 / 3.0)*(-oRGBTheta[i][j] - M_PI_2));
					}
					else
					{
						throw ErrorClass(11, "unexpected angle");
					}

				}
			}

			double angle = (oRGBTheta[i][j] - theta);
			double inverseR[2][2] = { { cos(angle), sin(angle) } ,
									  { -sin(angle),cos(angle) } };
			double C1C2Vector[2];

			for (int r = 0; r < 2; r++) {
				C1C2Vector[r] = 0;
				for (int c = 0; c < 2; c++)
				{
					C1C2Vector[r] += inverseR[r][c] * oRGB[i][j][1 + c];
				}
			}

			for (int r = 0; r < 2; r++)
			{
				oRGB[i][j][1 + r] = C1C2Vector[r];
			}

#pragma endregion

#pragma region undo LCC converting

			Vec3b  intensity;
			for (int r = 0; r < 3; r++) {
				double value = 0;
				for (int c = 0; c < 3; c++)
				{
					value += reverseCoersionMatrix[r][c] * oRGB[i][j][c];
				}
				if (value < 0)
				{
					if (value < -0.3)
						value = value;
					value = 0;
				}
				if (value > 1)
				{
					if (value > 1.3)
						value = value;
					value = 1;
				}
				value = GammaExpand_sRGB(value);

				double scaledIntensity = value *255.0;

				//to avoid overflowing 
				//in consequence of rounding in calculations
				if (scaledIntensity > 255)
				{
					double d = 255 - scaledIntensity;
					if (d > 10)
						d = -d;
					scaledIntensity = 255;
				}
				if (scaledIntensity < 0)
				{
					if (scaledIntensity < -10)
						scaledIntensity = -scaledIntensity;
					scaledIntensity = 0;
				}

				intensity[2-r] = scaledIntensity;
			}
			/*intensity[0] = 0;
			intensity[1] = 255;
			intensity[2] = 0;*/
			resImage.at<Vec3b>(i, j) = intensity;
#pragma endregion
		}
	}

	return resImage;
}