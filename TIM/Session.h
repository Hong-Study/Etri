#pragma once

class Session : public std::enable_shared_from_this<Session>
{
public:
	Session(SOCKET sock, SOCKADDR_IN addr, Device device);
	virtual ~Session();

	virtual void		Init();

	// Recv
	void				Recv();
	int32				OnRecv(BYTE* buffer, int32 size);
	virtual void		OnRecvPacket(BYTE* buffer, int32 size) { }

	// Disconnect
	void				Disconnect();
	virtual void		OnDisconnected() { }

	// Send
	void				Send(SendBufferRef buffer);
	void				Send(BYTE* buffer, int32 size);

	// Network Getter
	void				SetSocket(SOCKET socket) { _socket = socket; }
	SOCKET				GetSocket() { return _socket; }
	const SOCKADDR_IN&	GetSockAddr() { return _sockAddr; }

	// Information Setter	
	void				SetDeviceType(const Device device) { _deviceType = device; }
	void				SetPairState(const ePairState state) { _pairState = state; }
	void				SetListId(const int32 id) { _listId = id; }
	void				SetPairingId(const int32 id) { _pairingId = id; }

	// MyInfo Getter
	Device				GetDeviceType() { return _deviceType; }
	ePairState			GetPairState() { return _pairState; }
	int32				GetPairingId() { return _pairingId; }
	int32				GetListId() { return _listId; }

	uint64				GetCurrnetRecvTime() { return _currentRecvTime; }
	void				SetCurrentRecvTime(uint64 time) { _currentRecvTime = time; }
	// CMD â���� ���� ���
	// void				PrintSessionInfo();

private:
	SOCKET		_socket;
	SOCKADDR_IN	_sockAddr;
	uint32		_tickCount = 0;

	RecvBuffer	_recvBuffer;
	uint64		_startTime = 0;
	uint64		_currentRecvTime = 0;

protected:
	USE_LOCK;
	Device		_deviceType;
	ePairState	_pairState;
	int32		_pairingId = 0;
	int32		_listId = 0;
};