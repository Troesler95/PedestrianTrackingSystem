#include "pch.h"
#include "PTATracking.h"

<<<<<<< HEAD
// TrackingController class member function implementations



TrackingController::TrackingController()
{

}

TrackingController::TrackingController(map<int, Rect>* dataSet)
{
	controller.createNewFrame(dataSet);
}

void TrackingController::nextFrame(map<int, Rect>* dataSet)
{
	controller.createNewFrame(dataSet);
}

detectionMap TrackingController::getPredictions()
{
	if (controller.previousFrameExists() && controller.currentFrameExists())
	{
		generateScores();
		setPredictions();

		return predictions;
	}
}

void TrackingController::generateScores()
{
	// First we need to generate all the likelihood scores for all the current detections for each previous detection
	for (int i = 0; i < controller.previousDetectionsSize(); i++)	// Iterate for all the previous detections
	{
		map<int, double> scores;	// scores is a map, which holds the current detection identifier and the score generated for that current detection
		for (int p = 0; p < controller.currentDetectionsSize(); p++)	// For each of the previous detections, iterate through all of the current detections
		{
			double distValue = calcDetectionDist(p, i);	// Note if something is wrong try switching p and i
			double sizeValue = calcSize(p, i);
			distValue *= distWeight;
			sizeValue *= sizeWeight;
			scores.insert(pair<int, double>(p, distValue + sizeValue));	// p is the current detection and distValue + sizeValue is the score generated for that current detection
		}
		predictionScoreRecords.insert(pair<int, map<int, double>>(i, scores));	// Add the scores made for each current detection for the previous detection to the record
	}
}

void TrackingController::setPredictions()
{
	for (int previous = 0; previous < controller.previousDetectionsSize(); previous++)	// Iterate through each of the previous detections
	{
		map<int, double> scoreMap = predictionScoreRecords.at(previous);	// Get the map of current detection scores for the previous detections

		map<int, double>::iterator step;
		map<int, double>::iterator initial = scoreMap.begin();
		for (step = scoreMap.begin(); step != scoreMap.end(); step++)
		{
			if (initial->second < step->second)
			{
				initial = step;
			}
		}

		predictions.insert(pair<int, int>(previous, initial->first));
	}
}

// In this member function, we are trying to get the score generated for the current detection by the distance between the current and previous detection. But in order to do that, we will need the distances between the previous frame and all the current frames. We
// need the distances between all the current and the previous because the current detection's distance compared to the distances of all the other current detections will give us the score. I wanted to give more weight to the current detections that were closer, and
// less weight to the ones that were far away (obviously), but I didn't want it to be linear. So the equation I used is score = (1/2^i), were i is the element with the ith least distance.
double TrackingController::calcDetectionDist(int currDetection, int prevDetection)
{
	// Get the distances between the current frames and the previous frame, and save it in the distances vector (detection 0 will be element 0 in the distance vector)
	double one, two, distance;
	map<int, double> distances;
	for (int i = 0; i < controller.currentDetectionsSize(); i++)
	{
		one = pow(controller.getDetection(i).y - controller.getDetection(prevDetection, "previous").y, 2);
		two = pow(controller.getDetection(i).x - controller.getDetection(prevDetection, "previous").x, 2);
		distance = sqrt(one + two);
		distances.insert(pair<int, double>(i, distance));
	}

	// Now you want to find the index with the lowest score and put the index into the queue. Do this repeatedly until the queue has all the indices. 
	// It will form a queue where the bottom has the index with the least distance and the top has the index with the greatest distance 
	queue<int> indices;
	map<int, double>::iterator step;
	map<int, double>::iterator initial;
	int size = distances.size();
	for (int i = 0; i < size; i++)
	{
		initial = distances.begin();
		for (step = distances.begin(); step != distances.end(); step++)
		{
			if (initial->second > step->second)
			{
				initial = step;
			}
		}
		indices.push(initial->first);
		distances.erase(initial);
	}

	// Remove elements from the queue, until you get to the element which is the index of currDetection. Use i to generate the score.
	size = indices.size();
	for (int i = 0; i < size; i++)
	{
		if (indices.front() == currDetection)
		{
			return (1 / pow(2, i));
			break;
		}
		indices.pop();
	}
}

double TrackingController::calcSize(int currDetection, int prevDetection)
{
	// Calculate the size of the previous detection
	double prevDectectionSize = controller.getDetection(prevDetection, "previous").width * controller.getDetection(prevDetection, "previous").height;

	// Get the distances between the current frames and the previous frame, and save it in the distances vector (detection 0 will be element 0 in the distance vector)
	double area;
	map<int, double> sizes;
	for (int i = 0; i < controller.currentDetectionsSize(); i++)
	{
		area = controller.getDetection(i).width * controller.getDetection(i).height;
		sizes.insert(pair<int, double>(i, area));
	}

	// Now you want to find the index with the lowest score and put the index into the queue. Do this repeatedly until the queue has all the indices. 
	// It will form a queue where the bottom has the index with the least distance and the top has the index with the greatest distance 
	queue<int> indices;
	map<int, double>::iterator step;
	map<int, double>::iterator initial;
	int size = sizes.size();
	for (int i = 0; i < size; i++)
	{
		initial = sizes.begin();
		for (step = sizes.begin(); step != sizes.end(); step++)
		{
			if (abs(prevDectectionSize - step->second) < abs(prevDectectionSize - initial->second))
			{
				initial = step;
			}
		}
		indices.push(initial->first);
		sizes.erase(initial);
	}

	// Remove elements from the queue, until you get to the element which is the index of currDetection. Use i to generate the score.
	size = indices.size();
	for (int i = 0; i < size; i++)
	{
		if (indices.front() == currDetection)
		{
			return (1 / pow(2, i));
			break;
		}
		indices.pop();
	}
}



// FramesController class member function implementations



FramesController::FramesController()
{
}

bool FramesController::createNewFrame(frame data)
{
	try
	{
		if (current != nullptr)
		{
			if (previous != nullptr)
				delete previous;
			previous = current;
			current = data;
		}
		else
		{
			current = data;
		}
		currentFrameNum++;
	}
	catch (...)
	{
		return false;
	}
	return true;
}

Rect FramesController::getDetection(int detection, string type = "current")
{
	if (type == "previous")
		return previous->at(detection);
	else
		return current->at(detection);
}

frame FramesController::getCurrentFrameData()
{
	return current;
}

frame FramesController::getPreviousFrameData()
{
	return previous;
}

int FramesController::currentDetectionsSize()
{
	return current->size();
}

int FramesController::previousDetectionsSize()
{
	return previous->size();
}

FramesController::~FramesController()
{
	delete current;
	delete previous;
	currentFrameNum = 0;
}

bool FramesController::currentFrameExists()
{
	return current != nullptr ? true : false;
}

bool FramesController::previousFrameExists()
{
	return previous != nullptr ? true : false;
}
=======
>>>>>>> master
