#include "pch.h"
#include "Session.h"
#include "TIMServer.h"
#include "WinApi.h"

Session::Session(SOCKET sock, SOCKADDR_IN addr, Device device)
	: _socket(sock), _sockAddr(addr), _recvBuffer(BUFSIZE)
	, _deviceType(device), _pairState(ePairState::PairState_Unpair)
{
	
}

Session::~Session()
{
	if(_socket != INVALID_SOCKET)
		Disconnect();
}

void Session::Init()
{
	int nValue = 1;
	int32 retVal = setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&nValue), sizeof(nValue));
	if (retVal != 0)
		CRASH("setsockopt"); 

	DWORD time = NO_MSG_CHECK_TIME;
	retVal = setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&time), sizeof(time));
	if (retVal != 0)
		CRASH("setsockopt");

	_currentRecvTime = GetTickCount64();
}

void Session::Recv()
{

	int32 _recvLen = recv(_socket, reinterpret_cast<char*>(_recvBuffer.WritePos()), _recvBuffer.FreeSize(), 0);
	if (_recvLen <= 0)
	{
		int32 errorCode = WSAGetLastError();
		// Check �ʿ�
		if (errorCode == WSAEWOULDBLOCK)
		{
			return;
		}

		Disconnect();
		return;
	}

	if (_recvBuffer.OnWrite(_recvLen) == false)
	{
		// OverFlow
		Disconnect();
	}

	int32 processLen = OnRecv(_recvBuffer.ReadPos(), _recvLen);
	if (processLen < 0 || _recvBuffer.OnRead(processLen) == false)
		Disconnect();

	_tickCount = 0;

	_recvBuffer.Clean();
}

int32 Session::OnRecv(BYTE* buffer, int32 size)
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

		OnRecvPacket(&buffer[processLen], total);

		processLen += total;
	}
	
	return processLen;
}

void Session::Disconnect()
{
	if (_socket == INVALID_SOCKET)
		return;

	if (_pairState == ePairState::PairState_Pair)
	{
		TIM->PopPairingList(_pairingId, shared_from_this());
	}
	else if (_pairState == ePairState::PairState_Unpair)
	{
		TIM->PopList(shared_from_this());
	}

	int32 retVal = closesocket(_socket);
	if (retVal != 0)
		CRASH("CloseSocket");

	_socket = INVALID_SOCKET;

	OnDisconnected();
}

void Session::Send(SendBufferRef buffer)
{
	Send(buffer->Data(), buffer->Size());
}

void Session::Send(BYTE* buffer, int32 size)
{
	WRITE_LOCK;

	int32 tickCount = 0;
	while (true)
	{
		int32 retVal = send(_socket, reinterpret_cast<char*>(buffer), size, 0);
		if (retVal <= 0)
		{
			int32 errorCode = WSAGetLastError();
			if (errorCode == WSAETIMEDOUT || errorCode == WSAEWOULDBLOCK)
			{
				if (++tickCount == 5)
				{
					Disconnect();
					break;
				}
				continue;
			}
			Disconnect();
		}
		break;
	}
}