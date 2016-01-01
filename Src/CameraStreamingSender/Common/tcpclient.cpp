
#include "stdafx.h"
#include "tcpclient.h"
#include <iostream>
#include <process.h> 

using namespace std;
using namespace network;

unsigned WINAPI TCPClientThreadFunction(void* arg);


cTCPClient::cTCPClient()
: m_isConnect(false)
, m_threadLoop(true)
, m_handle(NULL)
, m_sleepMillis(30)
, m_maxBuffLen(BUFFER_LENGTH)
{
}

cTCPClient::~cTCPClient()
{
	Close();
}


bool cTCPClient::Init(const string &ip, const int port, 
	const int packetSize, const int maxPacketCount, const int sleepMillis)
	// packetSize = 512, maxPacketCount = 10, int sleepMillis = 30
{
	Close();

	m_ip = ip;
	m_port = port;
	m_sleepMillis = sleepMillis;
	m_maxBuffLen = packetSize;

	if (network::LaunchClient(ip, port, m_socket))
	{
		cout << "Connect TCP/IP Client ip= " << ip << ", port= " << port << endl;

		if (!m_recvQueue.Init(packetSize, maxPacketCount))
		{
			Close();
			return false;
		}

		if (!m_sendQueue.Init(packetSize, maxPacketCount))
		{
			Close();
			return false;
		}

 		m_isConnect = true;
 		m_threadLoop = true;
		if (!m_handle)
		{
			m_handle = (HANDLE)_beginthreadex(NULL, 0, TCPClientThreadFunction, this, 0, (unsigned*)&m_threadId);
		}
	}
	else
	{
		cout << "Error!! Connect TCP/IP Client ip=" << ip << ", port=" << port << endl;
		return false;
	}

	return true;
}


void cTCPClient::Send(BYTE *buff, const int len)
{
	RET(!m_isConnect);
	m_sendQueue.Push(m_socket, buff, len);
}


void cTCPClient::Close()
{
	m_isConnect = false;
	m_threadLoop = false;
	if (m_handle)
	{
		::WaitForSingleObject(m_handle, 1000);
		m_handle = NULL;
	}
	closesocket(m_socket);
	m_socket = INVALID_SOCKET;
}


unsigned WINAPI TCPClientThreadFunction(void* arg)
{
	cTCPClient *client = (cTCPClient*)arg;
	char *buff = new char[client->m_maxBuffLen];
	const int maxBuffLen = client->m_maxBuffLen;

	while (client->m_threadLoop)
	{
		const timeval t = { 0, client->m_sleepMillis };
		
		//-----------------------------------------------------------------------------------
		// Receive Packet
		fd_set readSockets;
		FD_ZERO(&readSockets);
		FD_SET(client->m_socket, &readSockets);
		const int ret = select(readSockets.fd_count, &readSockets, NULL, NULL, &t);
		if (ret != 0 && ret != SOCKET_ERROR)
		{
			const int result = recv(readSockets.fd_array[0], buff, maxBuffLen, 0);
			if (result == SOCKET_ERROR || result == 0) // ���� ��Ŷ����� 0�̸� ������ ������ ����ٴ� �ǹ̴�.
			{
				// error occur
				client->m_isConnect = false;
				client->m_threadLoop = false;
				cout << "cTCPClient socket close " << endl;
			}
			else
			{
				//cout << "recv packet size = " << result << endl;
				client->m_recvQueue.Push(readSockets.fd_array[0], (BYTE*)buff, result, true);
			}
		}
		//-----------------------------------------------------------------------------------


		//-----------------------------------------------------------------------------------
		// Send Packet
		client->m_sendQueue.SendAll();
		//-----------------------------------------------------------------------------------
	}

	delete[] buff;
	return 0;
}
