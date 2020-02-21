#include <opencv2/opencv.hpp>
#include <iostream>
#include <time.h> 
#include <cmath>
//#include "Histogram1D.h"
using namespace cv;
using namespace std;

double px_to_cm_ratio = 1; // 1cm�� �� �ȼ����� �����ϴ� ����

double calc_dist(Point A, Point B)
// cv::Point ����ü�� �Ķ���ͷ� �޾Ƽ� �� cv::Point�� �Ÿ��� �����ϴ� �Լ�
{
	double dist; 
	int x_dist = A.x - B.x; // A�� B�� x���� �Ÿ����� 
	int y_dist = A.y - B.y; // A�� B�� y���� �Ÿ�����
	dist = x_dist * x_dist + y_dist * y_dist; // A�� B�� �Ÿ������� ������ ��
	return sqrt((double)dist); // A�� B�� �Ÿ��� return����

}

void on_trackbar(int, void*)
// cv::createTrackbar�� ���� �Լ�(��� ����)
{

}

void detect_eel(
	Mat& input, // �Էµ� �̹���
	Mat& output, // ����� ��� �̹���
	int brightness, // ������ ��� ���ΰ�
	int saturation // ������ ä�� ���ΰ�
	//Histogram1D& h // Histogram�� �̿��� ��� ������ ���� Ŭ����(���� ����� �߰����� ����)
)
// ����� ���̸� �����ϰ� ��� �̹����� ������ִ� �Լ�
{
	double length = 0; // ����� ���̰� ���� ����
	double detect_area = 0; // ����� ������ ���� ����
	Mat cam_img = input.clone(); // input�̹����� ������
	Mat threshold_img(input.size(), CV_8U); // input�̹����� ����ũ���� ����ִ� cv::Mat ����
	Mat detect(input.size(), CV_8UC3); // input�̹����� ����ũ���� ����ִ� cv::Mat ����
	detect = Scalar(0, 0, 0); // detect�� ������ �̹����� �ʱ�ȭ
	Mat hsv_img; // hsv������ ���� �����Ͱ� ����� cv::Mat ����
	cvtColor(cam_img, hsv_img, COLOR_BGR2HSV); 
	// hsv_img������ cam_img�� �����͸� hsv�������� ��ȯ�ؼ� ����

	vector<Mat> channels; // h, s, v �����͸� ���� ������ vector<Mat>����
	split(hsv_img, channels); // channels�� h, s, v �����͸� ���� ������


	uchar* fixed_data = (uchar*)threshold_img.data; // threshold_img�� �����ϱ� ���� ������
	uchar* data_s = (uchar*)channels[1].data; // channels[1] (ä�� ������)�� �����ϱ� ���� ������
	uchar* data_v = (uchar*)channels[2].data; // channels[2] (��� ������)�� �����ϱ� ���� ������
	int data_len = channels[1].rows * channels[1].cols; // �Է¹��� �̹����� ũ��

	for (int i = 0; i < data_len; i++)
	// hsv�����͸� �а� �̹������� ���� ����� ���󿵿��� ã�Ƴ��� for��
	{
		fixed_data[i] = data_s[i] < saturation || data_v[i] < brightness ? 255u : 0u;
		//���� ����� ���� ������ ���, �׷��� ���� ������ ���������� ����
	}

	vector<vector<Point>> contours; // threshold_img�� �������� ������ �Լ�
	vector<Vec4i> hierarchy;
	findContours(threshold_img, contours, hierarchy, 
		RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
	// threshold�� �������� contours�� ������
	int size = contours.size(); // contours�� ����� contour�� ������ ����ϴ� ����
	int max_contour = 0;
	if (size) // contour�� ������ �������� ����
	{
		int max_Area = 0; // ����ū contour�� ������ ����ϴ� ����
		for (int i = 0; i < size; i++) // contours���� ���� ������ ū contour�� ã�� for��
		{
			if (max_Area < contourArea(contours[i])) // ���� contour�� ������ max_Area����ũ��
			{
				max_Area = contourArea(contours[i]); // max_Area�� ���� contour�� ������ ����
				max_contour = i; //���� ������ ū contour�� ��ȣ�� ���
			}
		}
		detect_area = contourArea(contours[max_contour]); // ���� ������ ū contour�� ������ detect_area�� ����
		double min_dist = 10e+10; // ����� �β��� ���ϴ� �Լ�
		Point minA, minB; // contour�� cv::Point�� ������ ����
		
		for (int i = 0; i < contours[max_contour].size() >> 1; i++) // ���� ������ ū contour�� 0������ ���ݱ��� �ݺ�
			// i�� ����ū contour�� 0������ ���ݱ��� �ݺ�
		{
			for (int j = -((int)contours[max_contour].size() >>3) + 1;
				j < ((int)contours[max_contour].size() >> 3);
				j++)
				// j�� 0���� ����ū contourũ���� 1/8 ���� ����
			{
				int k = ((i + contours[max_contour].size() / 2) + j)
					% contours[max_contour].size();
				// k�� i�� �ݴ��������� �� contourũ���� 1/8 ����
				double dist = calc_dist(contours[max_contour][i], contours[max_contour][k]); 
				// ����ū contour�� i��°�� k��° ������ �Ÿ��� ����ϴ� ����
				if (dist < min_dist) // dist�� �ִܰŸ��̸�
				{
					min_dist = dist; // dist�� ����
					minA = contours[max_contour][i]; minB = contours[max_contour][k];
					// i�� k�� ��ġ�� ���
				}
			}
		}
		line(detect, minA, minB, Scalar(255, 0, 0), 2); //�����ص� �Ǵ� ����
		putText(detect, to_string(round(min_dist / px_to_cm_ratio * 10) / 10),
			Point(50, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255), 2);
		length = round(detect_area / min_dist / px_to_cm_ratio);
		putText(cam_img, to_string(length),
			Point(50, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(255, 0, 0), 2);
		cout << "����:" << detect_area / min_dist << "px" << endl;
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
	//Histogram1D h;
	int brightness_to_detect;
	int saturation_to_detect;
	//int color_to_detect;
	//int color_range;
	VideoCapture cap(0);
	Mat img;
	Mat detect_img;
	Mat view_img;

	if (!cap.isOpened())
	{
		cerr << "ī�޶� �� �� ����" << endl;
		return -1;
	}
	string brightness_trackbar_name = "�����ҹ��";
	string saturation_trackbar_name = "������ä��";
	namedWindow("detect", WINDOW_FREERATIO);
	createTrackbar(brightness_trackbar_name, "detect", 0, 255, on_trackbar);
	createTrackbar(saturation_trackbar_name, "detect", 0, 255, on_trackbar);
	setTrackbarPos(brightness_trackbar_name, "detect", 160);
	setTrackbarPos(saturation_trackbar_name, "detect", 160);
	while (1)
	{

		clock_t begin_t, end_t;
		begin_t = clock();

		cap.read(img);
		if (img.empty())
		{
			cerr << "�� ������ ĸ�ĵ�" << endl;
			break;
		}

		brightness_to_detect = getTrackbarPos(brightness_trackbar_name, "detect");
		saturation_to_detect = getTrackbarPos(saturation_trackbar_name, "detect");
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
		cout << "����ð�:" << (double)(end_t - begin_t) / CLOCKS_PER_SEC << endl;
	}
	return 0;
}