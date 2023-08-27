#include "pch.h"
#include "TIMServer.h"
#include "ThreadManager.h"
#include "PairSession.h"
#include "WinApi.h"
#include "TifdSession.h"
#include "TirdSession.h"

using std::string;
void TIMServer::Init()
{
	_listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSocket == INVALID_SOCKET)
		CRASH("Create Socket");

	ZeroMemory(&_sockAddr, sizeof(SOCKADDR_IN));

	_sockAddr.sin_family = AF_INET;

	IN_ADDR address;
	::InetPtonW(AF_INET, GServerIP.c_str(), &address);
	_sockAddr.sin_addr = address;

	u_long on = 1;
	int32 retVal = ioctlsocket(_listenSocket, FIONBIO, &on);
	if (retVal == SOCKET_ERROR)
		CRASH("ioctlsocket Error");

	FD_ZERO(&_fds);
	_recvBuffer = make_shared<RecvBuffer>(1024);
	_infos.reserve(64);
}

void TIMServer::Clear()
{
	for (auto tifd : _tifdList)
	{
		tifd.second->SetPairState(ePairState::Disconnect);
		tifd.second->Disconnect();
	}

	for (auto tird : _tirdList)
	{
		tird.second->SetPairState(ePairState::Disconnect);
		tird.second->Disconnect();
	}

	_tifdList.clear();
	_tirdList.clear();
	_pairingSessions.clear();

	ClearJobs();

	FD_ZERO(&_fds);
	_isWork = false;
	_totalSize.store(0);
}

void TIMServer::PopList(SessionRef ref)
{
	if (ref->GetDeviceType() == DeviceTIFD)
		PopTifdList(static_pointer_cast<TifdSession>(ref));
	else if(ref->GetDeviceType() == DeviceTIRD)
		PopTirdList(static_pointer_cast<TirdSession>(ref));
}

void TIMServer::PopTifdList(TifdRef tifd)
{
	SOCKET sock = tifd->GetSocket();
	string deviceId = tifd->GetDeviceIdToString();

	if (sock == INVALID_SOCKET)
		return;

	auto it = _tifdList.find(deviceId);
	if (it == _tifdList.end())
		return;

	WINGUI->DoAsync(&WinApi::DeleteTifdPendingList, tifd->GetListId());

	TIM->DoAsync([=]() {
		FD_CLR(sock, &_fds);
		_tifdList.erase(deviceId);
		_totalSize -= 1;
		});
}

void TIMServer::PopTirdList(TirdRef tird)
{
	SOCKET sock = tird->GetSocket();
	string deviceId = tird->GetDeviceIdToString();

	if (sock == INVALID_SOCKET)
		return;

	auto it = _tirdList.find(deviceId);
	if (it == _tirdList.end())
		return;

	WINGUI->DoAsync(&WinApi::DeleteTirdPendingList, tird->GetListId());

	TIM->DoAsync([=]() {
		FD_CLR(sock, &_fds);
		_tirdList.erase(deviceId);
		_totalSize -= 1;
		});
}

bool TIMServer::PushPairingList(TifdRef tifd, TirdRef tird, int32 distance)
{
	if (tifd == nullptr || tird == nullptr)
		return false;

	if (tird->GetDeviceType() != Device::DeviceTIRD || tifd->GetDeviceType() != Device::DeviceTIFD)
		return false;
	if (tird->GetPairState() == ePairState::PairState_Pair || tifd->GetPairState() == ePairState::PairState_Pair)
		return false;

	auto tifdData = tifd->GetData();
	auto tirdData = tird->GetData();
	
	if (tifdData == nullptr || tirdData == nullptr)
		return false;

	// 정보 저장 및 세션 옮김
	{
		tifd->SetTarget(tird);
		tird->SetTarget(tifd);

		int32 pairingId = WINGUI->NewPairingList(tifd->GetListId(), tird->GetListId(), distance);
		PairSessionRef pair = make_shared<PairSession>(tifd, tird, pairingId, distance);

		tifd->SetPairingId(pairingId);
		tird->SetPairingId(pairingId);

		auto it = _pairingSessions.insert({ pairingId, pair });
		if (it.second == false)
			CRASH("Pairing already inside");

		WINGUI->DoAsync(&WinApi::DeleteTifdPendingList, tifd->GetListId());
		WINGUI->DoAsync(&WinApi::DeleteTirdPendingList, tird->GetListId());
	}

	wstring str = std::format(L"Pairing On : TIFD = {0}, TIRD = {1}"
		, StringToWstring(tifdData->deviceId)
		, StringToWstring(tirdData->deviceId));
	WINGUI->DoAsync(&WinApi::AddLogList, str);
	
	// Pairing Send
	{
		SendBufferRef buf = MakeSendPairingBuffer(tirdData->deviceId, tifdData->trainLength);
		tifd->Send(buf);
	}

	// LoraCH Send
	{
		SendBufferRef buf = MakeSendLoraBuffer(_loraInfo.ch);
		tifd->Send(buf);
		tird->Send(buf);
		_loraInfo.bUse[_loraInfo.ch-1] = true;
		_loraInfo.ch += 1;
		if (_loraInfo.ch > LORA_MAX_CH)
			_loraInfo.ch = 2;
	}

	return true;
}

