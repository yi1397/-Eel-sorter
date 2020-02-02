#include <opencv2/opencv.hpp>
#include <iostream>
using namespace cv;
using namespace std;

void on_trackbar(int, void*)
{

}

void detect_eel(Mat input, Mat& output, int brightness)
{
	int sum = 0;
	Mat detect;
	Mat mask;
	Mat hsv_img;
	cvtColor(input, hsv_img, COLOR_BGR2HSV);
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
			//double fixed_data = pow(static_cast<double>(data[j]) / 255.0, 1 / exp(value)) * 255;
			data[j] = static_cast<uchar>(fixed_data);
		}
	}
	detect = channels[2];
	Canny(detect, detect, 1, 1, 3);
	//bitwise_xor(input, mask, input);
	cvtColor(detect, detect, COLOR_GRAY2BGR);
	putText(detect, to_string(sum), Point(50, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255), 2);

	hconcat(input, detect, output);
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
		brightness_to_detect = getTrackbarPos("Brightness to detect", "detect");
		detect_eel(img, detect_img, brightness_to_detect);
		imshow("detect", detect_img);
		if (waitKey(1) == 27)
		{
			break;
		}
		cout << "1" << endl;
	}
	return 0;
}