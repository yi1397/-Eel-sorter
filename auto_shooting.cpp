#include <iostream>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

void mode1(int number)
{
	Mat cam_img;
	cv::VideoCapture cap(1 + cv::CAP_DSHOW);
	if (!cap.isOpened())
	{
		cerr << "카메라를 열 수 없음" << endl;
		return;
	}

	cap.set(cv::CAP_PROP_FRAME_WIDTH, 320); // 카메라 영상 가로 크기 설정
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, 240); // 카메라 영상 세로 크기 설정

	cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64FC1);
	cv::Mat distCoeffs = cv::Mat::zeros(1, 5, CV_64FC1);;

	cameraMatrix = (cv::Mat1d(3, 3) << 327.495, 0, 170.231, 0, 342.608, 120.341, 0, 0, 1);
	distCoeffs = (cv::Mat1d(1, 4) << -0.447434, 0.231295, 0.000754, -0.001325);


	int cnt = 0;

	while (1)
	{
		cap.read(cam_img);
		if (cam_img.empty())
		{
			cerr << "빈 영상이 캡쳐됨" << endl;
			break;
		}
		imwrite("dataset/320_240/"
			+ to_string(number) + "_" + to_string(cnt) + ".png", cam_img);

		cv::Mat temp;
		cv::undistort(cam_img, temp, cameraMatrix, distCoeffs);

		imwrite("dataset/320_240_Calibration/"
			+ to_string(cnt) + "_" + to_string(cnt) + ".png", temp);

		imshow("Cam", cam_img);
		if (waitKey(1) == 27)
			break;
		cnt++;
	}
	destroyAllWindows();
}

void mode2(int number)
{
	Mat cam_img;
	cv::VideoCapture cap(1 + cv::CAP_DSHOW);
	if (!cap.isOpened())
	{
		cerr << "카메라를 열 수 없음" << endl;
		return;
	}

	cap.set(cv::CAP_PROP_FRAME_WIDTH, 640); // 카메라 영상 가로 크기 설정
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480); // 카메라 영상 세로 크기 설정

	cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64FC1);
	cv::Mat distCoeffs = cv::Mat::zeros(1, 5, CV_64FC1);;

	cameraMatrix = (cv::Mat1d(3, 3) << 652.551, 0, 333.847, 0, 641.968, 234.218, 0, 0, 1);
	distCoeffs = (cv::Mat1d(1, 4) << -0.444277, 0.253481, 0.001623, 0.000861);


	int cnt = 0;

	while (1)
	{
		cap.read(cam_img);
		if (cam_img.empty())
		{
			cerr << "빈 영상이 캡쳐됨" << endl;
			break;
		}
		imwrite("dataset/640_480/"
			+ to_string(number) + "_" + to_string(cnt) + ".png", cam_img);

		cv::Mat temp;
		cv::undistort(cam_img, temp, cameraMatrix, distCoeffs);

		imwrite("dataset/640_480_Calibration/"
			+ to_string(cnt) + "_" + to_string(cnt) + ".png", temp);

		imshow("Cam", cam_img);
		if (waitKey(1) == 27)
			break;
		cnt++;
	}
	destroyAllWindows();
}

