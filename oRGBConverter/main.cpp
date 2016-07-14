/*
author: vovk, vovk@windowslive.com

*/

#include "oRGBImage.h"


/*
load image, check format, file corruption, etc.
*/
void LoadImage(string inputFileName, Mat& image)
{
	image = imread(inputFileName, IMREAD_COLOR); // Read the file
	if (image.empty()) // Check for invalid input
	{
		throw string("invalid input");//use string for simplicity
	}
	return;
}

int main(int argc, char** argv)
{
	string inputFileName = "image.png";
	string outputFileName = "out.png";

	//Image processing
	Mat image;
	try {
		LoadImage(inputFileName, image);
	}
	catch (string s)
	{
		cout << "image processing error: " << s << endl;
		system("pause");
		return 1;
	}
	ORGBImage oRGBImage(image);

	Mat resImage = oRGBImage.GetTestImage(0.1);

	vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(9);

	try {
		imwrite(outputFileName, resImage, compression_params);
	}
	catch (runtime_error& ex) {
		cout<<"Exception converting image to PNG format: "<<ex.what()<<endl;
		//return 1;
	}


	//Mat resImage(image.rows*3, image.cols*3, image.type());
	////image.copyTo(resImage(Range(image.rows, image.rows), Range(image.cols, image.cols)));

	////double a = 1.5;
	//oRGBImage.SetBlueYellowScaleFactor(2);
	//Mat img = oRGBImage.GetImageFromORGB();
	////cout << &img << endl;
	//hconcat(img, image, resImage);

	/*oRGBImage.ScaleYellowBlueChannel(1.0);
	img = oRGBImage.GetImageFromORGB();
	hconcat(resImage, img, resImage);*/

	//oRGBImage.ScaleRedGreenChannel(1 / a);
	//img = oRGBImage.GetImageFromORGB();
	//vconcat(resImage, resImage, resImage);

	//namedWindow("Display window", WINDOW_AUTOSIZE); // Create a window for display.
	//imshow("Display window", resImage); // Show our image inside it.

	//waitKey(0); // Wait for a keystroke in the window



	system("pause");

	return 0;
}