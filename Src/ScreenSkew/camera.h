//
// ī�޶� �Կ��� ������ ��������, �����쿡 ����ϴ� ����� �Ѵ�.
// ī�޶� ���� �ְ��� �����ϴ� ����� �Ѵ�.
//
#pragma once

#include "CameraDistortion.h"


class cCamera
{
public:
	cCamera();
	virtual ~cCamera();

	bool Init(const int index=0);
	IplImage* GetCapture();
	cv::Mat& GetCaptureUndistortion();
	
	IplImage* ShowCapture();
	cv::Mat& ShowCaptureUndistortion();


protected:
	CvCapture* m_capture;
	cv::Mat m_undistortCapture;
	CameraDistortion m_camDistort;
};
