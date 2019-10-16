#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <vector>


using namespace cv;
using namespace std;

Mat img;
Scalar colour;
int morph_elem = 1;
int morph_size = 1;

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	/*colour = img.at<uchar>(Point(x, y));
	if (event == EVENT_LBUTTONDOWN)
	{
		cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ") is" << colour << endl;
	}*/
	int i = img.at<Vec3b>(Point(x, y))[0];
	int j = img.at<Vec3b>(Point(x, y))[1];
	int k = img.at<Vec3b>(Point(x, y))[2];

	if (event == EVENT_LBUTTONDOWN)
	{
		cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ") is (" << i << ", " << j << ", " << k << ")" << endl;
	}
}

int main()
{
	Mat image = imread("img_input.jpg");
	Mat mask_out, image_hsv;
	Mat channel[3];
	Mat brownMask;
	Vec3b bgrPixel(31, 42, 62);
	Mat3b bgr(bgrPixel);
	Scalar highBrown = Scalar(114, 121, 151);
	Scalar lowBrown = Scalar(38, 50, 80);

	img = image;
	//input image and resize
	resize(image, image, Size(), 0.3, 0.3);
	//detect color module
	namedWindow("1. Input");
	//set the callback function for any mouse event
	setMouseCallback("1. Input", CallBackFunc, NULL);
	//show the image
	imshow("1. Input", image); waitKey(0);

	//convert to HSV
	cvtColor(image, image_hsv, COLOR_BGR2HSV);

	imshow("2. HSV Input", image_hsv); waitKey(0);
	split(image_hsv, channel);
	//convert to HSV channel[1]
	imshow("3. Saturation Channel", channel[1]); waitKey(0);
	//create Mask form HSV cut the red out
	Mat hsvMask;
	inRange(image_hsv, Scalar(0, 0, 80), Scalar(255, 255, 255), hsvMask);
	imshow("4. RED OUT form (2)", hsvMask); waitKey(0);
	//create Mask form HSV channel[1] <Saturation>
	Mat vMask, mergeMask;
	inRange(channel[1], Scalar(67, 67, 67), Scalar(255, 255, 255), vMask);
	bitwise_not(vMask, vMask);
	imshow("5. BLACK OUT from (3)", vMask); waitKey(0);
	//create Mask from light brown
	inRange(image, lowBrown, highBrown, brownMask);
	imshow("6. Reflected Brown Mask from (1)", brownMask); waitKey(0);

	vector<Point> cut_start = { Point(0,0),Point(277, 478) , Point(0, 292),  Point( brownMask.cols / 2,0),Point(478, 365),Point(328, 515),	Point(436, 438) };
	vector<Point> cut_finish = { Point(419,303),Point(461, 585), Point(282, 581) ,Point(brownMask.cols, brownMask.rows),Point(575, 476),Point(676, brownMask.rows),	Point(620, 533) };

	/*
 (806, 310) is (19, 27, 50)
Left button of the mouse is clicked - position (937, 392)
	*/
	
	cout << "COL" << brownMask.cols << "ROW" << brownMask.rows << endl;
	for (int i = 0; i < cut_start.size(); i++) {
		rectangle(brownMask, cut_start[i], cut_finish[i], Scalar(0), CV_FILLED);
	}
	
	imshow("7. Cut Shadow <Black> from (6)", brownMask); waitKey(0);
	
	
	//Combine two mask to a single mask
	bitwise_and(hsvMask, vMask, mergeMask);
	imshow("8. (4) and (5)", mergeMask); waitKey(0);
	bitwise_or(mergeMask, brownMask, mergeMask);
	imshow("9. (8) or (5)", mergeMask); waitKey(0);

	vector<Point> white_start = { Point(0,0),Point(394, 510) , Point(111,0),Point(371, 525),Point(386, 518) ,Point(136, 355),Point(806, 310) };
	vector<Point> white_finish = { Point(163, 43),Point(449, 544) ,Point(308,30),Point(402, 542),Point(407, 533) ,Point(0, 467) ,Point(937, 392) };
	for (int i = 0; i < white_start.size(); i++) {
		rectangle(mergeMask, white_start[i], white_finish[i], Scalar(255), CV_FILLED);
	}
	imshow("10. Cut Shadow <White> from (9)", mergeMask); waitKey(0);
	
	bitwise_xor(mergeMask, brownMask, mergeMask);
	imshow("11. (7) xor (10)", mergeMask); waitKey(0);
	//close mask to remove line of tiles
	
	Mat element = getStructuringElement(morph_elem, Size(2 * morph_size + 1, 2 * morph_size + 1), Point(morph_size, morph_size));
	morphologyEx(mergeMask, mergeMask, MORPH_CLOSE, element);
	//morphologyEx(mergeMask, mergeMask, MORPH_OPEN, element);
    imshow("12. Morph Close & Morph Open", mergeMask); waitKey(0);
	
	
	Mat canny_output;
	
	

	//canny
	//blur(mergeMask,mask_out,Size(3,3));
	//imshow("6. blurred", mask_out);
	Canny(mergeMask, canny_output, 1, 3);
	imshow("13. Canny edge detection", canny_output); waitKey(0);

	//contour
	vector<vector<Point>> contours; // list of contour points
	vector<Vec4i> hierarchy;

	// find contours
	findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
	RNG rng(12345);
	//Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
	// drawContours
	for (size_t i = 0; i < contours.size(); i++)
	{
		Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
		drawContours(drawing, contours, (int)i, color, 2, LINE_8, hierarchy, 0);
	}
	// Wait until user press some key
	imshow("14. Contours", drawing);
	waitKey(0);
	image.release();

	

	return 0;
}