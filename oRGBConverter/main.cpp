/*
author: vovk, vovk@windowslive.com

*/

#include "oRGBImage.h"
#include <time.h>

/*
load image, check format, file corruption, etc.
*/
void LoadImage(string inputFileName, Mat& image)
{
	image = imread(inputFileName, IMREAD_COLOR);
	if (image.empty()) // Check for invalid input
	{
		throw ErrorClass(25, "invalid input");
	}
	return;
}

int main(int argc, char** argv)
{
#pragma region get command line arguments

	string inputFileName = "image.png";
	string outputFileName = "out.png";
	double shiftingFactor = 0.15;
	bool openImageAfterProcessing = false;

	try {
		//0 - nothing
		//1 - src find
		//2 - dest find
		//3 - shifting factor find
		//4 - openImage=true
		int argFoundIndex = 0;
		for (int ai = 1; ai < argc; ai++)
		{
			if (argFoundIndex != 0)
			{
				switch (argFoundIndex)
				{
				case 1:
					inputFileName = string(argv[ai]);
					break;
				case 2:
					outputFileName = string(argv[ai]);
					break;
				case 3:
					shiftingFactor = atof(argv[ai]);
					break;
				}
				argFoundIndex = 0;
			}
			else {
				if (string(argv[ai]) == "-src")
					argFoundIndex = 1;
				if (string(argv[ai]) == "-dest")
					argFoundIndex = 2;
				if (string(argv[ai]) == "-sf")
					argFoundIndex = 3;
				if (string(argv[ai]) == "-open")
					openImageAfterProcessing = true;
			}
		}
	}
	catch (...)
	{
		std::cout << "getting command parametrs error";
		std::system("pause");
		return 1;
	}
#pragma endregion

	std::cout << "processing started with parametrs:\n";
	std::cout << "input image path: " << inputFileName << "\n";
	std::cout << "output image path: " << outputFileName << "\n";
	std::cout << "shifting factor: " << shiftingFactor << "\n";

	const clock_t startProcessingTime = clock();
	Mat image;
	try {
		LoadImage(inputFileName, image);
	}
	catch (...)
	{
		std::cout << "image loading error";
		std::system("pause");
		return 1;
	}

	Mat resImage;
	try {
		ORGBImage oRGBImage(image);
		resImage = oRGBImage.GetTestImage(shiftingFactor);		
	}
	catch (...)
	{
		std::cout << "image processing error";
		std::system("pause");
		return 1;
	}

	try {
		imwrite(outputFileName, resImage);
	}
	catch (runtime_error& ex) {
		std::cout << "Exception converting image to PNG format: " << ex.what() << endl;
		std::system("pause");
		return 1;
	}
	catch (...)
	{
		std::cout << "output image saving error";
		std::system("pause");
		return 1;
	}

	if (openImageAfterProcessing)
	{
		try {
			// Create a window for display.
			namedWindow("Display window", WINDOW_AUTOSIZE);
			// Show image
			imshow("Display window", resImage);
			
		}
		catch (...)
		{
			std::cout << "show image error";
			std::system("pause");
			return 1;
		}
		// Wait for a keystroke in the window
		waitKey(0);
	}

	std::cout << float(clock() - startProcessingTime) / CLOCKS_PER_SEC;
	std::cout << "image processing is successful!\n"
		<<"Processing time = "<< float(clock() - startProcessingTime) / CLOCKS_PER_SEC
		<<"sec \n\n";
	std::system("pause");

	return 0;
}