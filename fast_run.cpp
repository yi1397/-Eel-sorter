#include <opencv2/opencv.hpp>
#include <iostream>
#include <time.h> 
#include <cmath>
using namespace cv;
using namespace std;

double px_to_cm_ratio = 21;

void detect_eel(
	Mat& input,
	int brightness,
	int saturation
)
{
	double length;
	double detect_area;
	Mat cam_img = input.clone();
	Mat threshold_img(input.size(), CV_8U);
	Mat hsv_img;
	cvtColor(cam_img, hsv_img, COLOR_BGR2HSV);
	vector<Mat> channels;
	split(hsv_img, channels);


	uchar* fixed_data = (uchar*)threshold_img.data;
	uchar* data_s = (uchar*)channels[1].data;
	uchar* data_v = (uchar*)channels[2].data;
	int data_len = channels[1].rows * channels[1].cols;

	for (int i = 0; i < data_len; i++)
	{
		fixed_data[i] = data_s[i] < saturation || data_v[i] < brightness ? 255u : 0u;
	}
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(threshold_img, contours, hierarchy,
		RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

	int size = contours.size();
	int max_contour = 0;

	for (int i = 0; i < size; i++)
	{

		int max_Area = 0;
		if (max_Area < contourArea(contours[i]))
		{
			max_Area = contourArea(contours[i]);
			max_contour = i;
		}
	}

	if (size)
	{
		detect_area = contourArea(contours[max_contour]);
		double min_dist = 10e+10;

		for (int i = 0; i < contours[max_contour].size() >> 1; i++) //1중 for문으로 변경해야함
		{
			for (int j = -((int)contours[max_contour].size() >> 3) + 1;
				j < ((int)contours[max_contour].size() >> 3);
				j++)
			{
				int k = ((i + contours[max_contour].size() >> 1) + j)
					% contours[max_contour].size();
				double dist = sqrt((double)(
					(contours[max_contour][i].x - contours[max_contour][k].x) *
					(contours[max_contour][i].x - contours[max_contour][k].x)+
					(contours[max_contour][i].y - contours[max_contour][k].y)*
					(contours[max_contour][i].y - contours[max_contour][k].y)));
				min_dist = dist < min_dist ? dist : min_dist;
			}
		}
		length = round(detect_area / min_dist / px_to_cm_ratio);

		cout << "길이:" << to_string(length) << "cm" << endl;
	}
}

int main()
{
	VideoCapture cap(0);
	Mat img;

	if (!cap.isOpened())
	{
		cerr << "카메라를 열 수 없음" << endl;
		return -1;
	}
	do
	{

		clock_t begin_t, end_t;
		begin_t = clock();

		cap.read(img);
		//img = imread("test_img/t.png");

		detect_eel(img, 
			160,
			160);

		end_t = clock();
		cout << "실행시간:" << (double)(end_t - begin_t) / CLOCKS_PER_SEC << endl;
	} while (1);
	return 0;
}