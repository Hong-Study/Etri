#pragma once
#include "JobQueue.h"

class TIMServer : public JobQueue
{
	SINGLETON(TIMServer)

public:
	void Init();
	void Clear();
	void Start();
	void Update();

	// JobQueue 사용
	void PushTifdList(SOCKET sock, SOCKADDR_IN sockAddr, const StTifdData* data);
	void PushTirdList(SOCKET sock, SOCKADDR_IN sockAddr, const StTirdData* data);

	void PopList(SessionRef ref);
	void PopTifdList(TifdRef tifd);
	void PopTirdList(TirdRef tird);	

	void PushPairingList(TifdRef tifd, TirdRef tird, int32 distance);
	bool PopPairingList(int32 pairingId, SessionRef session);

	void GetPossiblePairingList(pair<float, float> tifdLocation, vector<PossiblePairingList>& lists);

	void SendKeepAliveUpdate();
	void SendKeepAlive();

private:

	bool PopPairingList(int32 pairingId, TifdRef session);
	bool PopPairingList(int32 pairingId, TirdRef session);
	
private:
	USE_LOCK;

	map<std::string, TifdRef>				_tifdList;
	map<std::string, TirdRef>				_tirdList;
	
	// ListId가 Key로 작동
	std::map<int32, PairSessionRef>	_pairingSessions;

	fd_set _fds;
private:
	bool		_isWork = true;
	StLoraInfo	_loraInfo;
};