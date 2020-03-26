/*
	장어 선별기에 사용될 소프트웨어 입니다.
	움직이는 장어의 길이를 카메라 영상처리를 이용하여 순간적으로 측정하는 프로그램입니다.

	사용 카메라 : oCam-5CRO-U-M
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

#define brightness_trackbar_name "감지할밝기"
#define saturation_trackbar_name "감지할채도"
// trackbar의 이름

typedef struct _eel_data {
	bool success;
	float length;

	_eel_data(bool _success) : success(_success), length(0) {}

	_eel_data(bool _success, float _length) : success(_success), length(_length) {}


} eel_data;

float px_to_cm_ratio = 1; // 1cm가 몇 픽셀인지 저장하는 변수


inline int calc_dist(cv::Point& A, cv::Point& B)
// cv::Point 구조체를 파라미터로 받아서 두 cv::Point의 거리를 측정하는 함수
{
	int x_dist = A.x - B.x; // A와 B의 x방향 거리차이 
	int y_dist = A.y - B.y; // A와 B의 y방향 거리차이

	return x_dist * x_dist + y_dist * y_dist; // A와 B의 거리차이의 제곱의 합을 return해줌

}

void on_trackbar(int, void*)
// cv::createTrackbar를 위한 함수(기능 없음)
{

}


eel_data detect_eel(
	cv::Mat& input, // 입력된 이미지
	int brightness, // 감지할 밝기 문턱값
	int saturation // 감지할 채도 문턱값
	//Histogram1D& h // Histogram을 이용한 장어 감지를 위한 클래스(아직 기능을 추가하지 않음)
)
// 장어의 길이를 감지하고 결과 이미지를 출력해주는 함수
{
	bool success = false;

	float length = 0; // 장어의 길이가 기억될 변수

	float detect_area = 0; // 장어의 면적이 기억될 변수

	cv::Mat threshold_img(input.size(), CV_8U); // input이미지와 같은크기의 비어있는 cv::Mat 변수

	cv::Mat detect(input.size(), CV_8UC3); // input이미지와 같은크기의 비어있는 cv::Mat 변수

	detect = cv::Scalar(0, 0, 0); // detect를 검은색 이미지로 초기화

	static cv::Mat hsv_img; // hsv형식의 색상 데이터가 저장될 cv::Mat 변수

	cv::medianBlur(input, input, 3);

	cv::cvtColor(input, hsv_img, cv::COLOR_BGR2HSV);
	// hsv_img변수에 cam_img의 데이터를 hsv형식으로 변환해서 저장

	static cv::Mat channels[3]; // h, s, v 데이터를 각각 저장할 vector<Mat>선언
	cv::split(hsv_img, channels); // channels에 h, s, v 데이터를 각각 저장함


	uchar* fixed_data = (uchar*)threshold_img.data; // threshold_img에 접근하기 위한 포인터
	uchar* data_s = (uchar*)channels[1].data; // channels[1] (채도 데이터)에 접근하기 위한 포인터
	uchar* data_v = (uchar*)channels[2].data; // channels[2] (밝기 데이터)에 접근하기 위한 포인터
	int data_len = input.rows * input.cols; // 입력받은 이미지의 크기

	for (int i = data_len; i--;)
		// hsv데이터를 읽고 이미지에서 장어와 비슷한 색상영역을 찾아내는 for문
	{
		fixed_data[i] = data_s[i] < saturation || data_v[i] < brightness ? 255u : 0u;
		//장어와 비슷한 색상 영역은 255u, 그렇지 않은 영역은 0u으로 저장
	}

	std::vector<std::vector<cv::Point>> contours; // threshold_img의 윤곽선을 저장할 벡터
	std::vector<cv::Vec4i> hierarchy;

	cv::findContours(threshold_img, contours, hierarchy,
		cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	// threshold의 윤곽선을 contours에 저장함

	int size = contours.size(); // contours에 저장된 contour의 갯수를 기억하는 변수
	int max_contour = 0;

	if (size) // contour가 없으면 실행하지 않음
	{
		success = true;

		for (int i = 0; i < size; i++) // contours에서 가장 면적이 큰 contour를 찾는 for문
		{
			double area = cv::contourArea(contours[i]); // 현재 contour의 면적

			if (detect_area < area) // 현재 contour의 면적이 max_Area보다크면
			{
				detect_area = area; // max_Area에 현재 contour의 면적을 저장
				max_contour = i; //가장 면적이 큰 contour의 번호를 기억
			}
		}

		float min_dist = 10e+10; // 장어의 두께를 기억하는 변수

		cv::Point minA, minB; // contour의 cv::Point를 저장할 변수

		int contour_count = (int)contours[max_contour].size(); // contour의 꼭지점의 개수

		for (int i = contour_count >> 1; i; --i) // 가장 면적이 큰 contour의 1번부터 절반까지 반복
			// i는 가장큰 contour의 0번부터 절반까지 반복
		{
			for (int j = -(contour_count >> 3) + 1;
				j < (contour_count >> 3);
				++j)
				// j는 contour크기의 -1/8부터 가장큰 contour크기의 1/8 까지 증가
			{
				int k = (i + (contour_count >> 1) + j);
				k = k < contour_count ? k : k - contour_count;
				// k는 i의 반대지점에서 ± contour크기의 1/8 사이

				int dist = calc_dist(contours[max_contour][i], contours[max_contour][k]);
				// 가장큰 contour의 i번째와 k번째 사이의 거리를 기억하는 변수

				if (dist < min_dist) // dist가 최단거리이면
				{
					min_dist = dist; // dist를 저장

					minA = contours[max_contour][i];
					minB = contours[max_contour][k];
					// i번째 꼭지점과 k번째 꼭지점의 위치를 기억
				}
			}
		}
		min_dist = sqrt(min_dist);
		// 장어의 두께를 계산

		cv::line(detect, minA, minB, cv::Scalar(255, 0, 0), 2);
		// 결과 이미지에 minA와 minB 사이를 표시해줌

		cv::putText(detect, std::to_string(round(min_dist / px_to_cm_ratio * 10) / 10),
			cv::Point(50, 50), cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(0, 0, 255), 2);
		// 결과 이미지에 두께를 표시해줌

		length = round(detect_area / min_dist / px_to_cm_ratio);
		// 길이를 계산함

		cv::putText(input, std::to_string(length),
			cv::Point(50, 50), cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 0, 0), 2);
		// 결과 이미지에 길이를 표시해줌

		std::cout << "길이:" << length << "cm" << std::endl;
		// 콘솔창에 길이를 출력
	}

	cv::cvtColor(threshold_img, threshold_img, cv::COLOR_GRAY2BGR);
	// 결과 이미지를 컬러로 변환

	cv::drawContours(threshold_img, contours, max_contour,
		cv::Scalar(255, 0, 0), 2, 8, hierarchy, 0, cv::Point());
	// 결과 이미지에 장어 윤곽선을 표시해줌

	cv::drawContours(detect, contours, max_contour,
		cv::Scalar(0, 0, 255), 1, 8, hierarchy, 0, cv::Point());
	// 결과 이미지에 장어 윤곽선을 표시해줌

	cv::putText(threshold_img,
		std::to_string(round(detect_area / px_to_cm_ratio / px_to_cm_ratio)),
		cv::Point(50, 50), cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(0, 0, 255), 2);
	// 결과 이미지에 장어의 면적을 표시해줌

	cv::imshow("threshold_img", threshold_img);
	cv::imshow("input", input);
	cv::imshow("detect", detect);
	// 이미지 출력

	return eel_data(success, length);
}


int main()
{

	//Histogram1D h; // Histogram을 이용한 장어 감지를 위한 클래스(아직 기능을 추가하지 않음)

#if CAP_MODE == NO_CAM_TEST
#pragma message("NO_USE_CAM")
	// 카메라 사용안함

#else
#pragma message("USE_CAM")
	//카메라 사용 

	cv::VideoCapture cap(1 + cv::CAP_DSHOW); //카메라를 불러옴

	if (!cap.isOpened())
		//카메라 연결 실패
	{
		std::cerr << "카메라를 열 수 없음" << std::endl;
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
	// 카메라 캘리브레이션을 위한 데이터

	cap.set(cv::CAP_PROP_FRAME_WIDTH, 320); // 카메라 영상 가로 크기 설정
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, 240); // 카메라 영상 세로 크기 설정

#elif CAM_MODE == CAM_640_480
#pragma message("CAM_640_480")
	// 640*480

	std::cout << "CAM_MODE is 640*480" << std::endl;

	cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64FC1);
	cv::Mat distCoeffs = cv::Mat::zeros(1, 5, CV_64FC1);;

	cameraMatrix = (cv::Mat1d(3, 3) << 652.551, 0, 333.847, 0, 641.968, 234.218, 0, 0, 1);
	distCoeffs = (cv::Mat1d(1, 4) << -0.444277, 0.253481, 0.001623, 0.000861);
	// 카메라 캘리브레이션을 위한 데이터

	cap.set(cv::CAP_PROP_FRAME_WIDTH, 640); // 카메라 영상 가로 크기 설정
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480); // 카메라 영상 세로 크기 설정

#else
#pragma message("This CAM_MODE is not supported")
	// 정의되지않은 카메라 모드

	std::cerr << "This CAM_MODE is not supported" << std::endl;
	return -1;

#endif


#endif

	int brightness_to_detect; // 감지할 밝기 문턱값
	int saturation_to_detect; // 감지할 채도 문턱값


#if CAM_MODE != NO_CAM_TEST
	// 카메라 설정

	double fpsWanted = 120; // 카메라 영상 fps 
	if (!cap.set(cv::CAP_PROP_FPS, fpsWanted)) // fps 설정
		std::cout << fpsWanted << "fps is not supported" << std::endl; // 예외 메시지

#endif

	cv::Mat img; // 카메라 영상이 기억될 변수

	clock_t begin_t, end_t; // 실행 시간을 기억하는 변수

	cv::namedWindow("detect", cv::WINDOW_NORMAL);
	// 출력 윈도우

		cv::createTrackbar(brightness_trackbar_name, "detect", 0, 255, on_trackbar);
	cv::createTrackbar(saturation_trackbar_name, "detect", 0, 255, on_trackbar);
	// 윈도우에 trackbar를 만듬

	cv::setTrackbarPos(brightness_trackbar_name, "detect", 160);
	cv::setTrackbarPos(saturation_trackbar_name, "detect", 160);
	// trackbar의 기본값을 설정

#if CAM_MODE == NO_CAM_TEST	
	// 카메라 미사용 테스트

	img = cv::imread("test_img/test3.png");

	if (img.empty())
		// 파일 읽기 실패
	{
		std::cerr << "파일 읽기 실패" << std::endl;
		return -1;
	}

#endif

	while (1)
		// 반복
	{
		begin_t = clock(); // 시작 시간 기억

#if CAM_MODE != NO_CAM_TEST
		// 카메라 사용시

		cap.read(img); // 영상을 카메라에서 읽어옴
		if (img.empty())
			// 영상 인식 실패(카메라 연결 끊김)
		{
			std::cerr << "카메라 연결 끊김" << std::endl;
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
		// trackbar에서 값을 가져옴

		detect_eel(img, brightness_to_detect, saturation_to_detect);
		// 장어를 감지하는 함수 호출


		switch (cv::waitKeyEx(1)) // 키보드 입력체크
		{
		case -1: // 키 입력 없음
			break;

		case 27: // esc 키 입력
			std::cout << "종료됨" << std::endl;

			return 0;
			// 종료

		case 2424832: // ← 키 입력
			cv::setTrackbarPos(brightness_trackbar_name, "detect",
				cv::getTrackbarPos(brightness_trackbar_name, "detect") - 1);
			// brightness_trackbar_name의 값 1 감소

			break;

		case 2555904: // → 키 입력
			cv::setTrackbarPos(brightness_trackbar_name, "detect",
				cv::getTrackbarPos(brightness_trackbar_name, "detect") + 1);
			// brightness_trackbar_name의 값 1 증가

			break;

		case 2490368: // ↑ 키 입력
			cv::setTrackbarPos(saturation_trackbar_name, "detect",
				cv::getTrackbarPos(saturation_trackbar_name, "detect") + 1);
			// saturation_trackbar_name의 값 1 증가

			break;

		case 2621440: // ↓ 키 입력
			cv::setTrackbarPos(saturation_trackbar_name, "detect",
				cv::getTrackbarPos(saturation_trackbar_name, "detect") - 1);
			// saturation_trackbar_name의 값 1 감소

			break;
		}

		end_t = clock(); // 실행 시간 기억
		std::cout << "실행시간:" << end_t - begin_t << std::endl;
		// 실행시간 출력
	}

	std::cout << "종료됨" << std::endl;
	return 0;
}