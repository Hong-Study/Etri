#pragma once
#include "ThreadManager.h"
#include "JobQueue.h"
class TIMServer : public JobQueue
{
	SINGLETON(TIMServer)
public:
	void Init();
	void Clear();
	void Start();
	void Update();

	bool PushPendingList(TifdRef tifd);
	bool PushPendingList(TirdRef tird);

	// Push 관련
	bool PushPairingList(TifdRef tifd, TirdRef tird, int32 distance);
	
	// Pop 관련
	bool PopPendingList(SessionRef session);
	bool PopPairingList(int32 pairingId, SessionRef session);

	void GetPossiblePairingList(pair<float, float> tifdLocation, vector<PossiblePairingList>& lists);

	void SendKeepAliveUpdate();
	void SendKeepAlive();

private:
	bool PopPendingList(TifdRef tifd);
	bool PopPendingList(TirdRef tird);

	bool PopPairingList(int32 pairingId, TifdRef session);
	bool PopPairingList(int32 pairingId, TirdRef session);
	
private:
	USE_LOCK;

	vector<SessionRef>				_totalSessions;

	// string이 키 값으로 작동하니 느리다.
	std::map<std::string, TifdRef>	_pendingTifd;
	std::map<std::string, TirdRef>	_pendingTird;

	// ListId가 Key로 작동
	std::map<int32, PairSessionRef>	_pairingSessions;

	FD_SET _fds;
private:
	bool		_isWork = true;
	StLoraInfo	_loraInfo;
};