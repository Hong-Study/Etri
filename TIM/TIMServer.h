#pragma once
#include "JobQueue.h"

struct SessionInfo
{
	SOCKET sock;
	SOCKADDR_IN sockAddr;
	uint64	currentRecvTime = 0;
};

class TIMServer : public JobQueue
{
public:
	TIMServer() { } 
	~TIMServer() { }

	void		Init();
	void		Clear();
	bool		Start();
	void		Update();

	// JobQueue 사용 -> 멀티스레드 오류 방지
	void		PushTifdList(SOCKET sock, SOCKADDR_IN sockAddr, const StTifdData* data);
	void		PushTirdList(SOCKET sock, SOCKADDR_IN sockAddr, const StTirdData* data);

	void		PopList(SessionRef ref);
	
	bool		PushPairingList(TifdRef tifd, TirdRef tird, int32 distance);
	bool		ChangePairingList(TifdRef tifd, TirdRef tird);
	void		PopPairingList(int32 pairingId, SessionRef session);

	void		GetPossiblePairingList(pair<float, float> tifdLocation, vector<PossiblePairingList>& lists);

	void		SendKeepAliveUpdate();
	void		SendKeepAlive();
	StLoraInfo	GetLoraInfo() { return _loraInfo; }
private:
	// 실제 작동하는 코드
	void		PopTifdList(TifdRef tifd);
	void		PopTirdList(TirdRef tird);

	void		PopPairingList(int32 pairingId, TifdRef session);
	void		PopPairingList(int32 pairingId, TirdRef session);
	
private:
	// 초기 접속용 네트워크 코드
	void		Disconnect(SessionInfo* info);
	void		Recv(SessionInfo* info);
	int32		OnRecv(SessionInfo* info, BYTE* buffer, int32 size);
	void		OnRecvPacket(SessionInfo* info, BYTE* buffer, int32 size);

	bool		Send(SOCKET sock, SendBufferRef buffer);
	bool		Send(SOCKET sock, BYTE* buffer, int32 size);

private:
	SOCKET			_listenSocket = INVALID_SOCKET;
	SOCKADDR_IN		_sockAddr;
	fd_set			_rds;

private:
	vector<SessionInfo>		_infos;
	shared_ptr<RecvBuffer>	_recvBuffer;

private:
	USE_LOCK;

	atomic<int32>							_totalSize = 0;
	map<std::string, TifdRef>				_tifdList;
	map<std::string, TirdRef>				_tirdList;
	
	// ListId가 Key로 작동
	std::map<int32, PairSessionRef>			_pairingSessions;

	fd_set _fds;

private:
	bool		_isWork = true;
	StLoraInfo	_loraInfo;
};