#include <opencv2/opencv.hpp>
#include <iostream>
#include <time.h> 
#include <cmath>
//#include "Histogram1D.h"
using namespace cv;
using namespace std;

double px_to_cm_ratio = 1; // 1cm가 몇 픽셀인지 저장하는 변수

double calc_dist(Point A, Point B)
// cv::Point 구조체를 파라미터로 받아서 두 cv::Point의 거리를 측정하는 함수
{
	double dist; 
	int x_dist = A.x - B.x; // A와 B의 x방향 거리차이 
	int y_dist = A.y - B.y; // A와 B의 y방향 거리차이

	dist = x_dist * x_dist + y_dist * y_dist; // A와 B의 거리차이의 제곱의 합

	return sqrt((double)dist); // A와 B의 거리를 return해줌

}

void on_trackbar(int, void*)
// cv::createTrackbar를 위한 함수(기능 없음)
{

}

void detect_eel(
	Mat& input, // 입력된 이미지
	Mat& output, // 출력할 결과 이미지
	int brightness, // 감지할 밝기 문턱값
	int saturation // 감지할 채도 문턱값
	//Histogram1D& h // Histogram을 이용한 장어 감지를 위한 클래스(아직 기능을 추가하지 않음)
)
// 장어의 길이를 감지하고 결과 이미지를 출력해주는 함수
{
	double length = 0; // 장어의 길이가 기억될 변수

	double detect_area = 0; // 장어의 면적이 기억될 변수

	Mat cam_img = input.clone(); // input이미지를 복사함

	Mat threshold_img(input.size(), CV_8U); // input이미지와 같은크기의 비어있는 cv::Mat 변수

	Mat detect(input.size(), CV_8UC3); // input이미지와 같은크기의 비어있는 cv::Mat 변수

	detect = Scalar(0, 0, 0); // detect를 검은색 이미지로 초기화

	Mat hsv_img; // hsv형식의 색상 데이터가 저장될 cv::Mat 변수

	cvtColor(cam_img, hsv_img, COLOR_BGR2HSV); 
	// hsv_img변수에 cam_img의 데이터를 hsv형식으로 변환해서 저장

	vector<Mat> channels; // h, s, v 데이터를 각각 저장할 vector<Mat>선언
	split(hsv_img, channels); // channels에 h, s, v 데이터를 각각 저장함


	uchar* fixed_data = (uchar*)threshold_img.data; // threshold_img에 접근하기 위한 포인터
	uchar* data_s = (uchar*)channels[1].data; // channels[1] (채도 데이터)에 접근하기 위한 포인터
	uchar* data_v = (uchar*)channels[2].data; // channels[2] (밝기 데이터)에 접근하기 위한 포인터
	int data_len = channels[1].rows * channels[1].cols; // 입력받은 이미지의 크기

	for (int i = 0; i < data_len; i++)
	// hsv데이터를 읽고 이미지에서 장어와 비슷한 색상영역을 찾아내는 for문
	{
		fixed_data[i] = data_s[i] < saturation || data_v[i] < brightness ? 255u : 0u;
		//장어와 비슷한 색상 영역은 흰색, 그렇지 않은 영역은 검은색으로 저장
	}

	vector<vector<Point>> contours; // threshold_img의 윤곽선을 저장할 함수
	vector<Vec4i> hierarchy;
	findContours(threshold_img, contours, hierarchy, 
		RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
	// threshold의 윤곽선을 contours에 저장함

	int size = contours.size(); // contours에 저장된 contour의 갯수를 기억하는 변수
	int max_contour = 0;

	if (size) // contour가 없으면 실행하지 않음
	{
		int max_Area = 0; // 가장큰 contour의 면적을 기억하는 변수
		for (int i = 0; i < size; i++) // contours에서 가장 면적이 큰 contour를 찾는 for문
		{
			if (max_Area < contourArea(contours[i])) // 현재 contour의 면적이 max_Area보다크면
			{
				max_Area = contourArea(contours[i]); // max_Area에 현재 contour의 면적을 저장
				max_contour = i; //가장 면적이 큰 contour의 번호를 기억
			}
		}
		detect_area = contourArea(contours[max_contour]); // 가장 면적이 큰 contour의 면적을 detect_area에 저장

		double min_dist = 10e+10; // 장어의 두께를 기억하는 함수

		Point minA, minB; // contour의 cv::Point를 저장할 변수
		
		for (int i = 0; i < contours[max_contour].size() >> 1; i++) // 가장 면적이 큰 contour의 0번부터 절반까지 반복
			// i는 가장큰 contour의 0번부터 절반까지 반복
		{
			for (int j = -((int)contours[max_contour].size() >>3) + 1;
				j < ((int)contours[max_contour].size() >> 3);
				j++)
				// j는 0부터 가장큰 contour크기의 1/8 까지 증가
			{
				int k = ((i + contours[max_contour].size() / 2) + j)
					% contours[max_contour].size();
				// k는 i의 반대지점에서 ± contour크기의 1/8 사이

				double dist = calc_dist(contours[max_contour][i], contours[max_contour][k]); 
				// 가장큰 contour의 i번째와 k번째 사이의 거리를 기억하는 변수

				if (dist < min_dist) // dist가 최단거리이면
				{
					min_dist = dist; // dist를 저장

					minA = contours[max_contour][i]; minB = contours[max_contour][k];
					// i와 k의 위치를 기억
				}
			}
		}
		line(detect, minA, minB, Scalar(255, 0, 0), 2);
		// 결과 이미지에 minA와 minB 사이를 표시해줌

		putText(detect, to_string(round(min_dist / px_to_cm_ratio * 10) / 10),
			Point(50, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255), 2);
		// 결과 이미지에 두께를 표시해줌

		length = round(detect_area / min_dist / px_to_cm_ratio);
		// 길이를 계산함

		putText(cam_img, to_string(length),
			Point(50, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(255, 0, 0), 2);
		// 결과 이미지에 길이를 표시해줌

		cout << "길이:" << detect_area / min_dist << "px" << endl;
		// 콘솔창에 길이를 출력
	}

	cvtColor(threshold_img, threshold_img, COLOR_GRAY2BGR);
	// 결과 이미지를 컬러로 변환

	drawContours(threshold_img, contours, max_contour, 
		Scalar(255, 0, 0), 2, 8, hierarchy, 0, Point());
	// 결과 이미지에 장어 윤곽선을 표시해줌

	drawContours(detect, contours, max_contour, 
		Scalar(0, 0, 255), 1, 8, hierarchy, 0, Point());
	// 결과 이미지에 장어 윤곽선을 표시해줌

	putText(threshold_img, 
		to_string(round(detect_area / px_to_cm_ratio / px_to_cm_ratio)), 
		Point(50, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255), 2);
	// 결과 이미지에 장어의 면적을 표시해줌
	
	hconcat(cam_img, threshold_img, output);
	hconcat(output, detect, output);
	// 3개의 결과 이미지를 출력할 이미지에 저장
}

int main()
{
	//Histogram1D h; // Histogram을 이용한 장어 감지를 위한 클래스(아직 기능을 추가하지 않음)

	int brightness_to_detect; // 감지할 밝기 문턱값
	int saturation_to_detect; // 감지할 채도 문턱값

	VideoCapture cap(0); //카메라를 불러옴

	Mat img;
	Mat detect_img;

	if (!cap.isOpened())
		//카메라 실행 실패
	{
		cerr << "카메라를 열 수 없음" << endl;
		return -1;
	}

	string brightness_trackbar_name = "감지할밝기";
	string saturation_trackbar_name = "감지할채도";
	// trackbar의 이름

	namedWindow("detect", WINDOW_FREERATIO);
	// 출력 윈도우

	createTrackbar(brightness_trackbar_name, "detect", 0, 255, on_trackbar);
	createTrackbar(saturation_trackbar_name, "detect", 0, 255, on_trackbar);
	// 윈도우에 trackbar를 만듬

	setTrackbarPos(brightness_trackbar_name, "detect", 160);
	setTrackbarPos(saturation_trackbar_name, "detect", 160);
	// trackbar의 기본값을 설정

	while (1)
		// 반복
	{

		clock_t begin_t, end_t;
		begin_t = clock();

		cap.read(img);
		if (img.empty())
		{
			cerr << "빈 영상이 캡쳐됨" << endl;
			break;
		}

		brightness_to_detect = getTrackbarPos(brightness_trackbar_name, "detect");
		saturation_to_detect = getTrackbarPos(saturation_trackbar_name, "detect");

		detect_eel(img, detect_img, brightness_to_detect, saturation_to_detect);
		// 장어를 감지하는 함수 호출

		imshow("detect", detect_img);

		switch (waitKeyEx(1)) // 키보드 입력
		{
		case 27: // esc 키 입력
			return 0;
			// 종료
		case 2424832: // ← 키 입력
			setTrackbarPos(brightness_trackbar_name, "detect", 
				getTrackbarPos(brightness_trackbar_name, "detect") - 1);
			// brightness_trackbar_name의 값 1 감소

			break;
		case 2555904: // → 키 입력
			setTrackbarPos(brightness_trackbar_name, "detect", 
				getTrackbarPos(brightness_trackbar_name, "detect") + 1);
			// brightness_trackbar_name의 값 1 증가

			break;
		case 2490368: // ↑ 키 입력
			setTrackbarPos(saturation_trackbar_name, "detect",
				getTrackbarPos(saturation_trackbar_name, "detect") + 1);
			// saturation_trackbar_name의 값 1 증가

			break;
		case 2621440: // ↓ 키 입력
			setTrackbarPos(saturation_trackbar_name, "detect",
				getTrackbarPos(saturation_trackbar_name, "detect") - 1);
			// saturation_trackbar_name의 값 1 감소

			break;
		}

		end_t = clock();
		cout << "실행시간:" << (double)(end_t - begin_t) / CLOCKS_PER_SEC << endl;
	}
	return 0;
}