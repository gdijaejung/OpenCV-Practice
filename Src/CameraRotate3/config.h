//
// ���� ȯ�漳�� ������ �о, cConfig Ŭ������ ��� ������ �����Ѵ�.
//
#pragma once


class cConfig
{
public:
	cConfig();
	virtual ~cConfig();


protected:
	bool Parse(const string &fileName);
	void UpdateParseData();
	void InitDefault();


public:
	map<string, string> m_options;
	int m_screenWidth;
	int m_screenHeight;
	int m_cameraWidth;
	int m_cameraHeight;
	int m_screen_BinaryThreshold;
	int m_binaryThreshold;
	int m_rotateSensitivity;
	float m_detectPointMinArea;
	float m_detectPointMaxArea;
};


extern cConfig g_config;
