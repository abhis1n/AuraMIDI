#include <iostream>
#include <opencv2/opencv.hpp>
#include <json/json.h>
#include <fstream>
#include <vector>

static void createAndSetTrackbar(const cv::String &trackbarname, const cv::String &winname, int value, int count)
{
	cv::createTrackbar(trackbarname, winname, nullptr, count);
	cv::setTrackbarPos(trackbarname, winname, value);
}

static void flip(cv::Mat &img)
{
	cv::Mat imgtemp;
	cv::flip(img, imgtemp, 1);
	img = imgtemp;
}

int main() {
	cv::Mat image;
	cv::VideoCapture cap(0);

	if (!cap.isOpened()) 
	{
		std::cout << "Cannot open camera";
	}

	std::ifstream f("object.json");
	Json::Reader reader;
	Json::Value data;
	reader.parse(f, data);

	Json::Value jsonobj = data["highlighter"];
	std::vector<int> obj(6,0);

	for (int i = 0; i < 6; i++)
	{
		obj[i] = jsonobj[i].asInt();
	}

	// Creating the trackbars needed for adjusting the marker colour
	cv::namedWindow("Color detectors");
	createAndSetTrackbar("Upper Hue", "Color detectors", obj[0], 180);
	createAndSetTrackbar("Upper Saturation", "Color detectors", obj[1], 255);
	createAndSetTrackbar("Upper Value", "Color detectors", obj[2], 255);
	createAndSetTrackbar("Lower Hue", "Color detectors", obj[3], 180);
	createAndSetTrackbar("Lower Saturation", "Color detectors", obj[4], 255);
	createAndSetTrackbar("Lower Value", "Color detectors", obj[5], 255);

	while (true) 
	{
		cap >> image;
		flip(image);
		imshow("Display Cam", image);
		int key = (cv::waitKey(25) & 0xFF);
		if (key == 'q')
		{
			break;
		}
	}

	return 0;
}