#include "pch.h"
#include "TifdSession.h"
#include "WinApi.h"
#include "TIMServer.h"
#include "TirdSession.h"
#include "FileUtils.h"

TifdSession::TifdSession(SOCKET sock, SOCKADDR_IN addr, const StTifdData* data)
	: Super(sock, addr, Device::DeviceTIFD)
{ 
	Init();
	SetTifdData(data);
}

void TifdSession::Init()
{
	Super::Init();
	
	_myData = new StTifdData;
	_writer = new FileWriter();
}

void TifdSession::OnRecvPacket(BYTE* buffer, int32 size)
{
	PktHead* head = reinterpret_cast<PktHead*>(buffer);
	switch (head->command)
	{
	case CommandType::CmdType_Tifd_Info:
		if (_deviceType = Device::DeviceTIFD)
		{
			if (_pairState == ePairState::PairState_Unpair)
				HandleUpdatePendingInfo(reinterpret_cast<StTifdData*>(&head[1]));
			else if (_pairState == ePairState::PairState_Pair)
				HandleUpdatePairingInfo(reinterpret_cast<StTifdData*>(&head[1]));
		}
		else
			Disconnect();
		break;
	default:
		Disconnect();
		break;
	}
}

void TifdSession::OnDisconnected()
{
	if (_pairingTarget != nullptr)
		_pairingTarget = nullptr;

	wstring str = std::format(L"Disconnected TIFD {0}", GetDeviceIdToWString());
	WINGUI->DoAsync(&WinApi::AddLogList, str);

	if (_myData != nullptr)
		delete _myData;
	if (_writer != nullptr)
		delete _writer;

	SetDeviceType(Device::DeviceNone);
}

void TifdSession::UpdateToPairingSuccess()
{
	_possibleLists.clear();
	std::tm localTime = GetLocalTime();
	_writer->FileStreamOpenWithCSV(localTime.tm_mon, localTime.tm_mday, localTime.tm_hour, _myData->deviceId, _deviceType, ePairState::PairState_Pair);
}

void TifdSession::SetTifdData(const StTifdData* data)
{
	WRITE_LOCK;

	memcpy(_myData, data, sizeof(StTifdData));
	_myData->stTime.hour = (_myData->stTime.hour + 9) % 24;

	if (_nowTime != _myData->stTime.hour)
	{
		_nowTime = _myData->stTime.hour;
		std::tm localTime = GetLocalTime();
		_writer->FileStreamOpenWithCSV(localTime.tm_mon, localTime.tm_mday, localTime.tm_hour, _myData->deviceId, _deviceType, _pairState);
	}
		
	if (_pairState == ePairState::PairState_Pair)
	{
		if (_pairingTarget != nullptr)
		{
			StTirdData tird;
			memcpy(&tird, _pairingTarget->GetData(), sizeof(StTirdData));
			if (_myData->distance - _myData->trainLength >= GTrainSeparationCheckDistance)
				_distanceCheckCount.fetch_add(1);
			else
				_distanceCheckCount.store(0);

			if (_writer->WritePairingString(_myData, tird) == false)
				CRASH("WritePairingStringWithTifd");
		}
	}
	else if (_pairState == ePairState::PairState_Unpair)
	{
		if (_writer->WritePendingString(_myData) == false)
			CRASH("WritePendingStringWithTifd");
	}
}

pair<float, float> TifdSession::GetLocation()
{
	WRITE_LOCK;
	if (_myData == nullptr)
		return { -1.f, -1.f };
	return { _myData->lat, _myData->lon };
}

void TifdSession::HandleUpdatePendingInfo(const StTifdData* data)
{
	SetTifdData(data);
	
	WINGUI->UpdateTifdPendingInfo(GetListId(), GetTifdSession());

	if (data->speed >= GLowestSpeed)
	{
		if (CheckingPairingPossibleList())
		{
			UpdateToPairingSuccess();
			return;
		}

		FindNearPossibleTird();
	}
}

void TifdSession::HandleUpdatePairingInfo(const StTifdData* data)
{
	SetTifdData(data);

	if (_distanceCheckCount == GTrainSeparationCheckCount)
		_myData->trainStatus = TrainStatus_OpenAlarmRequest;

	// 알람이 울렸으니 새로운 페어링을 찾아보는건가?
	if (_myData->trainStatus == TrainStatus_OpenAlarmRequest)
	{
		bool newTird = true;
#ifdef TEST
		// 새로운 리스트 목록 가져오기
		FindNearPossibleTird();

		// 만약 목록이 비어있다면
		if (_possibleLists.empty())
		{
			newTird = false;
		}
		else
		{
			if (CheckingPairingPossibleList() == false)
			{
				newTird = false;
			}
		}

		if (newTird == false)
		{
			SendBufferRef buf = MakeSendBuffer(CommandType::CmdType_Alarm);
			Send(buf);
		}

#endif // TEST
		wstring str = std::format(L"Train({0}) is OpenAlramRequest", _myData->trainNo);
		wstring alram = L"Open Alram Request";
		WINGUI->DoAsync(&WinApi::AddLogList, str);
		WINGUI->DoAsync(&WinApi::ShowTrainAlramStatus, GetPairingId(), alram);
	}

	if (_pairingTarget != nullptr)
	{
		WINGUI->UpdateTifdPairingInfo(GetPairingId(), data->distance, GetData(), _pairingTarget->GetData());
	}
}
void TifdSession::FindNearPossibleTird()
{
	TIM->GetPossiblePairingList(GetLocation(), _possibleLists);
}

bool TifdSession::CheckingPairingPossibleList()
{
	// 이름 -> PossibleTirdList
	for (auto it = _possibleLists.begin();it != _possibleLists.end();)
	{
		if (it->target == nullptr || it->target->GetPairState() == ePairState::PairState_Pair)
		{
			it = _possibleLists.erase(it);
			continue;
		}

		auto tirdLocatoin = it->target->GetLocation();
		auto tifdLocation = GetLocation();
		int32 distance = CalculateDistance(tifdLocation, tirdLocatoin);

		if (it->isStart == false)
		{
			it->fistDistance = distance;
			it->isStart = true;
			it++;
		}
		else if (distance - it->fistDistance < GDistanceAccuarcy)
		{
			// 길이 체크 및 오차범위 체크해야함
			it->currentDistacne = distance;
			it->timeCount++;

			if (it->timeCount >= GSuccessCount)
			{
				if (GetPairState() == ePairState::PairState_Pair)
				{
					if(TIM->ChangePairingList(GetTifdSession(), it->target))
						return true;
					else
						it = _possibleLists.erase(it);
				}
				// 실패했다는 소리는 이미 페어링이 되어있다는 소리거나 TIRD가 아니라는 소리
				else if (TIM->PushPairingList(GetTifdSession(), it->target, distance))
				{
					return true;
				}
				else
				{
					it = _possibleLists.erase(it);
				}
			}
			else
				it++;
		}
		else
			it = _possibleLists.erase(it);
	}

	return false;
}