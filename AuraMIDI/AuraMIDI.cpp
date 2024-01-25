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
	cv::Mat temp;
	cv::flip(img, temp, 1);
	img = temp;
}

static void bgrToHsv(cv::Mat& img)
{
	cv::Mat temp;
	cv::cvtColor(img, temp, cv::COLOR_BGR2HSV);
	img = temp;
}

static void threshold(cv::Mat& img, cv::Scalar& lowerHsv, cv::Scalar& upperHsv)
{
	cv::Mat temp;
	cv::inRange(img, lowerHsv, upperHsv, temp);
	img = temp;
}

static void erosion(cv::Mat& img)
{
	cv::Mat temp;
	int erosion_type = 0; //erosion_type = MORPH_RECT
	cv::Mat element = cv::getStructuringElement(erosion_type, cv::Size(5, 5));
	cv::erode(img, temp, element);
	img = temp;
}

static void morphology(cv::Mat& img)
{
	cv::Mat temp;
	int morph_type = 0; //morph_type = Opening
	cv::Mat element = cv::getStructuringElement(morph_type, cv::Size(5, 5));
	cv::morphologyEx(img, temp, morph_type, element);
	img = temp;
}

static void dilation(cv::Mat& img)
{
	cv::Mat temp;
	int dilation_type = 0; //dilation_type = MORPH_RECT
	cv::Mat element = cv::getStructuringElement(dilation_type, cv::Size(5, 5));
	cv::dilate(img, temp, element);
	img = temp;
}

int main() {
	cv::Mat image;
	cv::Mat mask;
	cv::VideoCapture cap(0);

	if (!cap.isOpened())
	{
		std::cout << "Cannot open camera";
	}

	// Extracting hsv from json file
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
		mask = image;

		cv::Scalar grey(122, 122, 122);
		cv::Scalar green(0, 256, 0);
		cv::Scalar red(0, 0, 256);

		std::vector<cv::Scalar> patColors(5, grey);
		std::vector<cv::Scalar> trkColors(4, grey);

		// Adding the colour buttons to the live frame for colour access
		// Patterns
		cv::rectangle(image, cv::Point(80, 1), cv::Point(160, 80), patColors[0], -1);
		cv::rectangle(image, cv::Point(175, 1), cv::Point(255, 80), patColors[1], -1);
		cv::rectangle(image, cv::Point(270, 1), cv::Point(350, 80), patColors[2], -1);
		cv::rectangle(image, cv::Point(365, 1), cv::Point(445, 80), patColors[3], -1);
		cv::rectangle(image, cv::Point(460, 1), cv::Point(540, 80), patColors[4], -1);

		// Tracks
		cv::rectangle(image, cv::Point(1, 80), cv::Point(80, 160), trkColors[0], -1);
		cv::rectangle(image, cv::Point(1, 175), cv::Point(80, 255), trkColors[1], -1);
		cv::rectangle(image, cv::Point(1, 270), cv::Point(80, 350), trkColors[2], -1);
		cv::rectangle(image, cv::Point(1, 365), cv::Point(80, 445), trkColors[3], -1);

		// Image Processing
		int u_hue = cv::getTrackbarPos("Upper Hue", "Set HSV");
		int u_saturation = cv::getTrackbarPos("Upper Saturation", "Set HSV");
		int u_value = cv::getTrackbarPos("Upper Value", "Set HSV");
		int l_hue = cv::getTrackbarPos("Lower Hue", "Set HSV");
		int l_saturation = cv::getTrackbarPos("Lower Saturation", "Set HSV");
		int l_value = cv::getTrackbarPos("Lower Value", "Set HSV");
		cv::Scalar upperHsv(u_hue, u_saturation, u_value);
		cv::Scalar lowerHsv(l_hue, l_saturation, l_value);

		bgrToHsv(mask);
		threshold(mask, lowerHsv, upperHsv);
		erosion(mask);
		morphology(mask);
		dilation(mask);

		// Display
		imshow("Display Mask", mask);
		imshow("Display Cam", image);
		int key = (cv::waitKey(25) & 0xFF);
		if (key == 'q')
		{
			break;
		}
	}

	return 0;
}