#include <opencv2/opencv.hpp>
#include <iostream>
#include <time.h> 
using namespace cv;
using namespace std;

void on_trackbar(int, void*)
{

}

void detect_eel(Mat& input, Mat& output, int brightness)
{
	int sum = 0;
	Mat cam_img = input.clone();
	Mat threshhold_img;
	Mat detect(input.size(), CV_8UC3);
	detect = Scalar(0, 0, 0);
	Mat hsv_img;
	cvtColor(cam_img, hsv_img, COLOR_BGR2HSV);
	vector<Mat> channels;
	split(hsv_img, channels);
	
	for (int i = 0; i < hsv_img.rows; i++)
	{
		uchar* data = channels[2].ptr<uchar>(i);
		for (int j = 0; j < hsv_img.cols; j++)
		{
			int fixed_data = 0;
			if (data[j] < brightness)
			{
				fixed_data = 255;
				sum++;
			}
			data[j] = static_cast<uchar>(fixed_data);
		}
	}
	threshhold_img = channels[2];
	imshow("test", threshhold_img);
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(threshhold_img, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
	int cnt = 0;
	int max_contour = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		cnt++;
		if (contours[max_contour].size() < contours[i].size())
		{
			max_contour = i;
		}
	}
	
	for (vector<Point>::size_type i; i < contours[max_contour].size(); i++)
	{

	}

	drawContours(cam_img, contours, max_contour, Scalar(0, 0, 255), 1, 8, hierarchy, 0, Point());
	drawContours(detect, contours, max_contour, Scalar(0, 0, 255), 1, 8, hierarchy, 0, Point());
	//cout << cnt << endl;
	//cvtColor(detect, detect, COLOR_GRAY2BGR);
	putText(detect, to_string(sum), Point(50, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255), 2);

	hconcat(cam_img, detect, output);
}

int main()
{
	int brightness_to_detect;
	Mat img;
	Mat detect_img;
	Mat view_img;

	img = imread("test.jpg");
	if (img.empty())
	{
		cout << "No img" << endl;
		return -1;
	}

	namedWindow("detect", WINDOW_AUTOSIZE);
	createTrackbar("Brightness to detect", "detect", 0, 255, on_trackbar);
	while (1)
	{
		clock_t begin_t, end_t;
		begin_t = clock();

		brightness_to_detect = getTrackbarPos("Brightness to detect", "detect");
		detect_eel(img, detect_img, brightness_to_detect);
		imshow("detect", detect_img);
		if (waitKey(1) == 27)
		{
			break;
		}

		end_t = clock();
		cout << "실행시간:" << (double)(end_t - begin_t) / CLOCKS_PER_SEC << endl;
	}
	return 0;
}