void TIMServer::PopPairingList(int32 pairingId, SessionRef session)
{
	if (session->GetDeviceType() == Device::DeviceTIFD)
		PopPairingList(pairingId, dynamic_pointer_cast<TifdSession>(session));
	else if (session->GetDeviceType() == Device::DeviceTIRD)
		PopPairingList(pairingId, dynamic_pointer_cast<TirdSession>(session));
}

void TIMServer::PopPairingList(int32 pairingId, TifdRef session)
{
	auto it = _pairingSessions.find(pairingId);
	if (it == _pairingSessions.end())
		return;
	else
	{
		PairSessionRef pairRef = it->second;
		pairRef->Disconnected();

		WINGUI->DeletePairingList(pairRef->GetPairingId(), session->GetDeviceType());

		{
			auto tird = pairRef->GetTirdSession();
			SendBufferRef sendBuf = MakeSendLoraBuffer(LORA_DEFAULT_CH);
			tird->Send(sendBuf);
		}

		wstring str = std::format(L"Pairing Off : tifd Disconnected {0}", session->GetDeviceIdToWString());
		WINGUI->DoAsync(&WinApi::AddLogList, str);

		_pairingSessions.erase(pairingId);
		PopTifdList(session);
	}
}

void TIMServer::PopPairingList(int32 pairingId, TirdRef session)
{
	auto it = _pairingSessions.find(pairingId);
	if (it == _pairingSessions.end())
		return;
	else 
	{
		PairSessionRef pairRef = it->second;
		pairRef->Disconnected();

		WINGUI->DeletePairingList(pairRef->GetPairingId(), session->GetDeviceType());

		{
			// TODO 체크
			auto tifd = pairRef->GetTifdSession();

			SendBufferRef sendBuf = MakeSendUnPairingBuffer();
			tifd->Send(sendBuf);
			sendBuf = MakeSendLoraBuffer(LORA_DEFAULT_CH);
			tifd->Send(sendBuf);
		}

		wstring str = std::format(L"Pairing Off : tird Disconnected {0}", session->GetDeviceIdToWString());
		WINGUI->DoAsync(&WinApi::AddLogList, str);

		_pairingSessions.erase(it);
		PopTirdList(session);
	}
}

void TIMServer::Disconnect(SessionInfo* info)
{
	if (info->sock == INVALID_SOCKET)
		return;

	FD_CLR(info->sock, &_fds);

	closesocket(info->sock);
	info->sock = INVALID_SOCKET;

	wstring str = std::format(L"Disconnect Unknow Clinet");
	WINGUI->DoAsync(&WinApi::AddLogList, str);
}

