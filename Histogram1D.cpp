#include "Histogram1D.h"

Histogram1D::Histogram1D()
{
	histSize[0] = 256;
	hranges[0] = 0.0;
	hranges[1] = 256.0;
	ranges[0] = hranges;
	channels[0] = 0;
}

cv::Mat Histogram1D::getHistogram(const cv::Mat& img)
{
	cv::Mat hist;
	cv::calcHist(&img, 1, channels, cv::Mat(), hist, 1, histSize, ranges);
	return hist;
}

cv::Mat Histogram1D::getHistogramImg(const cv::Mat& img)
{

	cv::Mat hist = getHistogram(img);

	double maxVal = 0;
	double minVal = 0;
	minMaxLoc(hist, &minVal, &maxVal, 0, 0);

	cv::Mat histImg(histSize[0], histSize[0], CV_8U, cv::Scalar(255));

	int hpt = static_cast<int>(0.9, histSize[0]);
	int maxH = 0;
	int maxV = 0;

	for (int h = 0; h < 256; h++)
	{
		float binVal = hist.at<float>(h);
		int intensity = static_cast<int>(binVal * hpt / maxVal);
		line(histImg, cv::Point(h, histSize[0]),
			cv::Point(h, histSize[0] - intensity), cv::Scalar::all(0));

		if (intensity > maxH)
		{
			maxH = intensity;
			maxV = h;
		}

	}
	return histImg;
}
