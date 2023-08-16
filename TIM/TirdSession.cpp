#include "pch.h"
#include "TirdSession.h"
#include "WinApi.h"
#include "TIMServer.h"
#include "TifdSession.h"
#include "FileUtils.h"

TirdSession::TirdSession(SOCKET sock, SOCKADDR_IN addr, const StTirdData* data)
	: Super(sock, addr, Device::DeviceTIRD)
{
	Init();
	SetTirdData(data);
	_nowTime = data->stTime.hour;
}

void TirdSession::Init()
{
	Super::Init();

	_myData = new StTirdData;
	_writer = new FileWriter();
}

void TirdSession::OnRecvPacket(BYTE* buffer, int32 size)
{
	PktHead* head = reinterpret_cast<PktHead*>(buffer);
	switch (head->command)
	{
	case CommandType::CmdType_Tird_Info:
		if (_deviceType = Device::DeviceTIRD)
		{
			if (_pairState == ePairState::PairState_Unpair)
				HandleUpdatePendingInfo(reinterpret_cast<StTirdData*>(&head[1]));
			else if (_pairState == ePairState::PairState_Pair)
				HandleUpdatePairingInfo(reinterpret_cast<StTirdData*>(&head[1]));
		}
		else
			Disconnect();
		break;
	default:
		Disconnect();
		break;
	}
}

void TirdSession::OnDisconnected()
{
	if (_pairingTarget != nullptr)
		_pairingTarget = nullptr;

	wstring str = std::format(L"Disconnected TIRD {0}", GetDeviceIdToWString());
	WINGUI->AddLogList(str);

	if(_myData != nullptr)
		delete _myData;
	if (_writer != nullptr)
		delete _writer;
	SetDeviceType(Device::DeviceNone);
}

void TirdSession::SetTirdData(const StTirdData* data)
{
	WRITE_LOCK;

	memcpy(_myData, data, sizeof(StTirdData));
	_myData->stTime.hour = (_myData->stTime.hour + 9) % 24;

	if (_pairState == ePairState::PairState_Unpair)
	{
		if (_nowTime != _myData->stTime.hour)
		{
			_nowTime = _myData->stTime.hour;
			std::tm localTime = GetLocalTime();
			_writer->FileStreamOpenWithCSV(localTime.tm_mon, localTime.tm_mday, localTime.tm_hour, _myData->deviceId, _deviceType, ePairState::PairState_Unpair);
		}

		if (_writer->WritePendingString(_myData) == false)
			return;
	}
	else if (_pairState == ePairState::PairState_Pair && _writer->IsOpen())
		_writer->FileStreamClose();
}

pair<float, float> TirdSession::GetLocation()
{
	WRITE_LOCK;

	return { _myData->lat, _myData->lon };
}

void TirdSession::HandleUpdatePendingInfo(const StTirdData* data)
{
	if (data == nullptr)
		return;
	SetTirdData(data);

	WINGUI->UpdateTirdPendingInfo(GetListId(), GetData());
}

void TirdSession::HandleUpdatePairingInfo(const StTirdData* data)
{
	if (data == nullptr)
		return;
	SetTirdData(data);
	if (_pairingTarget != nullptr)
		WINGUI->UpdateTirdPairingInfo(GetPairingId(), GetData());
}


// TIRD -> TIFD
void TirdSession::SendInputDataToTarget(BYTE* buffer, int32 size)
{
	if (_pairingTarget != nullptr && _deviceType == Device::DeviceTIRD)
		_pairingTarget->Send(buffer, size);
}