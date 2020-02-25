#define brightness_trackbar_name "�����ҹ��"
#define saturation_trackbar_name "������ä��"
// trackbar�� �̸�

#include <opencv2/opencv.hpp>
#include <iostream>
#include <time.h> 
#include <cmath>
//#include "Histogram1D.h"
using namespace cv;
using namespace std;

double px_to_cm_ratio = 21; // 1cm�� �� �ȼ����� �����ϴ� ����

inline int calc_dist(Point A, Point B)
// cv::Point ����ü�� �Ķ���ͷ� �޾Ƽ� �� cv::Point�� �Ÿ��� �����ϴ� �Լ�
{
	int x_dist = A.x - B.x; // A�� B�� x���� �Ÿ����� 
	int y_dist = A.y - B.y; // A�� B�� y���� �Ÿ�����

	return x_dist * x_dist + y_dist * y_dist; // A�� B�� �Ÿ������� ������ ���� return����

}

void on_trackbar(int, void*)
// cv::createTrackbar�� ���� �Լ�(��� ����)
{

}

void detect_eel(
	Mat& input, // �Էµ� �̹���
	int brightness, // ������ ��� ���ΰ�
	int saturation // ������ ä�� ���ΰ�
	//Histogram1D& h // Histogram�� �̿��� ��� ������ ���� Ŭ����(���� ����� �߰����� ����)
)
// ����� ���̸� �����ϰ� ��� �̹����� ������ִ� �Լ�
{
	double length = 0; // ����� ���̰� ���� ����

	double detect_area = 0; // ����� ������ ���� ����

	Mat threshold_img(input.size(), CV_8U); // input�̹����� ����ũ���� ����ִ� cv::Mat ����

	Mat detect(input.size(), CV_8UC3); // input�̹����� ����ũ���� ����ִ� cv::Mat ����

	detect = Scalar(0, 0, 0); // detect�� ������ �̹����� �ʱ�ȭ

	Mat hsv_img; // hsv������ ���� �����Ͱ� ����� cv::Mat ����

	cvtColor(input, hsv_img, COLOR_BGR2HSV);
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
			int area = contourArea(contours[i]);
			if (max_Area < area) // ���� contour�� ������ max_Area����ũ��
			{
				max_Area = area; // max_Area�� ���� contour�� ������ ����
				max_contour = i; //���� ������ ū contour�� ��ȣ�� ���
			}
		}
		detect_area = max_Area; // ���� ������ ū contour�� ������ detect_area�� ����

		double min_dist = 10e+10; // ����� �β��� ����ϴ� ����

		Point minA, minB; // contour�� cv::Point�� ������ ����
		int contour_size = (int)contours[max_contour].size();
		for (int i = contour_size >> 1; i; --i) // ���� ������ ū contour�� 1������ ���ݱ��� �ݺ�
			// i�� ����ū contour�� 0������ ���ݱ��� �ݺ�
		{
			for (int j = -(contour_size >> 3) + 1;
				j < (contour_size >> 3);
				++j)
				// j�� contourũ���� -1/8���� ����ū contourũ���� 1/8 ���� ����
			{
				int k = (i + (contour_size >> 1) + j);
				k = k < contour_size ? k : k - contour_size;
				// k�� i�� �ݴ��������� �� contourũ���� 1/8 ����

				int dist = calc_dist(contours[max_contour][i], contours[max_contour][k]); 
				// ����ū contour�� i��°�� k��° ������ �Ÿ��� ����ϴ� ����

				if (dist < min_dist) // dist�� �ִܰŸ��̸�
				{
					min_dist = dist; // dist�� ����

					minA = contours[max_contour][i];
					minB = contours[max_contour][k];
					// i�� k�� ��ġ�� ���
				}
			}
		}
		min_dist = sqrt(min_dist);

		line(detect, minA, minB, Scalar(255, 0, 0), 2);
		// ��� �̹����� minA�� minB ���̸� ǥ������

		putText(detect, to_string(round(min_dist / px_to_cm_ratio * 10) / 10),
			Point(50, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255), 2);
		// ��� �̹����� �β��� ǥ������

		length = round(detect_area / min_dist / px_to_cm_ratio);
		// ���̸� �����

		putText(input, to_string(length),
			Point(50, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(255, 0, 0), 2);
		// ��� �̹����� ���̸� ǥ������

		cout << "����:" << detect_area / min_dist << "px" << endl;
		// �ܼ�â�� ���̸� ���
	}

	cvtColor(threshold_img, threshold_img, COLOR_GRAY2BGR);
	// ��� �̹����� �÷��� ��ȯ

	drawContours(threshold_img, contours, max_contour, 
		Scalar(255, 0, 0), 2, 8, hierarchy, 0, Point());
	// ��� �̹����� ��� �������� ǥ������

	drawContours(detect, contours, max_contour, 
		Scalar(0, 0, 255), 1, 8, hierarchy, 0, Point());
	// ��� �̹����� ��� �������� ǥ������

	putText(threshold_img, 
		to_string(round(detect_area / px_to_cm_ratio / px_to_cm_ratio)), 
		Point(50, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255), 2);
	// ��� �̹����� ����� ������ ǥ������

	hconcat(threshold_img, detect, detect);
	hconcat(input, detect, detect);

	imshow("detect", detect);
	// �̹��� ���
}

