#pragma once
#include <opencv2/opencv.hpp>

class Histogram1D
{
private:
	int histSize[1];
	float hranges[2];
	const float* ranges[1];
	int channels[1];
public:	
	Histogram1D();

	cv::Mat getHistogram(const cv::Mat& img);

	cv::Mat getHistogramImg(const cv::Mat& img);
};
