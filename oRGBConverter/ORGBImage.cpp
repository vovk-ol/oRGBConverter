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

double ORGBImage::AngleTransform(double theta) {
	if (theta < M_PI / 3.0)
	{
		return 1.5*theta;
	}
	else
	{
		if (theta <= M_PI)
		{
			return M_PI_2 + 0.75*(theta - M_PI / 3.0);
		}
	}
	throw ErrorClass(10, "unexpected angle");
}
double ORGBImage::AngleReverseTransform(double oRGB_theta) {
	if (oRGB_theta < M_PI_2)
	{
		return (2.0*oRGB_theta) / 3.0;
	}
	else
	{
		if (oRGB_theta <= M_PI)
		{
			return M_PI / 3.0 + (4.0 / 3.0)*(oRGB_theta - M_PI_2);
		}
	}
	throw ErrorClass(11, "unexpected angle");
}

ORGBImage::ORGBImage()
{
	this->oRGB = NULL;
}
ORGBImage::ORGBImage(Mat _image) : image(_image) {

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

			Vec3b  intensity = image.at<Vec3b>(i, j);
			for (int ch_i = 0; ch_i < 3; ch_i++)
			{
				scaledOntersity[ch_i] = intensity[2 - ch_i] / 255.0;
			}

#pragma region LCC converting
			for (int r = 0; r < 3; r++) {
				oRGB[i][j][r] = 0;
				for (int c = 0; c < 3; c++)
				{
					oRGB[i][j][r] += coersionMatrix[r][c] * scaledOntersity[c];
				}
			}
#pragma endregion

#pragma region rotating

			double theta = atan2(oRGB[i][j][2], oRGB[i][j][1]);
			double oRGBTheta = 0;

			if (theta > 0)
				oRGBTheta = AngleTransform(theta);
			else
				oRGBTheta = -AngleTransform(-theta);

			//remark: if angle is less then some small parametr e. g. 1e-9, 
			//we could do no rotating for efficiency
			double angle = oRGBTheta - theta;

			//Point rotation
			double Cyb, Crg;
			double rotateMatrixCos = cos(angle);
			double rotateMatrixSin = sin(angle);
			Cyb = rotateMatrixCos*oRGB[i][j][1] - rotateMatrixSin*oRGB[i][j][2];
			Crg = rotateMatrixSin*oRGB[i][j][1] + rotateMatrixCos*oRGB[i][j][2];
			oRGB[i][j][1] = Cyb;
			oRGB[i][j][2] = Crg;
#pragma endregion
		}
	}
}
ORGBImage::ORGBImage(const ORGBImage& oRGBImage) :image(oRGBImage.image)
{
	oRGB = new double**[image.rows];
	for (int i = 0; i < image.rows; i++)
	{
		oRGB[i] = new double*[image.cols];
		for (int j = 0; j < image.cols; j++)
		{
			oRGB[i][j] = new double[3];
			for (int ci = 0; ci < 3; ci++)
			{
				oRGB[i][j][ci] = oRGBImage.oRGB[i][j][ci];
			}
		}
	}
}
ORGBImage& ORGBImage::operator=(const ORGBImage& oRGBImage) {
	if (this != &oRGBImage)
	{
		//resize oRGB dimention
		if (this->image.rows != oRGBImage.image.rows
			|| this->image.cols != oRGBImage.image.cols)
		{
			//remove this->oRGB
			if (this->oRGB != NULL)
			{
				for (int i = 0; i < oRGBImage.image.rows; i++)
				{
					for (int j = 0; j < oRGBImage.image.cols; j++)
					{
						delete[]this->oRGB[i][j];
					}
					delete[]this->oRGB[i];
				}
				delete[]this->oRGB;
			}
			//create new this->oRGB
			this->oRGB = new double**[oRGBImage.image.rows];
			for (int i = 0; i < oRGBImage.image.rows; i++)
			{
				this->oRGB[i] = new double*[oRGBImage.image.cols];
				for (int j = 0; j < oRGBImage.image.cols; j++)
					this->oRGB[i][j] = new double[3];
			}
		}

		this->image = oRGBImage.image;

		//copy new valuer to this->oRGB
		for (int i = 0; i < image.rows; i++)
			for (int j = 0; j < image.cols; j++)
				for (int ci = 0; ci < 3; ci++)
					this->oRGB[i][j][ci] = oRGBImage.oRGB[i][j][ci];				

	}
	return *this;
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
	
	if (this->oRGB == NULL)
	{
		return;
	}

	for (int i = 0; i < image.rows; i++)
	{
		for (int j = 0; j < image.cols; j++)
		{
			double LCbyCgrScaledVector[3];
			LCbyCgrScaledVector[0] = lumaScaleFactor*oRGB[i][j][0] + lumaShiftingFactor;
			LCbyCgrScaledVector[1] = blueYellowScaleFactor*oRGB[i][j][1] + blueYellowShiftingFactor;
			LCbyCgrScaledVector[2] = greenRedScaleFactor*oRGB[i][j][2] + greenRedShiftingFactor;

#pragma region undo rotation
			double oRGBTheta = atan2(LCbyCgrScaledVector[2], LCbyCgrScaledVector[1]);
			double theta;
			if (oRGBTheta > 0)
				theta = AngleReverseTransform(oRGBTheta);
			else
				theta = -AngleReverseTransform(-oRGBTheta);

			//remark: if angle is less then some small parametr e. g. 1e-9, 
			//we could do no rotating for efficiency
			double angle = oRGBTheta - theta;

			//rotation
			double Cby, Cgr;
			double rotateMatrixCos = cos(angle);
			double rotateMatrixSin = sin(angle);
			Cby = rotateMatrixCos*LCbyCgrScaledVector[1] + rotateMatrixSin*LCbyCgrScaledVector[2];
			Cgr = -rotateMatrixSin*LCbyCgrScaledVector[1] + rotateMatrixCos*LCbyCgrScaledVector[2];
			LCbyCgrScaledVector[1] = Cby;
			LCbyCgrScaledVector[2] = Cgr;

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
	if (this->oRGB == NULL)
	{
		return resImage;
	}

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

ORGBImage::~ORGBImage() {
	if (oRGB != NULL)
	{
		for (int i = 0; i < image.rows; i++)
		{
			for (int j = 0; j < image.cols; j++)
			{
				delete[] oRGB[i][j];
			}
			delete[] oRGB[i];
		}
		delete[] oRGB;
	}
}