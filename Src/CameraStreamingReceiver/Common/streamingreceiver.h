//
// Protocol
//	- (byte) chunk size (�̹��� �뷮 ������ ������ ����Ÿ ûũ�� ����)
//	- (byte) chunk index
//	- (byte) gray 0 : 1
//	- (byte) compressed 0 : 1
//	- (int) image size
//
#pragma once

#include "streaming.h"


namespace cvproc
{
	class cStreamingReceiver
	{
	public:
		cStreamingReceiver();
		virtual ~cStreamingReceiver();

		bool Init(const bool isUDP, const int bindPort);
		cv::Mat& Update();


		network::cUDPServer m_udpServer;
		network::cTCPServer m_tcpServer;
		bool m_isUDP;
		cv::Mat m_src;
		cv::Mat m_finalImage;		// ���� �̹���
		char *m_rcvBuffer;
	};
}
