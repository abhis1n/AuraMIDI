#include <iostream>
#include <opencv2/opencv.hpp>
#include <json/json.h>
#include <fstream>
#include <vector>
#include <set>
#include <RtMidi.h>

// Platform-dependent sleep routines.
#if defined(_WIN32)
#include <windows.h>
#define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds ) 
#else // Unix variants
#include <unistd.h>
#define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif

bool chooseMidiPort(RtMidiOut* rtmidi)
{
	std::string portName;
	unsigned int i = 0, nPorts = rtmidi->getPortCount();
	if (nPorts == 0) {
		std::cout << "No output ports available!" << std::endl;
		return false;
	}

	if (nPorts == 1) {
		std::cout << "\nOpening " << rtmidi->getPortName() << std::endl;
	}
	else {
		for (i = 0; i < nPorts; i++) {
			portName = rtmidi->getPortName(i);
			std::cout << "  Output port #" << i << ": " << portName << '\n';
		}

		do {
			std::cout << "\nChoose a port number: ";
			std::cin >> i;
		} while (i >= nPorts);
	}

	std::cout << "\n";
	rtmidi->openPort(i);

	return true;
}

RtMidi::Api chooseMidiApi()
{
	std::vector< RtMidi::Api > apis;
	RtMidi::getCompiledApi(apis);

	if (apis.size() <= 1)
		return RtMidi::Api::UNSPECIFIED;

	std::cout << "\nAPIs\n  API #0: unspecified / default\n";
	for (size_t n = 0; n < apis.size(); n++)
		std::cout << "  API #" << apis[n] << ": " << RtMidi::getApiDisplayName(apis[n]) << "\n";

	std::cout << "\nChoose an API number: ";
	unsigned int i;
	std::cin >> i;

	std::string dummy;
	std::getline(std::cin, dummy);  // used to clear out stdin

	return static_cast<RtMidi::Api>(i);
}

static void playNote(RtMidiOut* midiout, int note)
{
	std::vector<unsigned char> message(3);

	// Note On: 144, 64, 90
	message[0] = 144;
	message[1] = note;
	message[2] = 90;
	midiout->sendMessage(&message);

	SLEEP(1);

	// Note Off: 128, 64, 0
	message[0] = 128;
	message[1] = note;
	message[2] = 0;
	midiout->sendMessage(&message);
}

static void createAndSetTrackbar(const cv::String& trackbarname, const cv::String& winname, int value, int count)
{
	cv::createTrackbar(trackbarname, winname, nullptr, count);
	cv::setTrackbarPos(trackbarname, winname, value);
}

static int maxContour(std::vector<std::vector<cv::Point>>& contours)
{
	int maxAreaIndex = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		if (cv::contourArea(contours[i]) > cv::contourArea(contours[maxAreaIndex]))
		{
			maxAreaIndex = i;
		}
	}
	return maxAreaIndex;
}

static void setGreen(std::vector<cv::Scalar>& tileColor, int index, bool isMute = false)
{
	cv::Scalar grey(122, 122, 122);
	cv::Scalar green(0, 256, 0);
	cv::Scalar red(0, 0, 256);

	for (int i = 0; i < tileColor.size(); i++)
	{
		tileColor[i] = grey;
	}
	if (isMute == true)
	{
		tileColor[index] = red;
		return;
	}
	tileColor[index] = green;
}

