#include <iostream>
#include <opencv2/opencv.hpp>
#include <json/json.h>
#include <fstream>
#include <vector>

static void createAndSetTrackbar(const cv::String& trackbarname, const cv::String& winname, int value, int count)
{
	cv::createTrackbar(trackbarname, winname, nullptr, count);
	cv::setTrackbarPos(trackbarname, winname, value);
}

static void flip(cv::Mat& img)
{
	cv::Mat imgtemp;
	cv::flip(img, imgtemp, 1);
	img = imgtemp;
}

static void erosion(cv::Mat& src, cv::Mat& erosion_dst)
{
	int erosion_type = 0; //erosion_type = MORPH_RECT
	cv::Mat element = cv::getStructuringElement(0,
												cv::Size(5, 5));
	cv::erode(src, erosion_dst, element);
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
	std::vector<int> obj(6, 0);

	for (int i = 0; i < 6; i++)
	{
		obj[i] = jsonobj[i].asInt();
	}

	// Creating the trackbars needed for adjusting the marker colour
	cv::namedWindow("Set HSV");
	createAndSetTrackbar("Upper Hue", "Set HSV", obj[0], 180);
	createAndSetTrackbar("Upper Saturation", "Set HSV", obj[1], 255);
	createAndSetTrackbar("Upper Value", "Set HSV", obj[2], 255);
	createAndSetTrackbar("Lower Hue", "Set HSV", obj[3], 180);
	createAndSetTrackbar("Lower Saturation", "Set HSV", obj[4], 255);
	createAndSetTrackbar("Lower Value", "Set HSV", obj[5], 255);

	while (true)
	{
		cap >> image;
		flip(image);
		cv::Mat hsv;
		cv::Mat mask;
		cv::Mat maskerode;
		cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);

		int u_hue = cv::getTrackbarPos("Upper Hue", "Set HSV");
		int u_saturation = cv::getTrackbarPos("Upper Saturation", "Set HSV");
		int u_value = cv::getTrackbarPos("Upper Value", "Set HSV");
		int l_hue = cv::getTrackbarPos("Lower Hue", "Set HSV");
		int l_saturation = cv::getTrackbarPos("Lower Saturation", "Set HSV");
		int l_value = cv::getTrackbarPos("Lower Value", "Set HSV");
		cv::Scalar upperHsv(u_hue, u_saturation, u_value);
		cv::Scalar lowerHsv(l_hue, l_saturation, l_value);

		cv::inRange(hsv, lowerHsv, upperHsv, mask);
		erosion(mask, maskerode);

		imshow("Display Mask", mask);
		imshow("Display MaskErode", maskerode);
		imshow("Display Cam", image);
		int key = (cv::waitKey(25) & 0xFF);
		if (key == 'q')
		{
			break;
		}
	}

	return 0;
}