#pragma once
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>
#include <string>
#include <math.h>
#include <Queue>
#include <algorithm>

using namespace std;
using namespace cv;

typedef map<int, cv::Rect>* frame;
typedef map<int, int> detectionMap;	// Maps previous detection to current detection
typedef map<int, cv::Rect>* frame;

// Contains the binary tree that will holds the data for the current and previous frame
class FramesController
{
private:
	frame current = nullptr;	// Holds all the detections of the current frame
	frame previous = nullptr; // Holds all the detections of the previous frame
	unsigned long int currentFrameNum = 0;	// Holds the current frame number
public:
	FramesController();	// Default constructor
	bool createNewFrame(frame data);	// Creates a new frame and stores the data in it
	// Can use to get information about the a detection in the current or previous frame. Just pass the index of the detection, and some field, ex.) double value = frameObject.getDetection(1).x; to get information about the previous frame, pass the string: "previous"
	// ex.) double value = frameObject.getDetection(3,"previous").height;
	Rect getDetection(int detection, string type);
	frame getCurrentFrameData();	// Returns the current frame (as a "frame" object)
	frame getPreviousFrameData();	// Returns the previous frame (as a "frame" object)
	int currentDetectionsSize();	// Gets the number of detections in the current frame
	int previousDetectionsSize();	// Gets the number of detections in the previous frame
	bool previousFrameExists();	// Checks if the current frame exists
	bool currentFrameExists();	// Checks is the previous frame exists
	~FramesController();
};

/* This is the class that tracks the detections in each frame. It takes a map<int, Rect>, where int is the identifier of the detection and Rect is a Rect object supplied by OpenCV, which contains the x,y,height, and width of the detection. When the map<int, Rect> is
passed to a TrackingController object, it creates a new frame with the object using a FramesController object. That FramesController object holds all the data for the current frame and the previous frame. To get the predictions for the current frame, call the
getPredictions member function, which will return the */
class TrackingController
{
private:
	FramesController controller;	// Holds the data for the current and previous frames
	map<int, bool> predictedExists;	// The first int represents the current detection, and the map within represents the previous detection its been predicted to belong to, along with the generated score
	map<int, map<int, double>> predictionScoreRecords;	// A record of the best scores generated for each current detection (for each previous detection. the first int is the previous detection, the first int in the map is the current detection, and the second int it the score)
	map<int, int> predictions;	// The first int is the previous detection and the second int is the current detection
	double distWeight = .85;	// Weight given to distance
	double sizeWeight = .5;	// Weight given to size

	double calcDetectionDist(int currDetection, int prevDetection);	// calculates the distance between each current detection and the previous detection
	double calcSize(int currDetection, int prevDetection);		// calculates the size of each detection
																//	int calcDirection();	// calculates the direction each detection is going to
																//	int calcSpeed();	// calculates what the speed of the detection is (compared to the current detection) 

	void generateScores();
	void setPredictions();

public:
	TrackingController();
	TrackingController(map<int, Rect>* dataSet);	// The tracking controller can be initialized with the first frame and the corresponding data
	void nextFrame(map<int, Rect>* dataSet);	// Creates a new frame and makes it the current, pushes the old current to previous, and gets rid of previous
	detectionMap getPredictions();		// Returns map<int,int> where the first int is the identifier of the previous detection and the second int is the identifier of the most likely current detection
};