void TIMServer::Recv(SessionInfo* info)
{
	int32 _recvLen = recv(info->sock, reinterpret_cast<char*>(_recvBuffer->WritePos()), _recvBuffer->FreeSize(), 0);
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

int32 TIMServer::OnRecv(SessionInfo* info, BYTE* buffer, int32 size)
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

void TIMServer::OnRecvPacket(SessionInfo* info, BYTE* buffer, int32 size)
{
	PktHead* head = reinterpret_cast<PktHead*>(buffer);
	if (head->command == CommandType::CmdType_Reg_Request)
	{
		if (head->category == Device::DeviceTIFD)
		{
			PushTifdList(info->sock, info->sockAddr, reinterpret_cast<const StTifdData*>(&head[1]));
			info->sock = INVALID_SOCKET;
		}
		else if (head->category == Device::DeviceTIRD)
		{
			PushTirdList(info->sock, info->sockAddr, reinterpret_cast<const StTirdData*>(&head[1]));
			info->sock = INVALID_SOCKET;
		}
	}
	else
	{
		Disconnect(info);
	}
}

bool TIMServer::Send(SOCKET sock, SendBufferRef buffer)
{
	return Send(sock, buffer->Data(), buffer->Size());
}

bool TIMServer::Send(SOCKET sock, BYTE* buffer, int32 size)
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

void TIMServer::SendKeepAliveUpdate()
{
	THREAD->Push([=]() {
		uint64 tick = GetTickCount64();
		while (_isWork)
		{
			uint64 nowTick = GetTickCount64();
			if (nowTick - tick > 3000)
			{
				tick = nowTick;
				TIM->DoAsync(&TIMServer::SendKeepAlive);
			}
		}
	});
}

void TIMServer::SendKeepAlive()
{
	SendBufferRef ref = MakeSendBuffer(CommandType::CmdType_KeepAlive);

	for (auto begin = _tifdList.begin();begin != _tifdList.end();begin++)
	{
		begin->second->Send(ref);
	}

	for (auto begin = _tirdList.begin(); begin != _tirdList.end(); begin++)
	{
		begin->second->Send(ref);
	}
}

void TIMServer::GetPossiblePairingList(pair<float, float> tifdLocation, vector<PossiblePairingList>& lists)
{
	for (auto tirds : _tirdList)
	{
		bool possible = true;
		TirdRef tird = tirds.second;

		if (tird->GetPairState() == PairState_Pair)
			continue;

		for (PossiblePairingList& list : lists)
		{
			if (list.target == tird)
			{
				possible = false;
				break;
			}
		}

		if (possible)
		{
			// 속도 체크 해야하는가?
			auto tirdLocation = tird->GetLocation();

			// GPS 오류 체크 -> GPS가 제대로 안들어왔을 경우.
			// 오류 길이 정도를 체크
			int32 distance = CalculateDistance(tifdLocation, tirdLocation);

			if (distance < GMaximumDistance)
			{
				PossiblePairingList item{ tird, distance };
				lists.push_back(item);
			}
		}
	}
}

void TIMServer::Start()
{
	THREAD->Push([=]() {
		TIM->Update();
		});

	SendKeepAliveUpdate();
}

void TIMServer::Update()
{
	_sockAddr.sin_port = htons(GServerPort);

	if (bind(_listenSocket, (SOCKADDR*)&_sockAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
		CRASH("Bind Error");
	if (listen(_listenSocket, SOMAXCONN) == SOCKET_ERROR)
		CRASH("Listen Error");

	FD_SET(_listenSocket, &_fds);

	timeval cv;
	cv.tv_sec = 0;
	cv.tv_usec = 10000;

	while (_isWork)
	{
		Execute();

		fd_set tempSet = _fds;

		int32 retVal = select(0, &tempSet, (fd_set*)0, (fd_set*)0, &cv);

		if (retVal == SOCKET_ERROR)
		{
			int32 errorCode = WSAGetLastError();
			if (errorCode == WSAETIMEDOUT || errorCode == WSAEWOULDBLOCK)
				continue;
			break;
		}

		if (FD_ISSET(_listenSocket, &tempSet))
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
				WINGUI->DoAsync(&WinApi::AddLogList, str);

				SendBufferRef ref = MakeSendBuffer(CommandType::CmdType_Reg_Request);

				if (Send(clientSock, ref) == true)
				{
					_infos.push_back({ clientSock, addr });
					FD_SET(clientSock, &_fds);
				}
				else
					closesocket(clientSock);
			}
		}

		for (auto info = _infos.begin();info != _infos.end();)
		{
			if (info->sock == INVALID_SOCKET)
				info = _infos.erase(info);
			else if (FD_ISSET(info->sock, &tempSet))
			{
				Recv(info._Ptr);
				info++;
			}
			else
				info++;
		}

		for (auto begin = _tifdList.begin();begin != _tifdList.end();begin++)
		{
			TifdRef tifd = begin->second;
			SOCKET sock = tifd->GetSocket();
			if (FD_ISSET(sock, &tempSet))
				tifd->Recv();
		}

		for (auto begin = _tirdList.begin(); begin != _tirdList.end(); begin++)
		{
			TirdRef tird = begin->second;
			SOCKET sock = tird->GetSocket();
			if (FD_ISSET(sock, &tempSet))
				tird->Recv();
		}
	}
}

void TIMServer::PushTifdList(SOCKET sock, SOCKADDR_IN sockAddr, const StTifdData* data)
{
	string name = data->deviceId;

	auto it = _tifdList.find(name);
	if (it != _tifdList.end())
	{
		FD_CLR(sock, &_fds);
		closesocket(sock);
		return;
	}

	_totalSize.fetch_add(1);

	TifdRef tifd = make_shared<TifdSession>(sock, sockAddr, data);
	SendBufferRef buffer = MakeSendBuffer(CommandType::CmdType_Reg_Confirm);
	tifd->Send(buffer);
	_tifdList.insert({ name, tifd });

	wstring str = std::format(L"Connected TIFD {0}", StringToWstring(name));
	WINGUI->DoAsync(&WinApi::AddLogList, str);

	int32 listId = WINGUI->NewPendingList(tifd);
	tifd->SetListId(listId);
}

void TIMServer::PushTirdList(SOCKET sock, SOCKADDR_IN sockAddr, const StTirdData* data)
{
	string name = data->deviceId;

	auto it = _tirdList.find(name);
	if (it != _tirdList.end())
	{
		FD_CLR(sock, &_fds);
		closesocket(sock);
		return;
	}

	_totalSize.fetch_add(1);

	TirdRef tird = make_shared<TirdSession>(sock, sockAddr, data);
	SendBufferRef buffer = MakeSendBuffer(CommandType::CmdType_Reg_Confirm);
	tird->Send(buffer);
	_tirdList.insert({ name, tird });

	wstring str = std::format(L"Connected TIRD {0}", StringToWstring(name));
	WINGUI->DoAsync(&WinApi::AddLogList, str);

	int32 listId = WINGUI->NewPendingList(tird);
	tird->SetListId(listId);
}