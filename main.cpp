#include <opencv2/opencv.hpp>
#include <iostream>
#include <time.h> 
using namespace cv;
using namespace std;

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

void detect_eel(Mat& input, Mat& output, int brightness, int color, int color_range)
{
	double detect_area = 0;
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
				//sum++;
			}
			data[j] = static_cast<uchar>(fixed_data);
		}
	}
	threshhold_img = channels[2];
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(threshhold_img, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
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
		//vector<Point> detect_contour;
		//copy(contours[max_contour].begin(), contours[max_contour].end(), detect_contour.begin());
		for (int i = 0; i < contours[max_contour].size()/2; i++)
		{
			for (int j = -((int)contours[max_contour].size() / 8) + 1; j < ((int)contours[max_contour].size() / 8); j++)
			{
				int k = ((i + contours[max_contour].size() / 2) + j)% contours[max_contour].size();
				double dist = calc_dist(contours[max_contour][i], contours[max_contour][k]);
				if (dist < min_dist)
				{
					min_dist = dist;
					minA = contours[max_contour][i];
					minB = contours[max_contour][k];
				}
			}
		}
		line(detect, minA, minB, Scalar(255, 0, 0), 2);
		putText(detect, to_string(min_dist), Point(50, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255), 2);
		putText(cam_img, to_string(detect_area / min_dist), Point(50, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255), 2);
		cout << "길이:" << detect_area / min_dist << "px" << endl;
	}

	cvtColor(threshhold_img, threshhold_img, COLOR_GRAY2BGR);
	drawContours(threshhold_img, contours, max_contour, Scalar(0, 0, 255), 1, 8, hierarchy, 0, Point());
	drawContours(detect, contours, max_contour, Scalar(0, 0, 255), 1, 8, hierarchy, 0, Point());
	//cout << cnt << endl;
	putText(threshhold_img, to_string(detect_area), Point(50, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255), 2);
	hconcat(cam_img, threshhold_img, output);
	hconcat(output, detect, output);
}

int main()
{
	int brightness_to_detect;
	int color_to_detect;
	int color_range;
	Mat img;
	Mat detect_img;
	Mat view_img;

	img = imread("noise.jpg");
	if (img.empty())
	{
		cout << "No img" << endl;
		return -1;
	}
	string brightness_trackbar_name = "감지할밝기";
	string color_trackbar_name = "감지할색상";
	string color_range_trackbar_name = "색상범위";
	namedWindow("detect", WINDOW_AUTOSIZE);
	createTrackbar(brightness_trackbar_name, "detect", 0, 255, on_trackbar);
	createTrackbar(color_trackbar_name, "detect", 0, 180, on_trackbar);
	createTrackbar(color_range_trackbar_name, "detect", 0, 90, on_trackbar);
	setTrackbarPos(brightness_trackbar_name, "detect", 233);
	while (1)
	{
		clock_t begin_t, end_t;
		begin_t = clock();

		brightness_to_detect = getTrackbarPos(brightness_trackbar_name, "detect");
		color_to_detect = getTrackbarPos(color_trackbar_name, "detect");
		color_range = getTrackbarPos(color_range_trackbar_name, "detect");
		detect_eel(img, detect_img, brightness_to_detect, color_to_detect, color_range);
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