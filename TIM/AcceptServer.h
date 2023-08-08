#pragma once
#include "Session.h"

struct SessionInfo
{
	SOCKET sock;
	SOCKADDR_IN sockAddr;
};

class AcceptServer
{
	SINGLETON(AcceptServer);
public:
	void		Init();
	void		Update();
	void		Clear();
	void		Disconnect(SessionInfo& info);

private:
	void		Recv(SessionInfo& info);
	int32		OnRecv(SessionInfo& info, BYTE* buffer, int32 size);
	void		OnRecvPacket(SessionInfo& info, BYTE* buffer, int32 size);

	bool		Send(SOCKET sock, SendBufferRef buffer);
	bool		Send(SOCKET sock, BYTE* buffer, int32 size);

private:
	void		HandleTifdConnect(SessionInfo& info, const StTifdData* data);
	void		HandleTirdConnect(SessionInfo& info, const StTirdData* data);

private:
	SOCKET			_listenSocket = INVALID_SOCKET;
	SOCKADDR_IN		_sockAddr;
	fd_set			_rds;
	struct timeval	_cv;

private:
	vector<SessionInfo>		_infos;
	shared_ptr<RecvBuffer>	_recvBuffer;
};