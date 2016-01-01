// 
// CameraRotate3
//
// ���󿡼� ���� ��Ÿ���� n���� ���� �ν��� ��, �� ���� �߽����� ī�޶��� ȸ���� �ν��ؼ�
// Ÿ���� �̵���Ű�� ���α׷��̴�.
//
// ���󿡼� �ν��� ���� �������� skew transform�� �����Ѵ�.
//
// ���� ������ �������� Ÿ���� �̵��Ѵ�.
//
//
#include "global.h"
#include <iostream>
#include "rotatecamera.h"
#include "detectlinepoint.h"
#include "linecontour.h"


using namespace std;
using namespace cv;

int main(int argc, char* argv[])
{
	g_camera.Init(0, Size(g_config.m_cameraWidth, g_config.m_cameraHeight));
	g_screen.InitResolution(Rect(0, 0, g_config.m_screenWidth, g_config.m_screenHeight));

	vector<Point2f> points(4);
	points[0] = Point2f(420, 320);
	points[1] = Point2f(470, 320);
	points[2] = Point2f(520, 320);
	points[3] = Point2f(570, 320);
	g_screen.ShowPoints(points, Size2f(40,40));
	cvWaitKey(100);

	// ���� ���
	const Mat identityTM = (Mat_<double>(3, 3) << 1, 0, 0, 0, 1, 0, 0, 0, 1);

	// ȭ��� ���� �̷�� ������ �ν��Ѵ�.
	cDetectLinePoint detectLinePoint;
	detectLinePoint.Detect(g_camera, identityTM);

	// ���� ���� skew transform�� ���Ѵ�.
	const Mat transmtx = detectLinePoint.SkewTransform(g_config.m_cameraWidth, g_config.m_cameraHeight, 500, 400);

	// skew transform���� �ٽ� ���� �̷�� ������ ���ν� �Ѵ�.
	detectLinePoint.Detect(g_camera, transmtx);

	// ���� ������ �������� ī�޶� �̵��� ����Ѵ�.
 	cRotateCamera rotCamera;
 	cLineContour linePoints(detectLinePoint.GetDetectPoints());
	rotCamera.DetectCameraRotate(g_camera, linePoints.Center(), transmtx);

	return 0;
}
