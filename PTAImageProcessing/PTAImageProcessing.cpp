#include "pch.h"
#include "PTAImageProcessing.h"
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>

namespace PTA_ImageProcessing
{
	PedestrianDetectOpenCV::PedestrianDetectOpenCV()
	{
		_hog = cv::HOGDescriptor(cv::Size(48, 96), cv::Size(16, 16), cv::Size(8, 8), cv::Size(8, 8),
			9, 1, -1, cv::HOGDescriptor::L2Hys, 0.2, true, cv::HOGDescriptor::DEFAULT_NLEVELS);
		_hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
	}

	PedestrianDetectOpenCV::PedestrianDetectOpenCV(PTADetectType detectType)
	{
		PedestrianDetectOpenCV();
		if (PTADetectType::DIALMER)
		{
			_hog.setSVMDetector(cv::HOGDescriptor::getDaimlerPeopleDetector());
			_daimlerFlag = true;
		}
	}

	PedestrianDetectOpenCV::PedestrianDetectOpenCV(cv::HOGDescriptor existing)
	{
		_hog = existing;
	}

	std::vector<cv::Rect> PedestrianDetectOpenCV::Detect(cv::Mat inImg)
	{
		std::vector<cv::Rect> found, found_filtered;
		
		cv::cvtColor(inImg, inImg, cv::COLOR_RGB2GRAY);
		cv::resize(inImg, inImg, cv::Size(), 0.45, 0.45, cv::INTER_LINEAR);

		if (_daimlerFlag)
			_hog.detectMultiScale(inImg, found, 0.85, cv::Size(), cv::Size(), 1.05, 2.5, false);
		else
			_hog.detectMultiScale(inImg, found, 1.0, cv::Size(8,8), 
				cv::Size(32, 32), 1.05, 2, false);

		for (size_t i = 0; i < found.size(); i++)
		{
			cv::Rect r = found[i];

			size_t j;
			// Do not add small detections inside a bigger detection.
			for (j = 0; j < found.size(); j++)
				if (j != i && (r & found[j]) == r)
					break;

			if (j == found.size())
				found_filtered.push_back(r);
		}
		return found_filtered;
	}
}