void mode3(int number)
{
	Mat cam_img;
	cv::VideoCapture cap(1 + cv::CAP_DSHOW);
	if (!cap.isOpened())
	{
		cerr << "카메라를 열 수 없음" << endl;
		return;
	}

	cap.set(cv::CAP_PROP_FRAME_WIDTH, 320); // 카메라 영상 가로 크기 설정
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, 240); // 카메라 영상 세로 크기 설정

	cap.set(cv::CAP_PROP_FPS, 60);

	cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64FC1);
	cv::Mat distCoeffs = cv::Mat::zeros(1, 5, CV_64FC1);;

	cameraMatrix = (cv::Mat1d(3, 3) << 327.495, 0, 170.231, 0, 342.608, 120.341, 0, 0, 1);
	distCoeffs = (cv::Mat1d(1, 4) << -0.447434, 0.231295, 0.000754, -0.001325);


	int cnt = 0;

	Size size = Size((int)cap.get(CAP_PROP_FRAME_WIDTH),
		(int)cap.get(CAP_PROP_FRAME_HEIGHT));

	VideoWriter writer;
	double fps = 120.0;
	writer.open("dataset/320_240/" + to_string(number) + ".avi",
		VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, size, fps);

	VideoWriter writer_c;
	writer_c.open("dataset/320_240_Calibration/" + to_string(number) + ".avi",
		VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, size, fps);

	if (!writer.isOpened()||!writer_c.isOpened())
	{
		cerr << "동영상 초기화 실패" << endl;
		return;
	}

	while (1)
	{
		cap.read(cam_img);
		if (cam_img.empty())
		{
			cerr << "빈 영상이 캡쳐됨" << endl;
			break;
		}
		writer.write(cam_img);

		cv::Mat temp;
		cv::undistort(cam_img, temp, cameraMatrix, distCoeffs);

		writer_c.write(temp);

		imshow("Cam", cam_img);
		if (waitKey(1) == 27)
			break;
		cnt++;
	}
	destroyAllWindows();
}

void mode4(int number)
{
	Mat cam_img;
	cv::VideoCapture cap(1 + cv::CAP_DSHOW);
	if (!cap.isOpened())
	{
		cerr << "카메라를 열 수 없음" << endl;
		return;
	}

	cap.set(cv::CAP_PROP_FRAME_WIDTH, 640); // 카메라 영상 가로 크기 설정
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480); // 카메라 영상 세로 크기 설정
	cap.set(cv::CAP_PROP_FPS, 60);

	cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64FC1);
	cv::Mat distCoeffs = cv::Mat::zeros(1, 5, CV_64FC1);;

	cameraMatrix = (cv::Mat1d(3, 3) << 652.551, 0, 333.847, 0, 641.968, 234.218, 0, 0, 1);
	distCoeffs = (cv::Mat1d(1, 4) << -0.444277, 0.253481, 0.001623, 0.000861);


	int cnt = 0;

	Size size = Size((int)cap.get(CAP_PROP_FRAME_WIDTH),
		(int)cap.get(CAP_PROP_FRAME_HEIGHT));

	VideoWriter writer;
	double fps = 60.0;
	writer.open("dataset/640_480/" + to_string(number) + ".avi",
		VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, size, fps);

	VideoWriter writer_c;
	writer_c.open("dataset/640_480_Calibration/" + to_string(number) + ".avi",
		VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, size, fps);

	if (!writer.isOpened() || !writer_c.isOpened())
	{
		cerr << "동영상 초기화 실패" << endl;
		return;
	}

	while (1)
	{
		cap.read(cam_img);
		if (cam_img.empty())
		{
			cerr << "빈 영상이 캡쳐됨" << endl;
			break;
		}
		writer.write(cam_img);

		cv::Mat temp;
		cv::undistort(cam_img, temp, cameraMatrix, distCoeffs);

		writer_c.write(temp);

		imshow("Cam", cam_img);
		if (waitKey(1) == 27)
			break;
		cnt++;
	}
	destroyAllWindows();
}

int main()
{
	int cnt = 0;
	while (true) 
	{
		char command;

		cout << "1 : 320 * 240 png" << endl;
		cout << "2 : 640 * 480 png" << endl;
		cout << "3 : 320 * 240 avi" << endl;
		cout << "4 : 640 * 480 avi" << endl;

		cin >> command;



		switch (command)
		{
		case '1':
			mode1(cnt);
			break;

		case '2':
			mode2(cnt);
			break;

		case '3':
			mode3(cnt);
			break;

		case '4':
			mode4(cnt);
			break;

		default:
			break;
		}
		cnt++;
	}


	return 0;
}