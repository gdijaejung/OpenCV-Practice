
#pragma once

#include "global.h"


// ��ũ���� �ν��ؼ� ũ��� ����Ʈ�� �����Ѵ�.
cv::Rect ScreenDetectAuto(std::vector<cv::Point> &out,
	const int minScreenArea, OUT int &outThreshold);


cv::Rect ScreenDetectManual(std::vector<cv::Point> &out,
	const int minScreenArea, OUT int &outThreshold);
