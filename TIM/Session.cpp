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
	Disconnect();
}

void Session::Init()
{
	u_long on = 0;
	int32 retVal = ioctlsocket(_socket, FIONBIO, &on);
	if (retVal == SOCKET_ERROR)
		CRASH("ioctlsocket Error");

	// 네이글 알고리즘 제거
	int nValue = 1;
	retVal = setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&nValue), sizeof(nValue));
	if (retVal != 0)
		CRASH("setsockopt");

	// 1.5초 시간 -> 타임아웃 부여
	DWORD time = NO_MSG_CHECK_TIME;
	retVal = setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&time), sizeof(time));
	if (retVal != 0)
		CRASH("setsockopt");
	retVal = setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&time), sizeof(time));
	if (retVal != 0)
		CRASH("setsockopt");
}

void Session::UpdateRecv()
{
	// 멈추는 기능 넣어주기
	while (GStart)
	{
		int32 _recvLen = recv(_socket, reinterpret_cast<char*>(_recvBuffer.WritePos()), _recvBuffer.FreeSize(), 0);
		if (_recvLen <= 0)
		{
			int32 errorCode = WSAGetLastError();
			// Check 필요
			if (errorCode == WSAETIMEDOUT || errorCode == WSAEWOULDBLOCK)
			{
				if (++_tickCount == GOverCount)
					break;
				continue;
			}
			break;
		}
		// KeepAlive 보내는 코드 구현하기
		// 구현 부분

		if (_recvBuffer.OnWrite(_recvLen) == false)
		{
			// OverFlow
			CRASH("OverFloaw");
			break;
		}

		// PktHead 사이즈보다 클 경우 OnRecv 함수 실행
		int32 processLen = OnRecv(_recvBuffer.ReadPos(), _recvLen);
		if (processLen < 0 || _recvBuffer.OnRead(processLen) == false)
			break;
		
		_tickCount = 0;

		_recvBuffer.Clean();
	}

	Disconnect();
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

	int32 retVal = closesocket(_socket);
	if (retVal != 0)
		CRASH("CloseSocket");

	_socket = INVALID_SOCKET;

	switch (GetPairState())
	{
	case ePairState::PairState_Unpair:
		TIM->PopPendingList(shared_from_this());
		break;
	case ePairState::PairState_Pair:
		TIM->PopPairingList(GetPairingId(), shared_from_this());
		break;
	}

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
					CRASH("Can't Send")
					continue;
			}
			Disconnect();
		}
		break;
	}
}

/*
//void Session::PrintSessionInfo()
//{
//	// 정보 출력
//	char ip[INET_ADDRSTRLEN] = {};
//	inet_ntop(AF_INET, &(_sockAddr.sin_addr), ip, INET_ADDRSTRLEN);
//	string str = "";
//	if (_trainType == TrainType::TIRD_)
//		str = "TIRD";
//	else
//		str = "TIFD";
//	cout << std::format("TrainNum : {0}\nTrainTypoe : {1}\nIPAddr : {2}\nPort : {3}", _myInfo.trainNum, str, ip, ntohs(_sockAddr.sin_port)) << endl;
//}
*/