
#include "stdafx.h"
#include "streamingreceiver.h"


using namespace cvproc;


cStreamingReceiver::cStreamingReceiver() 
	: m_rcvBuffer(NULL)
{

}

cStreamingReceiver::~cStreamingReceiver()
{
	SAFE_DELETEA(m_rcvBuffer);

}


bool cStreamingReceiver::Init(const bool isUDP, const int bindPort)
{
	m_isUDP = isUDP;

	m_udpServer.Close();
	m_tcpServer.Close();
	
	if (isUDP)
	{
		m_udpServer.SetMaxBufferLength(307200);
		if (!m_udpServer.Init(0, bindPort))
			return false;
	}
	else
	{
		if (!m_tcpServer.Init(bindPort, g_maxStreamSize, 10, 10))
			return false;
	}

	if (m_src.empty())
		m_src = Mat(480, 640, CV_8UC1);
	if (m_finalImage.empty())
		m_finalImage = Mat(480, 640, CV_8UC1);

	if (!m_rcvBuffer)
		m_rcvBuffer = new char[g_maxStreamSize];

	return true;
}


cv::Mat& cStreamingReceiver::Update()
{
	const static int sizePerChunk = g_maxStreamSize - sizeof(sStreamingData); // size per chunk

	int len = 0;
	if (m_isUDP)
	{
		len = m_udpServer.GetRecvData(m_rcvBuffer, g_maxStreamSize);
	}
	else
	{
		network::sPacket packet;
		if (m_tcpServer.m_recvQueue.Front(packet))
		{
			len = packet.actualLen;
			memcpy(m_rcvBuffer, packet.buffer, packet.actualLen);
			m_tcpServer.m_recvQueue.Pop();
		}
	}

	if (len <= 0)
		return m_finalImage;

	sStreamingData *packet = (sStreamingData*)m_rcvBuffer;
	static int oldCompressed = -1;
	static int oldGray = -1;
	if (oldCompressed != (int)packet->isCompressed)
	{
		oldCompressed = (int)packet->isCompressed;
		std::cout << "compressed = " << (int)packet->isCompressed << std::endl;
	}
	if (oldGray != (int)packet->isGray)
	{
		oldGray = (int)packet->isGray;
		std::cout << "gray = " << (int)packet->isGray << std::endl;
	}


	if (packet->chunkSize == 1)
	{
		packet->data = m_rcvBuffer + sizeof(sStreamingData);

		if (packet->isCompressed)
		{
			memcpy((char*)m_src.data, packet->data, packet->imageBytes);
			cv::imdecode(m_src, 1, &m_finalImage);
		}
		else
		{
			// �ٷ� decode �� �����ؼ� �����Ѵ�.
			memcpy((char*)m_finalImage.data, packet->data, packet->imageBytes);
		}
	}
	else if (packet->chunkSize > 1)
	{
		static unsigned char oldId = 0;

		if (oldId != packet->id)
		{
			if (packet->isCompressed)
			{
				cv::imdecode(m_src, 1, &m_finalImage);
			}
			else
			{
				// �ٷ� decode �� �����ؼ� �����Ѵ�.
				memcpy((char*)m_finalImage.data, (char*)m_src.data, m_src.total() * m_src.elemSize());
			}

			oldId = packet->id;
		}

		packet->data = m_rcvBuffer + sizeof(sStreamingData);
		char *dst = (char*)m_src.data + packet->chunkIndex * sizePerChunk;
		const int copyLen = max(0, (len - (int)sizeof(sStreamingData)));
		memcpy(dst, packet->data, copyLen);
	}

	return m_finalImage;
}
