#include "FramesController.h"
#include <vector>
#include <map>
#include <string>
#include <opencv2/opencv.hpp>
#include <math.h>
#include <Queue>
#include <algorithm>

using namespace cv;
using namespace std;

typedef map<int,int> detectionMap;	// Maps previous detection to current detection
typedef map<int, cv::Rect>* frame;

class FramesController
{
private:
	frame current = nullptr;	// Holds all the detections of the current frame
	frame previous = nullptr; // Holds all the detections of the previous frame
	unsigned long int currentFrameNum = 0;	// Holds the current frame number
public:
	FramesController();
    bool createNewFrame(frame data);	// Creates a new frame and stores the data in it
	Rect getDetection(int detection, string type);
	frame getCurrentFrameData();
	frame getPreviousFrameData();
	int currentDetectionsSize();
	int previousDetectionsSize();
	bool previousFrameExists();
	bool currentFrameExists();

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
	double distWeight = .9;	// Weight given to distance
	double sizeWeight = .6;	// Weight given to size
	double speedWeight = .9;	// Weight given to speed

	double calcDetectionDist(int currDetection, int prevDetection);	// calculates the distance between each current detection and the previous detection
	double calcSize(int currDetection, int prevDetection);		// calculates the size of each detection
	//int calcDirection();	// calculates the direction each detection is going to
	//double calcSpeed(int currDetection, int prevDetection);	// calculates what the speed of the detection is (compared to the current detection)

	void generateScores();
	void setPredictions();
	void resolveDifferences();
public:
	TrackingController();
	TrackingController(map<int, Rect>* dataSet);	// The tracking controller can be initialized with the first frame and the corresponding data
	void nextFrame(map<int, Rect>* dataSet);	// Creates a new frame and makes it the current, pushes the old current to previous, and gets rid of previous
	bool makePredictions();		// Returns map<int,int> where the first int is the identifier of the previous detection and the second int is the identifier of the most likely current detection
	detectionMap getPredictions();
};
