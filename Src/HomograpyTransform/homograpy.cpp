
//
// 2016-03-16, jjuiddong
//
// ���콺 ���� ��ư Ŭ�� 4������ �ܰ� �簢�� �������� �����Ѵ�.
// 4���� �������� perspective transform �� �̿��� ȭ���� ��ȯ �Ѵ�.
// ���콺 ������ ��ư�� Ŭ���Ͽ�, ���� �̹��� ����Ʈ����, ��ȯ�� ����Ʈ�� Ȯ���� �� �ִ�.
//
//

#include<iostream>
#include<opencv2/opencv.hpp>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "opencv_core2411d.lib")
#pragma comment(lib, "opencv_highgui2411d.lib")
#pragma comment(lib, "opencv_imgproc2411d.lib")


using namespace std;
using namespace cv;

#define OUT

void MouseClickHandler(int event, int x, int y, int flags, void* userdata);
bool GetIntersectPoint(const Point2f& AP1, const Point2f& AP2, const Point2f& BP1, const Point2f& BP2, Point2f* IP);
void OrderedContours(const vector<cv::Point> &src, OUT vector<cv::Point> &dst);


vector<Point> g_mouseClickPos;
Point g_camPos(0,0);
Mat g_perspectivMat;


int main(int argc, char *argv[])
{
	namedWindow("MouseClick");
	cvResizeWindow("MouseClick", 640, 480);
	setMouseCallback("MouseClick", MouseClickHandler, NULL);

	Mat mouseMat(480, 640, CV_8UC3);
	Mat dest(480, 640, CV_8UC3);

	while (1)
	{
		mouseMat.setTo(Scalar(200, 200, 200));
		for (unsigned int i = 0; i < g_mouseClickPos.size(); ++i)
		{
			const Point pos = g_mouseClickPos[i];
			circle(mouseMat, pos, 10, Scalar(0, 0, 0), CV_FILLED);
		}

		circle(mouseMat, g_camPos, 10, Scalar(0, 100, 0), CV_FILLED);
		imshow("MouseClick", mouseMat);

		if (g_mouseClickPos.size() >= 4)
		{
			namedWindow("Homography");
			cvResizeWindow("Homography", 640, 480);
			dest.setTo(Scalar(200, 200, 200));
			cv::warpPerspective(mouseMat, dest, g_perspectivMat, dest.size());


			std::vector<Point2f> camera_corners;
			camera_corners.push_back(g_camPos);
			std::vector<Point2f> world_corners;
			perspectiveTransform(camera_corners, world_corners, g_perspectivMat);

			circle(dest, world_corners[0], 10, Scalar(0, 0, 100), CV_FILLED);

			imshow("Homography", dest);
		}

		if (waitKey(1) == 27) break;
	}
	return 0;
}



void MouseClickHandler(int event, int x, int y, int flags, void* userdata)
{
	if (event == EVENT_LBUTTONDOWN)
	{
		static int mouseIdx = 0;
		if (mouseIdx < 4)
		{
			g_mouseClickPos.push_back(Point(x, y));
		}
		else
		{
			g_mouseClickPos[mouseIdx % 4] = Point(x, y);
		}

		if (g_mouseClickPos.size() >= 4)
		{
			const int width = 640;
			const int height = 480;
			vector<Point2f> quad(4);
			quad[0] = Point2f(0, 0);
			quad[1] = Point2f((float)width, (float)0);
			quad[2] = Point2f((float)width, (float)height);
			quad[3] = Point2f((float)0, (float)height);

			vector<Point> contour(4);
			OrderedContours(g_mouseClickPos, contour);

			Point2f ip; // ����
			if (!GetIntersectPoint(contour[0], contour[2], contour[3], contour[1], &ip))
				return;

			const float scale = 1.f;
			const Point2f offset1 = ((Point2f)contour[0] - ip) * (scale - 1.f);
			const Point2f offset2 = ((Point2f)contour[1] - ip) * (scale - 1.f);
			const Point2f offset3 = ((Point2f)contour[2] - ip) * (scale - 1.f);
			const Point2f offset4 = ((Point2f)contour[3] - ip) * (scale - 1.f);

			vector<Point2f> screen(4);
			screen[0] = (Point2f)contour[0] + offset1;
			screen[1] = (Point2f)contour[1] + offset2;
			screen[2] = (Point2f)contour[2] + offset3;
			screen[3] = (Point2f)contour[3] + offset4;

			g_perspectivMat = getPerspectiveTransform(screen, quad);
		}

		++mouseIdx;
	}
	else if (event == EVENT_RBUTTONDOWN)
	{
		g_camPos = Point(x, y);
	}
	else if (event == EVENT_MBUTTONDOWN)
	{
		cout << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
	}

}


