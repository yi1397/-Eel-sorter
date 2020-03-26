/*
	��� �����⿡ ���� ����Ʈ���� �Դϴ�.
	�����̴� ����� ���̸� ī�޶� ����ó���� �̿��Ͽ� ���������� �����ϴ� ���α׷��Դϴ�.

	��� ī�޶� : oCam-5CRO-U-M
*/


#define CAM_MODE NO_CAM_TEST

#define NO_CAM_TEST 0
#define CAM_320_240 320640
#define CAM_640_480 640480


#include <opencv2/opencv.hpp>
#include <iostream>
#include <time.h> 
#include <cmath>
//#include "Histogram1D.h"

#define brightness_trackbar_name "�����ҹ��"
#define saturation_trackbar_name "������ä��"
// trackbar�� �̸�

typedef struct _eel_data {
	bool success;
	float length;

	_eel_data(bool _success) : success(_success), length(0) {}

	_eel_data(bool _success, float _length) : success(_success), length(_length) {}


} eel_data;

float px_to_cm_ratio = 1; // 1cm�� �� �ȼ����� �����ϴ� ����


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


eel_data detect_eel(
	cv::Mat& input, // �Էµ� �̹���
	int brightness, // ������ ��� ���ΰ�
	int saturation // ������ ä�� ���ΰ�
	//Histogram1D& h // Histogram�� �̿��� ��� ������ ���� Ŭ����(���� ����� �߰����� ����)
)
// ����� ���̸� �����ϰ� ��� �̹����� ������ִ� �Լ�
{
	bool success = false;

	float length = 0; // ����� ���̰� ���� ����

	float detect_area = 0; // ����� ������ ���� ����

	cv::Mat threshold_img(input.size(), CV_8U); // input�̹����� ����ũ���� ����ִ� cv::Mat ����

	cv::Mat detect(input.size(), CV_8UC3); // input�̹����� ����ũ���� ����ִ� cv::Mat ����

	detect = cv::Scalar(0, 0, 0); // detect�� ������ �̹����� �ʱ�ȭ

	static cv::Mat hsv_img; // hsv������ ���� �����Ͱ� ����� cv::Mat ����

	cv::medianBlur(input, input, 3);

	cv::cvtColor(input, hsv_img, cv::COLOR_BGR2HSV);
	// hsv_img������ cam_img�� �����͸� hsv�������� ��ȯ�ؼ� ����

	static cv::Mat channels[3]; // h, s, v �����͸� ���� ������ vector<Mat>����
	cv::split(hsv_img, channels); // channels�� h, s, v �����͸� ���� ������


	uchar* fixed_data = (uchar*)threshold_img.data; // threshold_img�� �����ϱ� ���� ������
	uchar* data_s = (uchar*)channels[1].data; // channels[1] (ä�� ������)�� �����ϱ� ���� ������
	uchar* data_v = (uchar*)channels[2].data; // channels[2] (��� ������)�� �����ϱ� ���� ������
	int data_len = input.rows * input.cols; // �Է¹��� �̹����� ũ��

	for (int i = data_len; i--;)
		// hsv�����͸� �а� �̹������� ���� ����� ���󿵿��� ã�Ƴ��� for��
	{
		fixed_data[i] = data_s[i] < saturation || data_v[i] < brightness ? 255u : 0u;
		//���� ����� ���� ������ 255u, �׷��� ���� ������ 0u���� ����
	}

	std::vector<std::vector<cv::Point>> contours; // threshold_img�� �������� ������ ����
	std::vector<cv::Vec4i> hierarchy;

	cv::findContours(threshold_img, contours, hierarchy,
		cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	// threshold�� �������� contours�� ������

	int size = contours.size(); // contours�� ����� contour�� ������ ����ϴ� ����
	int max_contour = 0;

	if (size) // contour�� ������ �������� ����
	{
		success = true;

		for (int i = 0; i < size; i++) // contours���� ���� ������ ū contour�� ã�� for��
		{
			double area = cv::contourArea(contours[i]); // ���� contour�� ����

			if (detect_area < area) // ���� contour�� ������ max_Area����ũ��
			{
				detect_area = area; // max_Area�� ���� contour�� ������ ����
				max_contour = i; //���� ������ ū contour�� ��ȣ�� ���
			}
		}

		float min_dist = 10e+10; // ����� �β��� ����ϴ� ����

		cv::Point minA, minB; // contour�� cv::Point�� ������ ����

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

				if (dist < min_dist) // dist�� �ִܰŸ��̸�
				{
					min_dist = dist; // dist�� ����

					minA = contours[max_contour][i];
					minB = contours[max_contour][k];
					// i��° �������� k��° �������� ��ġ�� ���
				}
			}
		}
		min_dist = sqrt(min_dist);
		// ����� �β��� ���

		cv::line(detect, minA, minB, cv::Scalar(255, 0, 0), 2);
		// ��� �̹����� minA�� minB ���̸� ǥ������

		cv::putText(detect, std::to_string(round(min_dist / px_to_cm_ratio * 10) / 10),
			cv::Point(50, 50), cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(0, 0, 255), 2);
		// ��� �̹����� �β��� ǥ������

		length = round(detect_area / min_dist / px_to_cm_ratio);
		// ���̸� �����

		cv::putText(input, std::to_string(length),
			cv::Point(50, 50), cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 0, 0), 2);
		// ��� �̹����� ���̸� ǥ������

		std::cout << "����:" << length << "cm" << std::endl;
		// �ܼ�â�� ���̸� ���
	}

	cv::cvtColor(threshold_img, threshold_img, cv::COLOR_GRAY2BGR);
	// ��� �̹����� �÷��� ��ȯ

	cv::drawContours(threshold_img, contours, max_contour,
		cv::Scalar(255, 0, 0), 2, 8, hierarchy, 0, cv::Point());
	// ��� �̹����� ��� �������� ǥ������

	cv::drawContours(detect, contours, max_contour,
		cv::Scalar(0, 0, 255), 1, 8, hierarchy, 0, cv::Point());
	// ��� �̹����� ��� �������� ǥ������

	cv::putText(threshold_img,
		std::to_string(round(detect_area / px_to_cm_ratio / px_to_cm_ratio)),
		cv::Point(50, 50), cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(0, 0, 255), 2);
	// ��� �̹����� ����� ������ ǥ������

	cv::imshow("threshold_img", threshold_img);
	cv::imshow("input", input);
	cv::imshow("detect", detect);
	// �̹��� ���

	return eel_data(success, length);
}


