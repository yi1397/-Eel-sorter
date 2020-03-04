#define brightness_trackbar_name "감지할밝기"
#define saturation_trackbar_name "감지할채도"
// trackbar의 이름

#include <opencv2/opencv.hpp>
#include <iostream>
#include <time.h> 
#include <cmath>
//#include "Histogram1D.h"

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

void detect_eel(
	cv::Mat& input, // 입력된 이미지
	int brightness, // 감지할 밝기 문턱값
	int saturation // 감지할 채도 문턱값
	//Histogram1D& h // Histogram을 이용한 장어 감지를 위한 클래스(아직 기능을 추가하지 않음)
)
// 장어의 길이를 감지하고 결과 이미지를 출력해주는 함수
{
	float length = 0; // 장어의 길이가 기억될 변수

	float detect_area = 0; // 장어의 면적이 기억될 변수

	cv::Mat threshold_img(input.size(), CV_8U); // input이미지와 같은크기의 비어있는 cv::Mat 변수


	cvtColor(input, input, cv::COLOR_BGR2HSV);
	// hsv_img변수에 cam_img의 데이터를 hsv형식으로 변환해서 저장

	cv::Mat channels[3]; // h, s, v 데이터를 각각 저장할 vector<Mat>선언
	split(input, channels); // channels에 h, s, v 데이터를 각각 저장함


	uchar* fixed_data = (uchar*)threshold_img.data; // threshold_img에 접근하기 위한 포인터
	uchar* data_s = (uchar*)channels[1].data; // channels[1] (채도 데이터)에 접근하기 위한 포인터
	uchar* data_v = (uchar*)channels[2].data; // channels[2] (밝기 데이터)에 접근하기 위한 포인터
	int data_len = channels[1].rows * channels[1].cols; // 입력받은 이미지의 크기

	for (int i = 0; i < data_len; i++)
		// hsv데이터를 읽고 이미지에서 장어와 비슷한 색상영역을 찾아내는 for문
		fixed_data[i] = data_s[i] < saturation || data_v[i] < brightness ? 255u : 0u;
		//장어와 비슷한 색상 영역은 255u, 그렇지 않은 영역은 0u으로 저장

	std::vector<std::vector<cv::Point>> contours; // threshold_img의 윤곽선을 저장할 벡터
	std::vector<cv::Vec4i> hierarchy;

	cv::findContours(threshold_img, contours, hierarchy,
		cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	// threshold의 윤곽선을 contours에 저장함

	int size = contours.size(); // contours에 저장된 contour의 갯수를 기억하는 변수
	int max_contour = 0;

	if (size) // contour가 없으면 실행하지 않음
	{
		int max_Area = 0; // 가장큰 contour의 면적을 기억하는 변수

		for (int i = 0; i < size; i++) // contours에서 가장 면적이 큰 contour를 찾는 for문
		{
			int area = cv::contourArea(contours[i]); // 현재 contour의 면적

			if (max_Area < area) // 현재 contour의 면적이 max_Area보다크면
			{
				max_Area = area; // max_Area에 현재 contour의 면적을 저장
				max_contour = i; //가장 면적이 큰 contour의 번호를 기억
			}
		}
		detect_area = max_Area; // 가장 면적이 큰 contour의 면적을 detect_area에 저장

		float min_dist = 10e+10; // 장어의 두께를 기억하는 변수

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

				min_dist = dist < min_dist ? dist : min_dist;
			}
		}

		std::cout << "길이:" << round(detect_area / sqrt(min_dist)) << "px" << std::endl;
		// 콘솔창에 길이를 출력
	}
}

int main()
{
	//Histogram1D h; // Histogram을 이용한 장어 감지를 위한 클래스(아직 기능을 추가하지 않음)

	cv::VideoCapture cap(1 + cv::CAP_DSHOW); //카메라를 불러옴

	cv::Mat img;

	clock_t begin_t, end_t; // 실행 시간을 기억하는 변수

	cap.set(cv::CAP_PROP_FRAME_WIDTH, 640); // 카메라 영상 가로 크기 설정
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480); // 카메라 영상 세로 크기 설정

	cap.set(cv::CAP_PROP_FPS, 120);


	while (1)
		// 반복
	{
		begin_t = clock(); // 시작 시간 기억

		cap.read(img); // 영상을 카메라에서 읽어옴
		//img = cv::imread("test_img/t.png");

		if (img.empty())
			// 영상 인식 실패
		{
			std::cerr << "빈 영상이 캡쳐됨" << std::endl;
			break;
		}

		detect_eel(img,	160, 160);
		// 장어를 감지하는 함수 호출


		end_t = clock(); // 실행 시간 기억

		std::cout << "실행시간:" << (float)(end_t - begin_t) / CLOCKS_PER_SEC << std::endl;
		// 실행시간 출력
	}
	return 0;
}