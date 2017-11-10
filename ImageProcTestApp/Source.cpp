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
// obviously, change this to be your own file path
const cv::String filePrefix = "C:\\Users\\troes\\Desktop\\AI\\epfl_lab\\20140804_160621_00\\rgb000";

// return the correct length number suffix
inline string GetStringFromNum(int num) {
	return (num < 10) ? "00" + to_string(num)
		: (num < 100) ? "0" + to_string(num)
		: (num < 1000) ? to_string(num) : "";
}


// FUNCTION HEADERS
Ptr<BackgroundSubtractorMOG2> GetAndTrainBkgSubtractor(size_t, size_t, bool);
void FindPeopleBkgSub(Ptr<BackgroundSubtractorMOG2>);

int main(void)
{
	// default inImg
	Mat inImg;

	// container for the masked image returned by the subtractor
	// This will be a binary image (0 = black, 1 = white)
	Mat subMask2;
	cv::String filename = "";

	Ptr<BackgroundSubtractorMOG2> bkgSubtractor = GetAndTrainBkgSubtractor();
	FindPeopleBkgSub(bkgSubtractor);
	
	return 0;
}

/**	GetAndTrainBkgSubtractor
* This function creates a new BackgroundSubtractorMOG object, trains it, and returns it to main
*
* @parameter trainIms: number of images to train the subtractor on. These should be photos without people in them!
* @parameter thresh: Theshold of the Mahalanobis distance to take as objects not within the background
* @parameter shadows: whether or not the background subtractor should pay attention to shadows of models
*
* @remarks: 
*     It was far simpler to combine the two functionalities (get and train) into one
*     rather than worry about pointers and such. In the future (if this function is to be kept)
*	  these should be separated.
*/
Ptr<BackgroundSubtractorMOG2> GetAndTrainBkgSubtractor(size_t trainIms=33, size_t thresh=150, bool shadows=false)
{
	// create a pointer to a BackgroundSubtractorMOG2
	 Ptr<BackgroundSubtractorMOG2> bkgSub = createBackgroundSubtractorMOG2(
		trainIms, // Use 33 training images
		thresh, // Use a threshold of 150. This means we only take things that have moved a whole bunch!
		shadows // Do not detect shadows
	);

	Mat trainImg;
	string filename = "";

	// train our subtractor
	for (size_t i = 0; i < 34; ++i)
	{
		// build the string
		filename = filePrefix + GetStringFromNum(i) + ".png";
		trainImg = imread(filename, IMREAD_COLOR);
		if (trainImg.data == NULL)
		{
			cout << "ERROR: Unable to open \"" << filename << "\"!" << endl;
			continue;
		}

		// reduce image size by 50 % to increase speed
		// TODO: Play with several sizes to find best overall results
		resize(trainImg, trainImg, Size(), 0.5, 0.5, INTER_LINEAR);
		cout << "Training on subtractor on: " << filename << endl;

		// apply the subtractor to the current image
		// store the result in the subMask2 container
		bkgSub->apply(trainImg, trainImg, -1); // -1 == automatic learning rate
	}
	return bkgSub;
}

/** FindPeopleBkgSub
 * Using the provided background subtractor, find people in the images.
 * 
 * @parameter bkgSubtractor: The background subtractor to use in finding the binary masked image
 * 
 * @remarks: 
 *     This is a naieve model that uses only the binary image returned by the subtractor to
 *     place a bounding box on the contours found. This model doesn't handle occlusion at all
 *     and is ignorant to the depth data we have available!
 */
void FindPeopleBkgSub(Ptr<BackgroundSubtractorMOG2> bkgSubtractor)
{
	Mat inImg, subMask2;
	string filename = "";

	// Open up windows to show our processes
	namedWindow("InImg", cv::WINDOW_AUTOSIZE);
	namedWindow("Subtracted img", cv::WINDOW_AUTOSIZE);
	namedWindow("Bounding Box", cv::WINDOW_AUTOSIZE);

	// play the remaining images like a video
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

		bkgSubtractor->apply(inImg, subMask2, 0);

		// TODO: Compare results from standard subtraction, closing, and median filtering
		//
		// get rid of black noise in binary image via closing
		Mat element = getStructuringElement(MORPH_RECT, Size(7, 7));
		morphologyEx(subMask2, subMask2, MORPH_CLOSE, element);
		//
		// get rid of black noise via Med filtering
		// medianBlur(subMask2, subMask2, MEDIAN_SIZE);

		// Show the result to the user
		imshow("Subtracted img", subMask2);

		vector<vector<Point>> contours;
		vector<Vec4i> hierarcy;

		// finds the contours in the returned binary images and stores them in the contours vector
		findContours(subMask2, contours, hierarcy, RETR_TREE, CHAIN_APPROX_TC89_L1);

		// remove if area of contour is less than 500 using the erase-remove_if idiom
		contours.erase(remove_if(contours.begin(),
								contours.end(),
								[&](const vector<Point> vec) -> bool
								{ return contourArea(vec) < 500; }),
						contours.end());

		// initialize vectors for the polygons and bounding
		// boxes that are the same size as the contours vector
		vector<vector<Point> > contours_poly(contours.size());
		vector<Rect> boundRect(contours.size());

		// Approximate shapes in the image,
		// then place a box around them
		for (size_t i = 0; i < contours.size(); i++)
		{
			// play with the last two values to try for better results
			approxPolyDP(Mat(contours[i]), contours_poly[i], 0.000005, true);
			boundRect[i] = boundingRect(Mat(contours_poly[i]));
		}

		// Draw the rectangles on the original image
		Mat drawing = Mat::zeros(subMask2.size(), CV_8UC3);
		for (size_t i = 0; i< contours.size(); i++)
		{
			// draw in green
			Scalar color = Scalar(0, 255, 0);
			drawContours(drawing, contours_poly, (int)i, color, 1, 8, vector<Vec4i>(), 0, Point());
			rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), color, 1);
			rectangle(inImg, boundRect[i].tl(), boundRect[i].br(), color, 1);	// draw box on original image
		}

		// update results in the windows
		imshow("Bounding Box", drawing);
		imshow("InImg", inImg);
		// waits 20ms before continuing
		waitKey(15);
	}
	// destroy them all
	destroyAllWindows();
}