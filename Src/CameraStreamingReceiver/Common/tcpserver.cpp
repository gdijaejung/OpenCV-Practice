
#include "stdafx.h"
#include "tcpserver.h"
#include <iostream>
#include <process.h> 
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

using namespace std;
using namespace network;

unsigned WINAPI TCPServerThreadFunction(void* arg);


cTCPServer::cTCPServer()
	: m_isConnect(false)
	, m_threadLoop(true)
	, m_maxBuffLen(BUFFER_LENGTH)
	, m_sleepMillis(10)
{
	InitializeCriticalSectionAndSpinCount(&m_criticalSection, 0x00000400);
}

cTCPServer::~cTCPServer()
{
	Close();

	DeleteCriticalSection(&m_criticalSection);
}


bool cTCPServer::Init(const int port, const int packetSize, const int maxPacketCount, const int sleepMillis)
// packetSize=512, maxPacketCout=10, sleepMillis=30
{
	Close();

	m_port = port;
	m_sleepMillis = sleepMillis;
	m_maxBuffLen = packetSize;

	if (network::LaunchServer(port, m_svrSocket))
	{
		cout << "Bind TCP Server = " << "port= " << port << endl;

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
			m_handle = (HANDLE)_beginthreadex(NULL, 0, TCPServerThreadFunction, this, 0, (unsigned*)&m_threadId);
		}
	}
	else
	{
		cout << "Error!! Bind TCP Server port=" << port << endl;
		return false;
	}

	return true;
}


void cTCPServer::Close()
{
	m_isConnect = false;
	m_threadLoop = false;
	if (m_handle)
	{
		::WaitForSingleObject(m_handle, 1000);
		m_handle = NULL;
	}
	closesocket(m_svrSocket);
	m_svrSocket = INVALID_SOCKET;
}


void cTCPServer::MakeFdSet(OUT fd_set &out)
{
	FD_ZERO(&out);
	for each (auto &session in m_sessions)
	{
		FD_SET(session.socket, (fd_set*)&out);
	}
}


bool cTCPServer::AddSession(const SOCKET remoteSock)
{
	cAutoCS cs(m_criticalSection);

	for each (auto &session in m_sessions)
	{
		if (remoteSock == session.socket)
			return false; // �̹� �ִٸ� ����.
	}

	sSession session;
	session.state = SESSION_STATE::LOGIN_WAIT;
	session.socket = remoteSock;
	m_sessions.push_back(session);
	return true;
}


void cTCPServer::RemoveSession(const SOCKET remoteSock)
{
	cAutoCS cs(m_criticalSection);

	for (u_int i = 0; i < m_sessions.size(); ++i)
	{
		if (remoteSock == m_sessions[i].socket)
		{
			closesocket(m_sessions[i].socket);
			common::rotatepopvector(m_sessions, i);
			break;
		}
	}
}


unsigned WINAPI TCPServerThreadFunction(void* arg)
{
	cTCPServer *server = (cTCPServer*)arg;

	char *buff = new char[server->m_maxBuffLen];
 	const int maxBuffLen = server->m_maxBuffLen;

	int lastAcceptTime = timeGetTime();

	while (server->m_threadLoop)
	{
		const timeval t = { 0, server->m_sleepMillis };

		// Accept�� ������ ó���Ѵ�.
		const int curT = timeGetTime();
		if (curT - lastAcceptTime > 300)
		{
			lastAcceptTime = curT;

			//-----------------------------------------------------------------------------------
			// Accept Client
			fd_set acceptSockets;
			FD_ZERO(&acceptSockets);
			FD_SET(server->m_svrSocket, &acceptSockets);
			const int ret1 = select(acceptSockets.fd_count, &acceptSockets, NULL, NULL, &t);
			if (ret1 != 0 && ret1 != SOCKET_ERROR)
			{
				// accept(��û�� ���� ����, ���� Ŭ���̾�Ʈ �ּ�)
				SOCKET remoteSocket = accept(acceptSockets.fd_array[0], NULL, NULL);
				if (remoteSocket == INVALID_SOCKET)
				{
	 				//clog::Error(clog::ERROR_CRITICAL, "Client�� Accept�ϴ� ���߿� ������ �߻���\n");
					continue;
				}
				server->AddSession(remoteSocket);			
			}
			//-----------------------------------------------------------------------------------
		}


		//-----------------------------------------------------------------------------------
		// Receive Packet
		fd_set readSockets;
		server->MakeFdSet(readSockets);
		const fd_set sockets = readSockets;

		const int ret = select(readSockets.fd_count, &readSockets, NULL, NULL, &t);
		if (ret != 0 && ret != SOCKET_ERROR)
		{
			for (u_int i = 0; i < sockets.fd_count; ++i)
			{
				if (!FD_ISSET(sockets.fd_array[i], &readSockets)) 
					continue;

				const int result = recv(sockets.fd_array[i], buff, maxBuffLen, 0);
				if (result == SOCKET_ERROR || result == 0) // ���� ��Ŷ����� 0�̸� ������ ������ ����ٴ� �ǹ̴�.
				{
					server->RemoveSession(sockets.fd_array[i]);
				}
				else
				{
					//cout << "packet recv size = " << result << endl;
					server->m_recvQueue.Push(sockets.fd_array[i], (BYTE*)buff, result, true);
				}
			}
		}
		//-----------------------------------------------------------------------------------


		//-----------------------------------------------------------------------------------
		// Send Packet
		server->m_sendQueue.SendAll();
		//-----------------------------------------------------------------------------------
	}

	delete[] buff;
	return 0;
}
