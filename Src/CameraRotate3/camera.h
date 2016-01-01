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

	bool Init(const int index=0, const cv::Size &resolution=cv::Size(640,480));
	IplImage* GetCapture();
	cv::Mat& GetCaptureUndistortion();
	
	IplImage* ShowCapture();
	cv::Mat& ShowCaptureUndistortion();
	cv::Mat& ShowCaptureUndistortion(const cv::Mat &skewTransform, OUT cv::Mat &dst);
	const cv::Size& Resolution() const;


protected:
	CvCapture* m_capture;
	cv::Mat m_undistortCapture;
	CameraDistortion m_camDistort;
	cv::Size m_resolution; // ī�޶� �ػ�
};


inline const cv::Size& cCamera::Resolution() const { return m_resolution;  }
