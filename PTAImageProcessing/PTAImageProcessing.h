#pragma once
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>

namespace PTA_ImageProcessing
{
	typedef enum {
		DEFAULT,
		DIALMER
	} PTADetectType;

	/** PedestrianDetectOpenCV
	 * @description - 
	 */
	class PedestrianDetectOpenCV
	{
	private:
		cv::HOGDescriptor _hog;
		bool _daimlerFlag = false;
	public:
		PedestrianDetectOpenCV();
		// @param - type of detector to use
		// @param - whether or not to use background detector
		PedestrianDetectOpenCV(PTADetectType);
		// @param - existing hog descriptor
		PedestrianDetectOpenCV(cv::HOGDescriptor);
		// @param - inImage
		std::vector<cv::Rect> Detect(cv::Mat);
	};
}