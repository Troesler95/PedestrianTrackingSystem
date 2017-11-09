#include <algorithm>
#include <iostream>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video/background_segm.hpp>
using namespace std;
using namespace cv;

#define MEDIAN_SIZE 5

int main(void)
{
	// create a pointer to a BackgroundSubtractorMOG2
	Ptr<BackgroundSubtractorMOG2> bkgSubractor = createBackgroundSubtractorMOG2(
		33, // Use 33 training images
		150, // Use a threshold of 150. This means we only take things that have moved a whole bunch!
		false // Do not detect shadows
	);

	// default inImg
	Mat inImg;
	
	// container for the masked image returned by the subtractor
	// This will be a binary image (0 = black, 1 = white)
	Mat subMask2;
	
	// obviously, change this to be your own file path
	cv::String filePrefix = "C:\\Users\\troes\\Desktop\\AI\\epfl_lab\\20140804_160621_00\\rgb000";
	cv::String filename = "";

	// return the proper length number suffix
	auto GetStringFromNum = [] (int num) -> string {
		return (num < 10) ? "00" + to_string(num)
			: (num < 100) ? "0" + to_string(num)
			: (num < 1000) ? to_string(num) : "";
	};

	// train our subtractor
	for (size_t i = 0; i < 34; ++i)
	{
		// build the string
		filename = filePrefix + GetStringFromNum(i) + ".png";
		inImg = imread(filename, IMREAD_COLOR);
		if (inImg.data == NULL)
		{
			cout << "ERROR: Unable to open \"" << filename << "\"!" << endl;
			continue;
		}
		// reduce image size by 50 % to increase speed
		// TODO: Play with several sizes to find best overall results
		resize(inImg, inImg, Size(), 0.5, 0.5, INTER_LINEAR); 
		cout << "Training on subtractor on: " << filename << endl;

		// apply the subtractor to the current image
		// store the result in the subMask2 container
		bkgSubractor->apply(inImg, subMask2, -1); // -1 == automatic learning rate
	}

	namedWindow("InImg", cv::WINDOW_AUTOSIZE);
	namedWindow("Subtracted img", cv::WINDOW_AUTOSIZE);

	for (size_t i = 34; i < 950; ++i)
	{
		filename = filePrefix + GetStringFromNum(i) + ".png";
		inImg = imread(filename, IMREAD_COLOR);
		if (inImg.data == NULL)
		{
			cout << "ERROR: Unable to open \"" << filename << "\"!" << endl;
			continue;
		}
		resize(inImg, inImg, Size(), 0.5, 0.5, INTER_LINEAR);

		bkgSubractor->apply(inImg, subMask2, 0);

		// TODO: Compare results from standard subtraction, closing, and median filtering
		//
		// get rid of black noise in binary image
		Mat element = getStructuringElement(MORPH_ELLIPSE, Size(9, 9));
		morphologyEx(subMask2, subMask2, MORPH_CLOSE, element);
		//
		// get rid of black noise via Med filtering
		// medianBlur(subMask2, subMask2, MEDIAN_SIZE);

		// Show the result to the user
		imshow("InImg", inImg);
		imshow("Subtracted img", subMask2);
		
		// waits 20ms before continuing
		waitKey(20);
	}
		// destroy them so we don't have a bunch
		// of open windows at once
		destroyAllWindows();
	return 0;
}