// hhttp://www.gisdeveloper.co.kr/15
// AP1 - AP2 �� ������ ����
// AP3 - AP4 �� ������ ����
// �� ������ ������ ã�� IP�� �����Ѵ�.
// ������ ���ٸ� false�� �����Ѵ�.
bool GetIntersectPoint(const Point2f& AP1, const Point2f& AP2,
	const Point2f& BP1, const Point2f& BP2, Point2f* IP)
{
	float t;
	float s;
	float under = (BP2.y - BP1.y)*(AP2.x - AP1.x) - (BP2.x - BP1.x)*(AP2.y - AP1.y);
	if (under == 0) return false;

	float _t = (BP2.x - BP1.x)*(AP1.y - BP1.y) - (BP2.y - BP1.y)*(AP1.x - BP1.x);
	float _s = (AP2.x - AP1.x)*(AP1.y - BP1.y) - (AP2.y - AP1.y)*(AP1.x - BP1.x);

	t = _t / under;
	s = _s / under;

	if (t<0.0 || t>1.0 || s<0.0 || s>1.0) return false;
	if (_t == 0 && _s == 0) return false;

	IP->x = AP1.x + t * (float)(AP2.x - AP1.x);
	IP->y = AP1.y + t * (float)(AP2.y - AP1.y);

	return true;
}



// contours �� ������� �����Ѵ�.
// 0 ---------- 1
// |            |
// |            |
// |            |
// 2 ---------- 3
void OrderedContours(const vector<cv::Point> &src, OUT vector<cv::Point> &dst)
{
	// ���� ū �ڽ��� ã�´�.
	Point center = src[0];
	center += src[1];
	center += src[2];
	center += src[3];

	// �߽��� ���
	center.x /= 4;
	center.y /= 4;

	vector<bool> check(4, false);
	vector<bool> check2(4, false);

	// �ð�������� left top ���� ������� 0, 1, 2, 3 ��ġ�� ����Ʈ�� ����ȴ�.
	for (int k = 0; k < 4; ++k)
	{
		if (!check[0] && (src[k].x < center.x) && (src[k].y < center.y))
		{
			dst[0] = src[k];
			check[0] = true;
			check2[k] = true;
		}
		if (!check[3] && (src[k].x < center.x) && (src[k].y > center.y))
		{
			dst[3] = src[k];
			check[3] = true;
			check2[k] = true;
		}
		if (!check[1] && (src[k].x > center.x) && (src[k].y < center.y))
		{
			dst[1] = src[k];
			check[1] = true;
			check2[k] = true;
		}
		if (!check[2] && (src[k].x > center.x) && (src[k].y > center.y))
		{
			dst[2] = src[k];
			check[2] = true;
			check2[k] = true;
		}
	}

	for (int i = 0; i < 4; ++i)
	{
		if (!check[i])
		{
			for (int k = 0; k < 4; ++k)
			{
				if (!check2[k])
				{
					check[i] = true;
					check2[k] = true;
					dst[i] = src[k];
				}
			}
		}
	}


}