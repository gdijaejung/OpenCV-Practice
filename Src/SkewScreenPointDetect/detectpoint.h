//
// ��ũ������ ������ ����Ʈ�� ã��, ��ġ�� ����Ѵ�.
//
// ��ũ���� ��� ������ ������ ������ ����Ʈ�� �ִٰ� �����Ѵ�.
//
#pragma once


class cDetectPoint
{
public:
	cDetectPoint();
	virtual ~cDetectPoint();

	cv::Point DetectPoint(cScreen &screen, cCamera &camera, const cv::Mat &src);
};

