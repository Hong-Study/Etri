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
	
}

void TIMServer::Clear()
{
	WRITE_LOCK_IDX(TIFD);
	WRITE_LOCK_IDX(TIRD);
	WRITE_LOCK_IDX(PAIRING);
	
	for (auto ref : _pendingTifd)
	{
		ref.second->SetPairState(ePairState::Disconnect);
		ref.second->Disconnect();
	}

	for (auto ref : _pendingTird)
	{
		ref.second->SetPairState(ePairState::Disconnect);
		ref.second->Disconnect();
	}
	
	for (auto ref : _pairingSessions)
	{
		ref.second->GetTifdSession()->SetPairState(ePairState::Disconnect);
		ref.second->GetTifdSession()->Disconnect();

		ref.second->GetTirdSession()->SetPairState(ePairState::Disconnect);
		ref.second->GetTirdSession()->Disconnect();
	}

	_pendingTifd.clear();
	_pendingTird.clear();
	_pairingSessions.clear();
}

bool TIMServer::PushPendingList(TifdRef tifd, string deviceId)
{
	WRITE_LOCK_IDX(TIFD);

	if (_pendingTifd.find(deviceId) != _pendingTifd.end() || tifd == nullptr)
	{
		return false;
	}
	
	if (tifd->GetListId() == 0)
	{
		int32 id = WINGUI->NewPendingList(tifd);
		tifd->SetListId(id);
	}
	
	_pendingTifd.insert({ deviceId, tifd });

	return true;
}

bool TIMServer::PushPendingList(TirdRef tird, string deviceId)
{
	WRITE_LOCK_IDX(TIRD);

	if (_pendingTird.find(deviceId) != _pendingTird.end() || tird == nullptr)
	{
		return false;
	}
	if (tird->GetListId() == 0)
	{
		int32 id = WINGUI->NewPendingList(tird);
		tird->SetListId(id);
	}
	
	_pendingTird.insert({ deviceId, tird });

	return true;
}

bool TIMServer::PopPendingList(SessionRef session)
{
	if (session->GetDeviceType() == Device::DeviceTIFD)
		return PopPendingList(dynamic_pointer_cast<TifdSession>(session));
	else if(session->GetDeviceType() == Device::DeviceTIRD)
		return PopPendingList(dynamic_pointer_cast<TirdSession>(session));

	return false;
}

bool TIMServer::PopPendingList(TifdRef tifd)
{
	WRITE_LOCK_IDX(TIFD);

	if (tifd == nullptr)
		return false;

	string deviceId = tifd->GetDeviceIdToString();

	auto it = _pendingTifd.find(deviceId);
	if (it == _pendingTifd.end())
		return false;

	WINGUI->DeleteTifdPendingList(tifd->GetListId());
	_pendingTifd.erase(it);

	return true;
}

bool TIMServer::PopPendingList(TirdRef tird)
{
	WRITE_LOCK_IDX(TIRD);

	if (tird == nullptr)
		return false;

	string deviceId = tird->GetDeviceIdToString();

	auto it = _pendingTird.find(deviceId);
	if (it == _pendingTird.end())
		return false;

	WINGUI->DeleteTirdPendingList(tird->GetListId());
	_pendingTird.erase(it);

	return true;
}

bool TIMServer::PushPairingList(TifdRef tifd, TirdRef tird, int32 distance)
{
	// 두개 다 해줘야됨.
	WRITE_LOCK_IDX(PAIRING);

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

		PopPendingList(tifd);
		PopPendingList(tird);

		tifd->SetPairingId(pairingId);
		tird->SetPairingId(pairingId);

		auto it = _pairingSessions.insert({ pairingId, pair });
		if (it.second == false)
			CRASH("Pairing already inside");
	}

	wstring str = std::format(L"Pairing On : TIFD = {0}, TIRD = {1}"
		, StringToWstring(tifdData->deviceId)
		, StringToWstring(tirdData->deviceId));
	WINGUI->AddLogList(str);
	
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

