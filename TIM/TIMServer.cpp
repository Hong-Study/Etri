#include "pch.h"
#include "TIMServer.h"
#include "ThreadManager.h"
#include "PairSession.h"
#include "WinApi.h"
#include "TifdSession.h"
#include "TirdSession.h"
#include "AcceptServer.h"

using std::string;
void TIMServer::Init()
{
	FD_ZERO(&_fds);
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
		SERVER->Update();
		});

	THREAD->Push([=]() {
		TIM->Update();
		});
}

void TIMServer::Update()
{
	SendKeepAliveUpdate();

	timeval cv;
	cv.tv_sec = 0;
	cv.tv_usec = 10000;

	while (_isWork)
	{
		Execute();

		if (_totalSize == 0)
			continue;

		fd_set tempFds = _fds;

		int32 retVal = select(0, &tempFds, (fd_set*)0, (fd_set*)0, &cv);

		if (retVal == SOCKET_ERROR)
		{
			int32 errorCode = WSAGetLastError();
			if (errorCode == WSAETIMEDOUT || errorCode == WSAEWOULDBLOCK || errorCode == 10022)
				continue;
			break;
		}

		for (auto begin = _tifdList.begin();begin != _tifdList.end();begin++)
		{
			TifdRef tifd = begin->second;
			SOCKET sock = tifd->GetSocket();
			if (FD_ISSET(sock, &_fds))
				tifd->Recv();
		}

		for (auto begin = _tirdList.begin(); begin != _tirdList.end(); begin++)
		{
			TirdRef tird = begin->second;
			SOCKET sock = tird->GetSocket();
			if (FD_ISSET(sock, &_fds))
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
		closesocket(sock);
		return;
	}

	FD_SET(sock, &_fds);
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
		closesocket(sock);
		return;
	}

	FD_SET(sock, &_fds);
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