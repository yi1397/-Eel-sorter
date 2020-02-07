#include <opencv2/opencv.hpp>
#include <iostream>
#include <time.h> 
#include <cmath>
using namespace cv;
using namespace std;

double px_to_cm_ratio = 20.94383;

void thinning(cv::Mat& img)
{
	cv::Mat skel(img.size(), CV_8UC1, cv::Scalar(0));
	cv::Mat temp(img.size(), CV_8UC1);
	cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
	bool done;
	do
	{
		cv::morphologyEx(img, temp, cv::MORPH_OPEN, element);
		cv::bitwise_not(temp, temp);
		cv::bitwise_and(img, temp, temp);
		cv::bitwise_or(skel, temp, skel);
		cv::erode(img, img, element);

		double max;
		cv::minMaxLoc(img, 0, &max);
		done = (max == 0);
	} while (!done);
	img = skel;
}

double calc_dist(Point A, Point B)
{
	double dist;
	int x_dist = A.x - B.x;
	int y_dist = A.y - B.y;
	dist = x_dist * x_dist + y_dist * y_dist;
	return sqrt((double)dist);

}

void on_trackbar(int, void*)
{

}

void detect_eel(
	Mat& input, 
	Mat& output, 
	int brightness, 
	int saturation
)
{
	double length = 0;
	double detect_area = 0;
	Mat cam_img = input.clone();
	Mat threshold_img(input.size(), CV_8U);
	Mat detect(input.size(), CV_8UC3);
	detect = Scalar(0, 0, 0);
	Mat hsv_img;
	cvtColor(cam_img, hsv_img, COLOR_BGR2HSV);
	vector<Mat> channels;
	split(hsv_img, channels);
	for (int i = 0; i < hsv_img.rows; i++)
	{
		uchar* fixed_data = threshold_img.ptr<uchar>(i);	//채도
		uchar* data_s = channels[1].ptr<uchar>(i);	//채도
		uchar* data_v = channels[2].ptr<uchar>(i);	//밝기
		for (int j = 0; j < hsv_img.cols; j++)
		{
			int fix_value = 0;
			if (data_s[j] < saturation || data_v[j] < brightness)
			{
				fix_value = 255;
				//sum++;
			}
			fixed_data[j] = static_cast<uchar>(fix_value);
		}
	}
	//Mat test = threshold_img.clone();
	//thinning(test);
	//imshow("test", test);
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(threshold_img, contours, hierarchy, 
		RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
	int cnt = 0;
	int max_contour = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		cnt++;
		if (contourArea(contours[max_contour]) < contourArea(contours[i]))
		{
			max_contour = i;
		}
		//cout << "i:" << i << endl;
		//cout << "size:" << contours[i].size() << endl;
	}
	//cout << "max_contour:" << max_contour << endl;
	if (cnt != 0)
	{
		detect_area = contourArea(contours[max_contour]);
		double min_dist = 10e+10;
		Point minA;
		Point minB;
		
		for (int i = 0; i < contours[max_contour].size()/2; i++)
		{
			for (int j = -((int)contours[max_contour].size() / 8) + 1;
				j < ((int)contours[max_contour].size() / 8);
				j++)
			{
				int k = ((i + contours[max_contour].size() / 2) + j)
					% contours[max_contour].size();
				double dist = calc_dist(contours[max_contour][i], 
					contours[max_contour][k]);
				if (dist < min_dist)
				{
					min_dist = dist;
					minA = contours[max_contour][i];
					minB = contours[max_contour][k];
				}
			}
		}
		line(detect, minA, minB, Scalar(255, 0, 0), 2);
		putText(detect, to_string(round(min_dist / px_to_cm_ratio * 10) / 10),
			Point(50, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255), 2);
		length = round(detect_area / min_dist / px_to_cm_ratio);
		putText(cam_img, to_string(length),
			Point(50, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(255, 0, 0), 2);
		cout << "길이:" << detect_area / min_dist << "px" << endl;
	}

	cvtColor(threshold_img, threshold_img, COLOR_GRAY2BGR);
	drawContours(threshold_img, contours, max_contour, 
		Scalar(255, 0, 0), 2, 8, hierarchy, 0, Point());
	drawContours(detect, contours, max_contour, 
		Scalar(0, 0, 255), 1, 8, hierarchy, 0, Point());
	//cout << cnt << endl;
	putText(threshold_img, 
		to_string(round(detect_area / px_to_cm_ratio / px_to_cm_ratio)), 
		Point(50, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255), 2);
	hconcat(cam_img, threshold_img, output);
	hconcat(output, detect, output);
}

int main()
{
	int brightness_to_detect;
	int saturation_to_detect;
	//int color_to_detect;
	//int color_range;
	Mat img;
	Mat detect_img;
	Mat view_img;

	img = imread("test_img/t.png");
	if (img.empty())
	{
		cerr << "No img" << endl;
		return -1;
	}
	string brightness_trackbar_name = "감지할밝기";
	string saturation_trackbar_name = "감지할채도";
	//string color_trackbar_name = "감지할색상";
	//string color_range_trackbar_name = "색상범위";
	namedWindow("detect", WINDOW_AUTOSIZE);
	createTrackbar(brightness_trackbar_name, "detect", 0, 255, on_trackbar);
	createTrackbar(saturation_trackbar_name, "detect", 0, 255, on_trackbar);
	//createTrackbar(color_trackbar_name, "detect", 0, 180, on_trackbar);
	//createTrackbar(color_range_trackbar_name, "detect", 0, 90, on_trackbar);
	setTrackbarPos(brightness_trackbar_name, "detect", 160);
	setTrackbarPos(saturation_trackbar_name, "detect", 160);
	while (1)
	{
		clock_t begin_t, end_t;
		begin_t = clock();

		brightness_to_detect = getTrackbarPos(brightness_trackbar_name, "detect");
		saturation_to_detect = getTrackbarPos(saturation_trackbar_name, "detect");
		//color_to_detect = getTrackbarPos(color_trackbar_name, "detect");
		//color_range = getTrackbarPos(color_range_trackbar_name, "detect");
		detect_eel(img, detect_img, brightness_to_detect, saturation_to_detect);
		imshow("detect", detect_img);
		switch (waitKeyEx(1))
		{
		case 27:
			return 0;
		case 2424832:
			setTrackbarPos(brightness_trackbar_name, "detect", 
				getTrackbarPos(brightness_trackbar_name, "detect") - 1);
			break;
		case 2555904:
			setTrackbarPos(brightness_trackbar_name, "detect", 
				getTrackbarPos(brightness_trackbar_name, "detect") + 1);
			break;
		case 2490368:
			setTrackbarPos(saturation_trackbar_name, "detect",
				getTrackbarPos(saturation_trackbar_name, "detect") + 1);
			break;
		case 2621440:
			setTrackbarPos(saturation_trackbar_name, "detect",
				getTrackbarPos(saturation_trackbar_name, "detect") - 1);
			break;
		}

		end_t = clock();
		cout << "실행시간:" << (double)(end_t - begin_t) / CLOCKS_PER_SEC << endl;
	}
	return 0;
}