int main() {
	cv::Mat image;
	cv::Mat mask;
	cv::VideoCapture cap(0);
	int track = 80;
	bool hasPlayed = false;

	if (!cap.isOpened())
	{
		std::cout << "Cannot open camera";
	}

	RtMidiOut* midiout = 0;
	std::vector<unsigned char> message;

	// RtMidiOut constructor
	try {
		midiout = new RtMidiOut(chooseMidiApi());
	}
	catch (RtMidiError& error) {
		error.printMessage();
		exit(EXIT_FAILURE);
	}

	// Call function to select port.
	try {
		if (chooseMidiPort(midiout) == false) std::cout << "Cannot open port";
	}
	catch (RtMidiError& error) {
		error.printMessage();
	}

	playNote(midiout, 60);

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

	cv::Scalar grey(122, 122, 122);
	cv::Scalar green(0, 256, 0);
	cv::Scalar red(0, 0, 256);
	cv::Scalar white(256, 256, 256);

	std::vector<cv::Scalar> patColor(5, grey);
	std::vector<cv::Scalar> trkColor(4, grey);

	while (true)
	{
		cap >> image;
		cap >> mask;
		cv::flip(image, image, 1);
		cv::flip(mask, mask, 1);

		// Adding the colour buttons to the live frame for colour access
		// Patterns
		cv::rectangle(image, cv::Point(80, 1), cv::Point(160, 80), patColor[0], -1);
		cv::rectangle(image, cv::Point(175, 1), cv::Point(255, 80), patColor[1], -1);
		cv::rectangle(image, cv::Point(270, 1), cv::Point(350, 80), patColor[2], -1);
		cv::rectangle(image, cv::Point(365, 1), cv::Point(445, 80), patColor[3], -1);
		cv::rectangle(image, cv::Point(460, 1), cv::Point(540, 80), patColor[4], -1);

		// Tracks
		cv::rectangle(image, cv::Point(1, 80), cv::Point(80, 160), trkColor[0], -1);
		cv::rectangle(image, cv::Point(1, 175), cv::Point(80, 255), trkColor[1], -1);
		cv::rectangle(image, cv::Point(1, 270), cv::Point(80, 350), trkColor[2], -1);
		cv::rectangle(image, cv::Point(1, 365), cv::Point(80, 445), trkColor[3], -1);

		// Tile Text
		cv::putText(image, "PAT 1", cv::Point(96, 33), cv::FONT_HERSHEY_SIMPLEX, 0.5, white, 1, cv::LINE_AA);
		cv::putText(image, "PAT 2", cv::Point(190, 33), cv::FONT_HERSHEY_SIMPLEX, 0.5, white, 1, cv::LINE_AA);
		cv::putText(image, "PAT 3", cv::Point(290, 33), cv::FONT_HERSHEY_SIMPLEX, 0.5, white, 1, cv::LINE_AA);
		cv::putText(image, "PAT 4", cv::Point(380, 33), cv::FONT_HERSHEY_SIMPLEX, 0.5, white, 1, cv::LINE_AA);
		cv::putText(image, "MUTE", cv::Point(480, 33), cv::FONT_HERSHEY_SIMPLEX, 0.5, white, 1, cv::LINE_AA);

		cv::putText(image, "TRACK 1", cv::Point(8, 115), cv::FONT_HERSHEY_SIMPLEX, 0.5, white, 1, cv::LINE_AA);
		cv::putText(image, "TRACK 2", cv::Point(8, 210), cv::FONT_HERSHEY_SIMPLEX, 0.5, white, 1, cv::LINE_AA);
		cv::putText(image, "TRACK 3", cv::Point(8, 305), cv::FONT_HERSHEY_SIMPLEX, 0.5, white, 1, cv::LINE_AA);
		cv::putText(image, "TRACK 4", cv::Point(8, 400), cv::FONT_HERSHEY_SIMPLEX, 0.5, white, 1, cv::LINE_AA);

		// Image Processing
		int u_hue = cv::getTrackbarPos("Upper Hue", "Set HSV");
		int u_saturation = cv::getTrackbarPos("Upper Saturation", "Set HSV");
		int u_value = cv::getTrackbarPos("Upper Value", "Set HSV");
		int l_hue = cv::getTrackbarPos("Lower Hue", "Set HSV");
		int l_saturation = cv::getTrackbarPos("Lower Saturation", "Set HSV");
		int l_value = cv::getTrackbarPos("Lower Value", "Set HSV");

		cv::Scalar upperHsv(u_hue, u_saturation, u_value);
		cv::Scalar lowerHsv(l_hue, l_saturation, l_value);
		cv::Mat element = cv::getStructuringElement(0, cv::Size(5, 5));

		cv::cvtColor(mask, mask, cv::COLOR_BGR2HSV);
		cv::inRange(mask, lowerHsv, upperHsv, mask);

		//erosion_type = MORPH_RECT
		cv::erode(mask, mask, element);

		//morph_type = Opening
		cv::morphologyEx(mask, mask, 0, element);

		//dilation_type = MORPH_RECT
		cv::dilate(mask, mask, element);

		// Find contours
		std::vector<std::vector<cv::Point>> contours;
		cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		if (contours.size() > 0)
		{
			int contour_index = maxContour(contours);
			std::vector<cv::Point> cnt = contours[contour_index];

			cv::Point2f center;
			float radius;
			cv::minEnclosingCircle(cnt, center, radius);
			cv::circle(image, center, int(radius), cv::Scalar(0, 255, 255), 2);

			if (center.x <= 80)
			{
				if ((80 <= center.y) && (center.y <= 160))
				{
					track = 80;
					setGreen(trkColor, 0);
				}
				else if ((175 <= center.y) && (center.y <= 255))
				{
					track = 70;
					setGreen(trkColor, 1);
				}
				else if ((270 <= center.y) && (center.y <= 350))
				{
					track = 60;
					setGreen(trkColor, 2);
				}
				else if ((365 <= center.y) && (center.y <= 445))
				{
					track = 50;
					setGreen(trkColor, 3);
				}
			}
			else if (center.y <= 80)
			{
				if ((80 <= center.x) && (center.x <= 160))
				{
					setGreen(patColor, 0);
					if (hasPlayed == false)
					{
						hasPlayed = true;
						playNote(midiout, track + 1);
					}
				}
				else if ((175 <= center.x) && (center.x <= 255))
				{
					setGreen(patColor, 1);
					if (hasPlayed == false)
					{
						hasPlayed = true;
						playNote(midiout, track + 2);
					}
				}
				else if ((270 <= center.x) && (center.x <= 350))
				{
					setGreen(patColor, 2);
					if (hasPlayed == false)
					{
						hasPlayed = true;
						playNote(midiout, track + 3);
					}
				}
				else if ((365 <= center.x) && (center.x <= 445))
				{
					setGreen(patColor, 3);
					if (hasPlayed == false)
					{
						hasPlayed = true;
						playNote(midiout, track + 4);
					}
				}
				else if ((460 <= center.x) && (center.x <= 540))
				{
					setGreen(patColor, 4, true);
					if (hasPlayed == false)
					{
						hasPlayed = true;
						playNote(midiout, track + 9);
					}
				}
			}
			else
			{
				hasPlayed = false;
			}
		}

		// Display
		imshow("Display Mask", mask);
		imshow("Display Cam", image);
		int key = (cv::waitKey(25) & 0xFF);
		// Press 'q' to quit
		if (key == 'q')
		{
			break;
		}
	}

	cap.release();
	cv::destroyAllWindows();

	return 0;
}