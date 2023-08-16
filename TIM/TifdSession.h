#pragma once
#include "Session.h"

class FileWriter;

class TifdSession : public Session
{
	friend class WinApi;

	using Super = Session;
public:
	TifdSession(SOCKET sock, SOCKADDR_IN addr, const StTifdData* data);
	virtual ~TifdSession() { }

	virtual void		Init() override;

	virtual void		OnRecvPacket(BYTE* buffer, int32 size) override;
	virtual void		OnDisconnected() override;
	void				UpdateToPairingSuccess();

	void				SetTifdData(const StTifdData* data);

	SessionRef			GetSessoin()     { return shared_from_this(); }
	TifdRef				GetTifdSession() { return static_pointer_cast<TifdSession>(shared_from_this()); }

public:
	wstring				GetDeviceIdToWString() { return StringToWstring(_myData->deviceId); }
	string				GetDeviceIdToString() { return _myData->deviceId; }
	StTifdData*			GetData() { READ_LOCK; return _myData; }
	pair<float, float>	GetLocation();
	void				SetTarget(const TirdRef ref) { _pairingTarget = ref; }
	TirdRef				GetTarget() { return _pairingTarget; }

private:
	// CmdType_Tifd_Info
	void				HandleUpdatePendingInfo(const StTifdData* data);
	void				HandleUpdatePairingInfo(const StTifdData* data);

	// Tifd_Pairing_Check
	void				FindNearPossibleTird();
	bool				CheckingPairingPossibleList();

private:
	std::vector<PossiblePairingList>	_possibleLists;
	std::atomic<int32>					_possibleListsSize = 0;
	TirdRef								_pairingTarget;
	StTifdData*							_myData = nullptr;
	class FileWriter*					_writer;
	int32								_nowTime = -1;
	std::string							_fileName;
};