bool TIMServer::PopPairingList(int32 pairingId, SessionRef session)
{
	if (session->GetDeviceType() == Device::DeviceTIFD)
		return PopPairingList(pairingId, dynamic_pointer_cast<TifdSession>(session));
	else if (session->GetDeviceType() == Device::DeviceTIRD)
		return PopPairingList(pairingId, dynamic_pointer_cast<TirdSession>(session));
	return false;
}

bool TIMServer::PopPairingList(int32 pairingId, TifdRef session)
{
	WRITE_LOCK_IDX(PAIRING);

	auto it = _pairingSessions.find(pairingId);
	if (it == _pairingSessions.end())
		return PopPendingList(session);
	else
	{
		PairSessionRef pairRef = it->second;
		pairRef->Disconnected();

		WINGUI->DeletePairingList(pairRef->GetPairingId(), session->GetDeviceType());

		{
			auto tird = pairRef->GetTirdSession();
			SendBufferRef sendBuf = MakeSendLoraBuffer(LORA_DEFAULT_CH);
			tird->Send(sendBuf);
			if (PushPendingList(tird, tird->GetDeviceIdToString()) == false)
				return false;
		}
		wstring str = std::format(L"Pairing Off : tifd Disconnected {0}", session->GetDeviceIdToWString());
		WINGUI->AddLogList(str);
		_pairingSessions.erase(pairingId);

		return true;
	}
}

bool TIMServer::PopPairingList(int32 pairingId, TirdRef session)
{
	WRITE_LOCK_IDX(PAIRING);

	auto it = _pairingSessions.find(pairingId);
	if (it == _pairingSessions.end())
		return PopPendingList(session);
	else 
	{
		PairSessionRef pairRef = it->second;
		if (pairRef == nullptr)
			return false;

		pairRef->Disconnected();

		WINGUI->DeletePairingList(pairRef->GetPairingId(), session->GetDeviceType());

		{
			// TODO 체크
			auto tifd = pairRef->GetTifdSession();
			PushPendingList(tifd, tifd->GetDeviceIdToString());

			SendBufferRef sendBuf = MakeSendUnPairingBuffer();
			tifd->Send(sendBuf);
			sendBuf = MakeSendLoraBuffer(LORA_DEFAULT_CH);
			tifd->Send(sendBuf);
		}

		wstring str = std::format(L"Pairing Off : tird Disconnected {0}", session->GetDeviceIdToWString());
		WINGUI->AddLogList(str);
		_pairingSessions.erase(it);

		return true;
	}
}

void TIMServer::SendKeepAlive()
{
	THREAD->Push([=]() {
		uint64 currentTick = GetTickCount64();
		while (GStart)
		{
			uint64 nowTick = GetTickCount64();
			if (nowTick - currentTick > 3000)
			{
				SendBufferRef ref = MakeSendBuffer(CommandType::CmdType_KeepAlive);
				{
					WRITE_LOCK_IDX(TIFD);
					for (auto& tifd : _pendingTifd)
					{
						tifd.second->Send(ref);
					}
				}
				{
					WRITE_LOCK_IDX(TIRD);
					for (auto& tird : _pendingTird)
					{
						tird.second->Send(ref);
					}
				}
				{
					WRITE_LOCK_IDX(PAIRING);
					for (auto& pair : _pairingSessions)
					{
						pair.second->GetTifdSession()->Send(ref);
						pair.second->GetTirdSession()->Send(ref);
					}
				}
				
				currentTick = nowTick;
			}
		}
	});
}

void TIMServer::GetPossiblePairingList(pair<float, float> tifdLocation, vector<PossiblePairingList>& lists)
{
	WRITE_LOCK_IDX(TIRD);

	for (auto tirds : _pendingTird)
	{
		bool possible = true;
		TirdRef tird = tirds.second;
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
}

void TIMServer::Update()
{
	while (true)
	{
		FD_ZERO(&_fds);

		
	}
}
