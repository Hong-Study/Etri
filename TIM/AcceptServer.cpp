#include "pch.h"
#include "AcceptServer.h"
#include "TIMServer.h"
#include "WinApi.h"
#include "RecvBuffer.h"
#include "TifdSession.h"
#include "TirdSession.h"
#include "JsonParser.h"

void AcceptServer::Init()
{
	_listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSocket == INVALID_SOCKET)
		CRASH("Create Socket");

	_sockAddr.sin_family = AF_INET;

	IN_ADDR address;
	::InetPtonW(AF_INET, GServerIP.c_str(), &address);
	_sockAddr.sin_addr = address;

	u_long on = 1;
	int32 retVal = ioctlsocket(_listenSocket, FIONBIO, &on);
	if (retVal == SOCKET_ERROR)
		CRASH("ioctlsocket Error");

	_cv.tv_sec = 1;
	_cv.tv_usec = 0;
	_recvBuffer = make_shared<RecvBuffer>(BUFSIZE);
}

void AcceptServer::Update()
{
	// 멈추는 조건 설정
	_sockAddr.sin_port = htons(GServerPort);

	if (bind(_listenSocket, (SOCKADDR*)&_sockAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
		CRASH("Bind Error");
	if (listen(_listenSocket, SOMAXCONN) == SOCKET_ERROR)
		CRASH("Listen Error");

	while (GStart)
	{
		if (_listenSocket == INVALID_SOCKET)
			break;

		FD_ZERO(&_rds);
		FD_SET(_listenSocket, &_rds);
		for (auto it = _infos.begin(); it != _infos.end();)
		{
			if (it->sock == INVALID_SOCKET)
				it = _infos.erase(it);
			else
			{
				FD_SET(it->sock, &_rds);
				it++;

			}
		}

		int32 retVal = select(0, &_rds, (fd_set*)0, (fd_set*)0, &_cv);
		if (retVal == SOCKET_ERROR)
		{
			int32 errorCode = WSAGetLastError();
			if (errorCode == WSAETIMEDOUT || errorCode == WSAEWOULDBLOCK)
				continue;
			break;
		}
		if (FD_ISSET(_listenSocket, &_rds))
		{
			SOCKADDR_IN addr;
			ZeroMemory(&addr, sizeof(addr));
			int32 addrLen = sizeof(SOCKADDR_IN);
			SOCKET clientSock = accept(_listenSocket, reinterpret_cast<sockaddr*>(&addr), &addrLen);
			if (clientSock == INVALID_SOCKET)
			{
				// TODO
				continue;
			}
			else
			{
				char clientIP[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &(addr.sin_addr), clientIP, INET_ADDRSTRLEN);
				wstring str = std::format(L"Connected {0}:{1}", StringToWstring(clientIP), ntohs(addr.sin_port));
				WINGUI->AddLogList(str);

				SendBufferRef ref = MakeSendBuffer(CommandType::CmdType_Reg_Request);
				if (Send(clientSock, ref) == true)
					_infos.push_back({ clientSock, addr });
				else
					closesocket(clientSock);
			}
		}
		for (SessionInfo& info : _infos)
		{
			if (FD_ISSET(info.sock, &_rds))
			{
				Recv(info);
			}
		}
	}
}

void AcceptServer::Disconnect(SessionInfo& info)
{
	if (info.sock == INVALID_SOCKET)
		return;

	closesocket(info.sock);
	info.sock = INVALID_SOCKET;
	wstring str = std::format(L"Disconnect Unknow Clinet");
	WINGUI->AddLogList(str);
}

void AcceptServer::Clear()
{
	if (_listenSocket == INVALID_SOCKET)
		return;
	int32 retVal = closesocket(_listenSocket);
	_listenSocket = INVALID_SOCKET;

	for (SessionInfo& info : _infos)
		closesocket(info.sock);

	_recvBuffer->Clean();
	_infos.clear();
}

void AcceptServer::Recv(SessionInfo& info)
{
	int32 _recvLen = recv(info.sock, reinterpret_cast<char*>(_recvBuffer->WritePos()), _recvBuffer->FreeSize(), 0);
	if (_recvLen < 0)
	{
		Disconnect(info);
		return;
	}

	if (_recvBuffer->OnWrite(_recvLen) == false)
	{
		// OverFlow
		Disconnect(info);
		return;
	}

	// PktHead 사이즈보다 클 경우 OnRecv 함수 실행
	int32 processLen = OnRecv(info, _recvBuffer->ReadPos(), _recvLen);
	if (processLen < 0 || _recvBuffer->OnRead(processLen) == false)
	{
		Disconnect(info);
		return;
	}

	_recvBuffer->Clean();
}

int32 AcceptServer::OnRecv(SessionInfo& info, BYTE* buffer, int32 size)
{
	int32 processLen = 0;
	while (true)
	{
		int32 dataSize = size - processLen;

		if (HEAD_SIZE > dataSize)
		{
			break;
		}

		PktHead* head = reinterpret_cast<PktHead*>(&buffer[processLen]);

		if ((head->payloadSize + HEAD_SIZE) > dataSize)
			break;
		int total = HEAD_SIZE + head->payloadSize;
		OnRecvPacket(info, &buffer[processLen], total);

		processLen += total;
	}

	return processLen;
}

void AcceptServer::OnRecvPacket(SessionInfo& info, BYTE* buffer, int32 size)
{
	PktHead* head = reinterpret_cast<PktHead*>(buffer);
	if (head->command == CommandType::CmdType_Reg_Request)
	{
		if (head->category == Device::DeviceTIFD)
		{
			HandleTifdConnect(info, reinterpret_cast<StTifdData*>(&head[1]));
		}
		else if (head->category == Device::DeviceTIRD)
		{
			HandleTirdConnect(info, reinterpret_cast<StTirdData*>(&head[1]));
		}
	}
	else
	{
		Disconnect(info);
	}
}

bool AcceptServer::Send(SOCKET sock, SendBufferRef buffer)
{
	return Send(sock, buffer->Data(), buffer->Size());
}

bool AcceptServer::Send(SOCKET sock, BYTE* buffer, int32 size)
{
	int32 tickCount = 0;
	while (true)
	{
		int32 retVal = send(sock, reinterpret_cast<char*>(buffer), size, 0);
		if (retVal <= 0)
		{
			int32 errorCode = WSAGetLastError();
			if (errorCode == WSAETIMEDOUT || errorCode == WSAEWOULDBLOCK)
			{
				if (++tickCount == 5)
					CRASH("Can't Send")
					continue;
				return false;
			}
		}
		break;
	}

	return true;
}

void AcceptServer::HandleTifdConnect(SessionInfo& info, const StTifdData* data)
{
	TifdRef session = make_shared<TifdSession>(info.sock, info.sockAddr, data);

	if (TIM->PushNewSession(session) == true)
	{
		info.sock = INVALID_SOCKET;
		wstring str = std::format(L"Connected TIFD {0}", StringToWstring(data->deviceId));
		WINGUI->AddLogList(str);
	}
	else
	{
		Disconnect(info);
		session->SetSocket(INVALID_SOCKET);
	}
}

void AcceptServer::HandleTirdConnect(SessionInfo& info, const StTirdData* data)
{
	TirdRef session = make_shared<TirdSession>(info.sock, info.sockAddr, data);

	if (TIM->PushNewSession(session) == true)
	{
		info.sock = INVALID_SOCKET;
		wstring str = std::format(L"Connected TIRD {0}", StringToWstring(data->deviceId));
		WINGUI->AddLogList(str);
	}
	else
	{
		Disconnect(info);
		session->SetSocket(INVALID_SOCKET);
	}
}
