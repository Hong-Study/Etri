#include "pch.h"
#include "TIRD_Select.h"
#include "TIM_Server.h"
#include "Session.h"

void TIRD_Select::Init()
{
	_listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSocket == INVALID_SOCKET)
		CRASH("Create Socket");

	_sockAddr.sin_port = htons(GServerPort);

	_sockAddr.sin_family = AF_INET;

	IN_ADDR address;
	::InetPtonW(AF_INET, IP, &address);
	_sockAddr.sin_addr = address;

	if (bind(_listenSocket, (SOCKADDR*)&_sockAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
		CRASH("Bind Error");
	if (listen(_listenSocket, SOMAXCONN) == SOCKET_ERROR)
		CRASH("Listen Error");

	u_long on = 1;
	int32 retVal = ioctlsocket(_listenSocket, FIONBIO, &on);
	if (retVal == SOCKET_ERROR)
		CRASH("ioctlsocket Error");

	_cv.tv_sec = 1;
	_cv.tv_usec = 0;
}

void TIRD_Select::Update()
{
	FD_ZERO(&_fds);
	FD_SET(_listenSocket, &_fds);

	// 멈추는 조건 설정
	while (true)
	{
		if (_listenSocket == INVALID_SOCKET)
			break;

		fd_set copy = _fds;
		int32 retVal = select(0, &copy, (fd_set*)0, (fd_set*)0, &_cv);
		if (retVal == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAETIMEDOUT)
				continue;
			break;
		}
		if (FD_ISSET(_listenSocket, &copy))
		{
			SOCKADDR_IN sockAddr;
			ZeroMemory(&sockAddr, sizeof(SOCKADDR_IN));
			int32 addrLen = sizeof(sockAddr);
			SOCKET clientSock = accept(_listenSocket, reinterpret_cast<sockaddr*>(&sockAddr), &addrLen);
			if (clientSock == INVALID_SOCKET)
			{
				continue;
			}
			else
			{
				SessionRef session = make_shared<Session>(clientSock, sockAddr);
				TIM->PushUpdate(session);
			}
		}
	}
	Disconnect();
}

void TIRD_Select::Disconnect()
{
	if (_listenSocket == INVALID_SOCKET)
		return;
	int32 retVal = closesocket(_listenSocket);
	_listenSocket = INVALID_SOCKET;
}