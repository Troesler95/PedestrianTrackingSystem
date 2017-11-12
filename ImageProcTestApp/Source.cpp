#include <algorithm>
#include <iostream>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/video/background_segm.hpp>
#include <yaml-cpp/yaml.h>
using namespace std;
using namespace cv;

#define MEDIAN_SIZE 5
typedef std::map<int, std::vector<cv::Rect>> TruthFrameMap;
// obviously, change this to be your own file path
const cv::String labDir = "C:\\Users\\troes\\Desktop\\AI\\epfl_lab\\"; 
const cv::String imFilePrefix = labDir + "20140804_160621_00\\rgb000";

// return the correct length number suffix
inline string GetStringFromNum(int num) {
	return (num < 10) ? "00" + to_string(num)
		: (num < 100) ? "0" + to_string(num)
		: (num < 1000) ? to_string(num) : "";
}


// FUNCTION HEADERS
// Dataset I/O
TruthFrameMap ParseImagePlaneDataset(std::string);
// Background subtraction
Ptr<BackgroundSubtractorMOG2> GetAndTrainBkgSubtractor(size_t trainIms = 33, size_t thresh = 150, bool shadows = false);
void FindPeopleBkgSub(Ptr<BackgroundSubtractorMOG2>, TruthFrameMap*);

int main(void)
{
	// default inImg
	Mat inImg;

	// container for the masked image returned by the subtractor
	// This will be a binary image (0 = black, 1 = white)
	Mat subMask2;
	cv::String filename = "";
	cv::String imgTruth = labDir + "ground_truth_image_plane.yaml";

	TruthFrameMap truthBoxes = ParseImagePlaneDataset(imgTruth);

	Ptr<BackgroundSubtractorMOG2> bkgSub = GetAndTrainBkgSubtractor();
	FindPeopleBkgSub(bkgSub, &truthBoxes);

	cv::waitKey();
	return 0;
}

/** ParseImagePlaneDataset
 * Given the file path to the gound_truth_image_plane.yaml dataset,
 * return a map structure that contains each frame and the bounding boxes
 * associated with them
 *
 * @parameter dataFile: filepath to the ground_truth_image_plane.yaml file
 *
 * @returns: a TruthFrameMap that contains each of the frames in the dataset and
 *			 their associated objects and bounding boxes
 *
 * @remarks:
 *		This is a very specific function, but if we are to scale it
 *		to work with other dataset truth files (i.e. those found in the corridor dataset)
 *		conderation should be given to making the operations more generic to those
 *      sets as well.
 */
TruthFrameMap ParseImagePlaneDataset(string dataFile)
{
	// NOTE: One should definitely check if the file exists before loading it!!!
	vector<YAML::Node> file = YAML::LoadAllFromFile(dataFile); // load the file into a list of YAML nodes
	YAML::Node topLevel = file.front()["20140804_160621_00"];  // we know that there is only one top-level node that 
															   // contains the rest of the data, so lets just ignore it

	TruthFrameMap truthBoxes; // map structure to store the frame and the associated objects and bounding boxes
							  // see the typedef above to see how it is stored
	size_t key = 34;	      // start off at 34 since the data starts there

	// This look iterates through each of the frames in the file (34-949)
	for (YAML::const_iterator it = topLevel.begin(); it != topLevel.end(); it++)
	{
		// makes for easier syntax
		YAML::Node objs = (*it).second;
		// temp storage to later be put in the map
		vector<cv::Rect> temp = vector<cv::Rect>();
		// iterate through each of the objects associated with the frame
		for (YAML::const_iterator it2 = objs.begin(); it2 != objs.end(); it2++)
			// we know the structure looks like this [x, y, w, h]
			// so we create a Rect object from this list and push it to our vector
			temp.push_back(Rect((*it2).second[0].as<int>(), (*it2).second[1].as<int>(),
			(*it2).second[2].as<int>(), (*it2).second[3].as<int>()));
		// store the list in the map structure with the frame # as the key
		truthBoxes[key] = temp;
		++key;
	}
	return truthBoxes;
}

/**	GetAndTrainBkgSubtractor
 * This function creates a new BackgroundSubtractorMOG object, trains it, and returns it to main
 *
 * @parameter trainIms: number of images to train the subtractor on. These should be photos without people in them!
 * @parameter thresh: Theshold of the Mahalanobis distance to take as objects not within the background
 * @parameter shadows: whether or not the background subtractor should pay attention to shadows of models
 *
 * @returns: the trained background subtractor cv pointer object
 *
 * @remarks: 
 *     It was far simpler to combine the two functionalities (get and train) into one
 *     rather than worry about pointers and such. In the future (if this function is to be kept)
 *	   these should be separated.
 */
Ptr<BackgroundSubtractorMOG2> GetAndTrainBkgSubtractor(size_t trainIms, size_t thresh, bool shadows)
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
		filename = imFilePrefix + GetStringFromNum(i) + ".png";
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
void FindPeopleBkgSub(Ptr<BackgroundSubtractorMOG2> bkgSubtractor, TruthFrameMap* truthMap)
{
	Mat inImg, subMask2;
	string filename = "";
	auto ReducePointSize = [=](cv::Point point) -> cv::Point {
		return Point(point.x / 2, point.y / 2);
	};

	// Open up windows to show our processes
	namedWindow("InImg", cv::WINDOW_AUTOSIZE);
	namedWindow("Subtracted img", cv::WINDOW_AUTOSIZE);
	namedWindow("Bounding Box", cv::WINDOW_AUTOSIZE);

	// play the remaining images like a video
	for (size_t i = 34; i < 950; ++i)
	{
		filename = imFilePrefix + GetStringFromNum(i) + ".png";
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
			//rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), color, 1);
			rectangle(inImg, boundRect[i].tl(), boundRect[i].br(), color, 1);	// draw box on original image
		}
		
		for(const auto &box: truthMap->at(i))
			rectangle(inImg, ReducePointSize(box.tl()), 
				ReducePointSize(box.br()), Scalar(0, 0, 255), 1);

		// update results in the windows
		imshow("Bounding Box", drawing);
		imshow("InImg", inImg);
		// waits 20ms before continuing
		waitKey(15);
	}
	// destroy them all
	destroyAllWindows();
}