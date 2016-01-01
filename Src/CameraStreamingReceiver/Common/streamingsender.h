//
// �̹��� ������ UDP�� ���� �����Ѵ�.
// cStreamingReceiver�� ���� �����Ѵ�.
// �̹����� �����ϰų�, ���� �״�� ������ �� �ִ�.
// �̹��� �뷮�� ũ��, ������ �����Ѵ�.
//
// Protocol
// - (byte) id (�̹����� ��Ÿ���� id)
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
	class cStreamingSender
	{
	public:
		cStreamingSender();
		virtual ~cStreamingSender();

		bool Init(const bool isUDP, const string &ip, const int port, 
			const bool isConvertGray=true, const bool isCompressed=true, const int jpgQuality=40);
		int Send(const cv::Mat &image);


	protected:
		int SendImage(const BYTE *data, const int len, const bool isGray, const bool isCompressed);
		bool SendSplit();


	public:
		network::cUDPClient m_udpClient;
		network::cTCPClient m_tcpClient;
		bool m_isConvertGray;
		bool m_isCompressed;
		int m_jpgCompressQuality;
		cv::vector<uchar> m_compBuffer;
		cv::Mat m_gray;
		char *m_sndBuffer;


	protected:
		enum STATE {
			NOTCONNECT,	// ������ ���ӵǾ� ���� ���� ����
			READY,	// �̹��� ������ ������ ����
			SPLIT,	// �̹����� �������� ûũ�� ����� ������ ����
		};
		STATE m_state;
		bool m_isUDP;	// udp/ip or tcp/ip

		// Send Split Parameters
		BYTE *m_imagePtr;
		int m_chunkId;
		int m_sendChunkIndex;
		int m_chunkCount;
		int m_imageBytes;
		int m_sendBytes;
	};
}
