#define brightness_trackbar_name "�����ҹ��"
#define saturation_trackbar_name "������ä��"
// trackbar�� �̸�

#include <opencv2/opencv.hpp>
#include <iostream>
#include <time.h> 
#include <cmath>
//#include "Histogram1D.h"

inline int calc_dist(cv::Point& A, cv::Point& B)
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
	cv::Mat& input, // �Էµ� �̹���
	int brightness, // ������ ��� ���ΰ�
	int saturation // ������ ä�� ���ΰ�
	//Histogram1D& h // Histogram�� �̿��� ��� ������ ���� Ŭ����(���� ����� �߰����� ����)
)
// ����� ���̸� �����ϰ� ��� �̹����� ������ִ� �Լ�
{
	float length = 0; // ����� ���̰� ���� ����

	float detect_area = 0; // ����� ������ ���� ����

	cv::Mat threshold_img(input.size(), CV_8U); // input�̹����� ����ũ���� ����ִ� cv::Mat ����


	cvtColor(input, input, cv::COLOR_BGR2HSV);
	// hsv_img������ cam_img�� �����͸� hsv�������� ��ȯ�ؼ� ����

	cv::Mat channels[3]; // h, s, v �����͸� ���� ������ vector<Mat>����
	split(input, channels); // channels�� h, s, v �����͸� ���� ������


	uchar* fixed_data = (uchar*)threshold_img.data; // threshold_img�� �����ϱ� ���� ������
	uchar* data_s = (uchar*)channels[1].data; // channels[1] (ä�� ������)�� �����ϱ� ���� ������
	uchar* data_v = (uchar*)channels[2].data; // channels[2] (��� ������)�� �����ϱ� ���� ������
	int data_len = channels[1].rows * channels[1].cols; // �Է¹��� �̹����� ũ��

	for (int i = 0; i < data_len; i++)
		// hsv�����͸� �а� �̹������� ���� ����� ���󿵿��� ã�Ƴ��� for��
		fixed_data[i] = data_s[i] < saturation || data_v[i] < brightness ? 255u : 0u;
		//���� ����� ���� ������ 255u, �׷��� ���� ������ 0u���� ����

	std::vector<std::vector<cv::Point>> contours; // threshold_img�� �������� ������ ����
	std::vector<cv::Vec4i> hierarchy;

	cv::findContours(threshold_img, contours, hierarchy,
		cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	// threshold�� �������� contours�� ������

	int size = contours.size(); // contours�� ����� contour�� ������ ����ϴ� ����
	int max_contour = 0;

	if (size) // contour�� ������ �������� ����
	{
		int max_Area = 0; // ����ū contour�� ������ ����ϴ� ����

		for (int i = 0; i < size; i++) // contours���� ���� ������ ū contour�� ã�� for��
		{
			int area = cv::contourArea(contours[i]); // ���� contour�� ����

			if (max_Area < area) // ���� contour�� ������ max_Area����ũ��
			{
				max_Area = area; // max_Area�� ���� contour�� ������ ����
				max_contour = i; //���� ������ ū contour�� ��ȣ�� ���
			}
		}
		detect_area = max_Area; // ���� ������ ū contour�� ������ detect_area�� ����

		float min_dist = 10e+10; // ����� �β��� ����ϴ� ����

		int contour_count = (int)contours[max_contour].size(); // contour�� �������� ����

		for (int i = contour_count >> 1; i; --i) // ���� ������ ū contour�� 1������ ���ݱ��� �ݺ�
			// i�� ����ū contour�� 0������ ���ݱ��� �ݺ�
		{
			for (int j = -(contour_count >> 3) + 1;
				j < (contour_count >> 3);
				++j)
				// j�� contourũ���� -1/8���� ����ū contourũ���� 1/8 ���� ����
			{
				int k = (i + (contour_count >> 1) + j);
				k = k < contour_count ? k : k - contour_count;
				// k�� i�� �ݴ��������� �� contourũ���� 1/8 ����

				int dist = calc_dist(contours[max_contour][i], contours[max_contour][k]);
				// ����ū contour�� i��°�� k��° ������ �Ÿ��� ����ϴ� ����

				min_dist = dist < min_dist ? dist : min_dist;
			}
		}

		std::cout << "����:" << round(detect_area / sqrt(min_dist)) << "px" << std::endl;
		// �ܼ�â�� ���̸� ���
	}
}

int main()
{
	//Histogram1D h; // Histogram�� �̿��� ��� ������ ���� Ŭ����(���� ����� �߰����� ����)

	cv::VideoCapture cap(1 + cv::CAP_DSHOW); //ī�޶� �ҷ���

	cv::Mat img;

	clock_t begin_t, end_t; // ���� �ð��� ����ϴ� ����

	cap.set(cv::CAP_PROP_FRAME_WIDTH, 640); // ī�޶� ���� ���� ũ�� ����
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480); // ī�޶� ���� ���� ũ�� ����

	cap.set(cv::CAP_PROP_FPS, 120);


	while (1)
		// �ݺ�
	{
		begin_t = clock(); // ���� �ð� ���

		cap.read(img); // ������ ī�޶󿡼� �о��
		//img = cv::imread("test_img/t.png");

		if (img.empty())
			// ���� �ν� ����
		{
			std::cerr << "�� ������ ĸ�ĵ�" << std::endl;
			break;
		}

		detect_eel(img,	160, 160);
		// �� �����ϴ� �Լ� ȣ��


		end_t = clock(); // ���� �ð� ���

		std::cout << "����ð�:" << (float)(end_t - begin_t) / CLOCKS_PER_SEC << std::endl;
		// ����ð� ���
	}
	return 0;
}