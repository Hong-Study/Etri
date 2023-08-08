#include "pch.h"
#include "TirdSession.h"
#include "WinApi.h"
#include "TIMServer.h"
#include "TifdSession.h"

TirdSession::TirdSession(SOCKET sock, SOCKADDR_IN addr, const StTirdData* data)
	: Super(sock, addr, Device::DeviceTIRD)
{
	Init();

	_myData = new StTirdData;
	SetTirdData(data);
}

void TirdSession::Init()
{
	Super::Init();
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

	delete _myData;
	SetDeviceType(Device::DeviceNone);
}

void TirdSession::SetTirdData(const StTirdData* data)
{
	WRITE_LOCK;

	memcpy(_myData, data, sizeof(StTirdData));
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
	TIM->SaveTirdCSV(GetData());
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