int main()
{

	//Histogram1D h; // Histogram�� �̿��� ��� ������ ���� Ŭ����(���� ����� �߰����� ����)

#if CAP_MODE == NO_CAM_TEST
#pragma message("NO_USE_CAM")
	// ī�޶� ������

#else
#pragma message("USE_CAM")
	//ī�޶� ��� 

	cv::VideoCapture cap(1 + cv::CAP_DSHOW); //ī�޶� �ҷ���

	if (!cap.isOpened())
		//ī�޶� ���� ����
	{
		std::cerr << "ī�޶� �� �� ����" << std::endl;
		return -1;
	}

#if CAM_MODE == CAM_320_240
#pragma message("CAM_320_240")
	// 320*240

	std::cout << "CAM_MODE is 320*240" << std::endl;

	cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64FC1);
	cv::Mat distCoeffs = cv::Mat::zeros(1, 5, CV_64FC1);;

	cameraMatrix = (cv::Mat1d(3, 3) << 327.495, 0, 170.231, 0, 342.608, 120.341, 0, 0, 1);
	distCoeffs = (cv::Mat1d(1, 4) << -0.447434, 0.231295, 0.000754, -0.001325);
	// ī�޶� Ķ���극�̼��� ���� ������

	cap.set(cv::CAP_PROP_FRAME_WIDTH, 320); // ī�޶� ���� ���� ũ�� ����
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, 240); // ī�޶� ���� ���� ũ�� ����

#elif CAM_MODE == CAM_640_480
#pragma message("CAM_640_480")
	// 640*480

	std::cout << "CAM_MODE is 640*480" << std::endl;

	cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64FC1);
	cv::Mat distCoeffs = cv::Mat::zeros(1, 5, CV_64FC1);;

	cameraMatrix = (cv::Mat1d(3, 3) << 652.551, 0, 333.847, 0, 641.968, 234.218, 0, 0, 1);
	distCoeffs = (cv::Mat1d(1, 4) << -0.444277, 0.253481, 0.001623, 0.000861);
	// ī�޶� Ķ���극�̼��� ���� ������

	cap.set(cv::CAP_PROP_FRAME_WIDTH, 640); // ī�޶� ���� ���� ũ�� ����
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480); // ī�޶� ���� ���� ũ�� ����

