//
// ī�޶� �󸶳� ȸ���ϴ��� �ν��ϴ� ������ �Ѵ�.
// ���� ����Ʈ�� ��� ������ �Ǵ��Ѵ�.
//
#pragma once


class cRotateCamera
{
public:
	cRotateCamera();
	virtual ~cRotateCamera();

	void DetectCameraRotate(cCamera &camera, const Point &origPoint, const Mat &skewTransform);

	SimpleBlobDetector::Params m_params;
};
