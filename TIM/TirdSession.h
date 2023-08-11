#pragma once
#include "Session.h"

class TirdSession : public Session
{
	using Super = Session;
public:
	TirdSession(SOCKET sock, SOCKADDR_IN addr, const StTirdData* data);
	virtual ~TirdSession();
	virtual void		Init() override;

	virtual void		OnRecvPacket(BYTE* buffer, int32 size) override;
	virtual void		OnDisconnected() override;

	void				SetTirdData(const StTirdData* data);

	SessionRef			GetSessoin() { return shared_from_this(); }
	TirdRef				GetTirdSession() { return static_pointer_cast<TirdSession>(shared_from_this()); }

public:
	wstring				GetDeviceIdToWString() { return StringToWstring(_myData->deviceId); }
	string				GetDeviceIdToString() { return _myData->deviceId; }
	StTirdData*			GetData() { READ_LOCK; return _myData; }
	pair<float, float>	GetLocation();

	void				SetTarget(const TifdRef ref) { _pairingTarget = ref; }
	TifdRef				GetTarget() { return _pairingTarget; }
private:
	// CmdType_Tird_Info:
	void				HandleUpdatePendingInfo(const StTirdData* data);
	void				HandleUpdatePairingInfo(const StTirdData* data);
	void				SendInputDataToTarget(BYTE* buffer, int32 size);

private:
	TifdRef				_pairingTarget;
	StTirdData*			_myData = nullptr;
	class FileWriter*	_writer = nullptr;
};