int main()
{
	//Histogram1D h; // Histogram�� �̿��� ��� ������ ���� Ŭ����(���� ����� �߰����� ����)

	int brightness_to_detect; // ������ ��� ���ΰ�
	int saturation_to_detect; // ������ ä�� ���ΰ�

	VideoCapture cap(0); //ī�޶� �ҷ���

	Mat img;

	if (!cap.isOpened())
		//ī�޶� ���� ����
	{
		cerr << "ī�޶� �� �� ����" << endl;
		return -1;
	}

	namedWindow("detect", WINDOW_FREERATIO);
	// ��� ������

	createTrackbar(brightness_trackbar_name, "detect", 0, 255, on_trackbar);
	createTrackbar(saturation_trackbar_name, "detect", 0, 255, on_trackbar);
	// �����쿡 trackbar�� ����

	setTrackbarPos(brightness_trackbar_name, "detect", 160);
	setTrackbarPos(saturation_trackbar_name, "detect", 160);
	// trackbar�� �⺻���� ����

	while (1)
		// �ݺ�
	{

		clock_t begin_t, end_t; // ���� �ð��� ����ϴ� ����
		begin_t = clock(); // ���� �ð� ���

		// cap.read(img); // ������ ī�޶󿡼� �о��
		img = imread("test_img/t.png");
		
		if (img.empty())
			// ���� �ν� ����
		{
			cerr << "�� ������ ĸ�ĵ�" << endl;
			break;
		}

		brightness_to_detect = getTrackbarPos(brightness_trackbar_name, "detect");
		saturation_to_detect = getTrackbarPos(saturation_trackbar_name, "detect");
		// trackbar���� ���� ������

		detect_eel(img, brightness_to_detect, saturation_to_detect);
		// �� �����ϴ� �Լ� ȣ��


		switch (waitKeyEx(1)) // Ű���� �Է�
		{
		case 27: // esc Ű �Է�
			return 0;
			// ����

		case 2424832: // �� Ű �Է�
			setTrackbarPos(brightness_trackbar_name, "detect", 
				getTrackbarPos(brightness_trackbar_name, "detect") - 1);
			// brightness_trackbar_name�� �� 1 ����

			break;

		case 2555904: // �� Ű �Է�
			setTrackbarPos(brightness_trackbar_name, "detect", 
				getTrackbarPos(brightness_trackbar_name, "detect") + 1);
			// brightness_trackbar_name�� �� 1 ����

			break;

		case 2490368: // �� Ű �Է�
			setTrackbarPos(saturation_trackbar_name, "detect",
				getTrackbarPos(saturation_trackbar_name, "detect") + 1);
			// saturation_trackbar_name�� �� 1 ����

			break;

		case 2621440: // �� Ű �Է�
			setTrackbarPos(saturation_trackbar_name, "detect",
				getTrackbarPos(saturation_trackbar_name, "detect") - 1);
			// saturation_trackbar_name�� �� 1 ����

			break;
		}

		end_t = clock(); // 

		cout << "����ð�:" << (double)(end_t - begin_t) / CLOCKS_PER_SEC << endl;
	}
	return 0;
}