#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/video/background_segm.hpp>
using namespace std;
using namespace cv;

int main(void)
{
	// create a pointer to a BackgroundSubtractorMOG2
	Ptr<BackgroundSubtractorMOG2> bkgSubractor = createBackgroundSubtractorMOG2(33, 16.0, true);
	// default inImg
	Mat inImg = Mat();
	// container for the masked image returned by
	// the subtractor
	// This will be a binary image (0 = black, 1 = white)
	Mat subMask2;
	// obviously, change this to be your own file path
	cv::String filePrefix = "C:\\Users\\troes\\Desktop\\AI\\epfl_lab\\20140804_160621_00\\rgb0000";

	// test out our subtractor
	for (int i = 10; i < 56; ++i)
	{
		// This logic handles adding the number of the image
		// to read.
		if (i < 10)
		{
			cout << filePrefix + '0' + (char)(i + '0') + ".png" << endl;
			inImg = imread(filePrefix + '0' + (char)(i + '0') + ".png", IMREAD_COLOR);
		}
		// split up the integers based by 10's and 1's place
		else
		{
			char cont1 = (char)((i % 10) + '0');
			char cont2 = (char)((i / 10) + '0');
			inImg = imread(filePrefix + cont2 + cont1 + ".png", 1);
		}

		// apply the subtractor to the current image
		// store the result in the subMask2 container
		bkgSubractor->apply(inImg, subMask2);

		// Show the result to the user
		namedWindow("InImg", cv::WINDOW_AUTOSIZE);
		imshow("InImg", inImg);
		namedWindow("Subtracted img", cv::WINDOW_AUTOSIZE);
		imshow("Subtracted img", subMask2);
		waitKey(0);
		// destroy them so we don't have a bunch
		// of open windows at once
		destroyWindow("InImg");
		destroyWindow("Subtracted img");
	}

	cin.get();

	return 0;
}