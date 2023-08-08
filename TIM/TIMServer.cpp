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
	std::filesystem::path path(GCSVPos + "temp.txt");
	if (std::filesystem::exists(path.parent_path()) == false)
	{
		std::filesystem::create_directories(path.parent_path());
	}
	// 시간 Device Lat Long 스피드 위성
	string pendingSet = ",,lat,long,,\n";

	// 시간 tifd tifd_lat tifd_long tird tird_lat tird_long distanec
	string pairingSet = ",TIFD,TIFD_Lat,TIFD_Long,TIRD,TIRD_Lat,TIRD_Long,Distance\n";

	// 여기서 설정하면됨.
	tifdCSV.open(GCSVPos + "tifdLog.csv");
	tirdCSV.open(GCSVPos + "tirdLog.csv");
	pairingCSV.open(GCSVPos + "pairing.csv");

	tifdCSV << pendingSet;
	tirdCSV << pendingSet;
	pairingCSV << pairingSet;
}

void TIMServer::Clear()
{
	WRITE_LOCK_IDX(TIFD);
	WRITE_LOCK_IDX(TIRD);
	WRITE_LOCK_IDX(PAIRING);
	
	for (auto ref : _pendingTifd)
	{
		ref.second->Disconnect();
	}

	for (auto ref : _pendingTird)
	{
		ref.second->Disconnect();
	}
	
	for (auto ref : _pairingSessions)
	{
		ref.second->GetTifdSession()->Disconnect();
		ref.second->GetTirdSession()->Disconnect();
	}

	_pendingTird.clear();
	_pendingTifd.clear();
	_pairingSessions.clear();

	tifdCSV.close();
	tirdCSV.close();
	pairingCSV.close();
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

void TIMServer::PopPendingList(SessionRef session)
{
	if (session->GetDeviceType() == Device::DeviceTIFD)
		PopPendingList(dynamic_pointer_cast<TifdSession>(session));
	else if(session->GetDeviceType() == Device::DeviceTIRD)
		PopPendingList(dynamic_pointer_cast<TirdSession>(session));
}

void TIMServer::PopPendingList(TifdRef tifd)
{
	WRITE_LOCK_IDX(TIFD);

	if (tifd == nullptr)
		return;

	string deviceId = tifd->GetDeviceIdToString();

	auto it = _pendingTifd.find(deviceId);
	if (it == _pendingTifd.end())
		return;

	WINGUI->DeleteTifdPendingList(tifd->GetListId());
	_pendingTifd.erase(it);
}

void TIMServer::PopPendingList(TirdRef tird)
{
	WRITE_LOCK_IDX(TIRD);

	if (tird == nullptr)
		return;

	string deviceId = tird->GetDeviceIdToString();

	auto it = _pendingTird.find(deviceId);
	if (it == _pendingTird.end())
		return;

	WINGUI->DeleteTirdPendingList(tird->GetListId());
	_pendingTird.erase(it);
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

void TIMServer::PopPairingList(int32 pairingId, SessionRef session)
{
	if (session->GetDeviceType() == Device::DeviceTIFD)
		PopPairingList(pairingId, dynamic_pointer_cast<TifdSession>(session));
	else if (session->GetDeviceType() == Device::DeviceTIRD)
		PopPairingList(pairingId, dynamic_pointer_cast<TirdSession>(session));
}

void TIMServer::PopPairingList(int32 pairingId, TifdRef session)
{
	WRITE_LOCK_IDX(PAIRING);

	auto it = _pairingSessions.find(pairingId);
	if (it == _pairingSessions.end())
		PopPendingList(session);
	{
		PairSessionRef pairRef = it->second;
		pairRef->Disconnected();

		WINGUI->DeletePairingList(pairRef->GetPairingId(), session->GetDeviceType());

		{
			auto tird = pairRef->GetTirdSession();
			SendBufferRef sendBuf = MakeSendLoraBuffer(LORA_DEFAULT_CH);
			tird->Send(sendBuf);
			PushPendingList(tird, tird->GetDeviceIdToString());
		}
		wstring str = std::format(L"Pairing Off : tifd Disconnected {0}", session->GetDeviceIdToWString());
		WINGUI->AddLogList(str);
		_pairingSessions.erase(pairingId);
	}
}

void TIMServer::PopPairingList(int32 pairingId, TirdRef session)
{
	WRITE_LOCK_IDX(PAIRING);

	auto it = _pairingSessions.find(pairingId);
	if (it == _pairingSessions.end())
		PopPendingList(session);
	else 
	{
		PairSessionRef pairRef = it->second;
		if(pairRef != nullptr)
			pairRef->Disconnected();

		WINGUI->DeletePairingList(pairRef->GetPairingId(), session->GetDeviceType());

		{
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

void TIMServer::SaveTifdCSV(StTifdData* tifd)
{
	if (tifd == nullptr)
		return;

	StTifdData data;
	memcpy(&data, tifd, sizeof(StTifdData));

	WRITE_LOCK_IDX(TIFD);
	// 시간 Device Lat Long 스피드 위성
	string str = std::format("{0}, {1}, {2:0.5f}, {3:0.5f}, {4}, {5}\n",
		0, data.deviceId, data.lat, data.lon, data.speed, data.sat);

	tifdCSV << str;
}

void TIMServer::SaveTirdCSV(StTirdData* tird)
{
	if (tird == nullptr)
		return;

	StTirdData data;
	memcpy(&data, tird, sizeof(StTirdData));

	WRITE_LOCK_IDX(TIRD);
	// 시간 Device Lat Long 스피드 위성
	string str = std::format("{0}, {1}, {2:0.5f}, {3:0.5f}, {4}, {5}\n",
		0, data.deviceId, data.lat, data.lon, data.speed, data.sat);

	tirdCSV << str;
}

void TIMServer::SavePairingCSV(StTifdData* tifdData, StTirdData* tirdData)
{
	if (tifdData == nullptr || tirdData == nullptr)
		return;
	WRITE_LOCK_IDX(PAIRING);
	
	StTifdData tifd;
	memcpy(&tifd, tifdData, sizeof(StTifdData));

	StTirdData tird;
	memcpy(&tird, tirdData, sizeof(StTirdData));

	// 시간 tifd tifd_lat tifd_long tird tird_lat tird_long distanec
	string str = std::format("{0}, {1}, {2:0.5f}, {3:0.5f}, {4}, {5:0.5f}, {6:0.5f}, {7}\n",
		0, tifd.deviceId, tifd.lat, tifd.lon, tird.deviceId, tird.lat, tird.lon, tifd.distance);

	pairingCSV << str;
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