#else
#pragma message("This CAM_MODE is not supported")
	// ���ǵ������� ī�޶� ���

	std::cerr << "This CAM_MODE is not supported" << std::endl;
	return -1;

#endif


#endif

	int brightness_to_detect; // ������ ��� ���ΰ�
	int saturation_to_detect; // ������ ä�� ���ΰ�


#if CAM_MODE != NO_CAM_TEST
	// ī�޶� ����

	double fpsWanted = 120; // ī�޶� ���� fps 
	if (!cap.set(cv::CAP_PROP_FPS, fpsWanted)) // fps ����
		std::cout << fpsWanted << "fps is not supported" << std::endl; // ���� �޽���

#endif

	cv::Mat img; // ī�޶� ������ ���� ����

	clock_t begin_t, end_t; // ���� �ð��� ����ϴ� ����

	cv::namedWindow("detect", cv::WINDOW_NORMAL);
	// ��� ������

		cv::createTrackbar(brightness_trackbar_name, "detect", 0, 255, on_trackbar);
	cv::createTrackbar(saturation_trackbar_name, "detect", 0, 255, on_trackbar);
	// �����쿡 trackbar�� ����

	cv::setTrackbarPos(brightness_trackbar_name, "detect", 160);
	cv::setTrackbarPos(saturation_trackbar_name, "detect", 160);
	// trackbar�� �⺻���� ����

#if CAM_MODE == NO_CAM_TEST	
	// ī�޶� �̻�� �׽�Ʈ

	img = cv::imread("test_img/test3.png");

	if (img.empty())
		// ���� �б� ����
	{
		std::cerr << "���� �б� ����" << std::endl;
		return -1;
	}

#endif

	while (1)
		// �ݺ�
	{
		begin_t = clock(); // ���� �ð� ���

#if CAM_MODE != NO_CAM_TEST
		// ī�޶� ����

		cap.read(img); // ������ ī�޶󿡼� �о��
		if (img.empty())
			// ���� �ν� ����(ī�޶� ���� ����)
		{
			std::cerr << "ī�޶� ���� ����" << std::endl;
			return -1;
		}

		{
			cv::Mat temp;
			cv::undistort(img, temp, cameraMatrix, distCoeffs);
			img = temp;
		}

#endif

		brightness_to_detect = cv::getTrackbarPos(brightness_trackbar_name, "detect");
		saturation_to_detect = cv::getTrackbarPos(saturation_trackbar_name, "detect");
		// trackbar���� ���� ������

		detect_eel(img, brightness_to_detect, saturation_to_detect);
		// �� �����ϴ� �Լ� ȣ��


		switch (cv::waitKeyEx(1)) // Ű���� �Է�üũ
		{
		case -1: // Ű �Է� ����
			break;

		case 27: // esc Ű �Է�
			std::cout << "�����" << std::endl;

			return 0;
			// ����

		case 2424832: // �� Ű �Է�
			cv::setTrackbarPos(brightness_trackbar_name, "detect",
				cv::getTrackbarPos(brightness_trackbar_name, "detect") - 1);
			// brightness_trackbar_name�� �� 1 ����

			break;

		case 2555904: // �� Ű �Է�
			cv::setTrackbarPos(brightness_trackbar_name, "detect",
				cv::getTrackbarPos(brightness_trackbar_name, "detect") + 1);
			// brightness_trackbar_name�� �� 1 ����

			break;

		case 2490368: // �� Ű �Է�
			cv::setTrackbarPos(saturation_trackbar_name, "detect",
				cv::getTrackbarPos(saturation_trackbar_name, "detect") + 1);
			// saturation_trackbar_name�� �� 1 ����

			break;

		case 2621440: // �� Ű �Է�
			cv::setTrackbarPos(saturation_trackbar_name, "detect",
				cv::getTrackbarPos(saturation_trackbar_name, "detect") - 1);
			// saturation_trackbar_name�� �� 1 ����

			break;
		}

		end_t = clock(); // ���� �ð� ���
		std::cout << "����ð�:" << end_t - begin_t << std::endl;
		// ����ð� ���
	}

	std::cout << "�����" << std::endl;
	return 0;
}