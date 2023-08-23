#pragma once
#include "JobQueue.h"

class TIMServer : public JobQueue
{
public:
	TIMServer() { } 
	~TIMServer() { }

	void Init();
	void Clear();
	void Start();
	void Update();

	// JobQueue ���
	void PushTifdList(SOCKET sock, SOCKADDR_IN sockAddr, const StTifdData* data);
	void PushTirdList(SOCKET sock, SOCKADDR_IN sockAddr, const StTirdData* data);

	void PopList(SessionRef ref);
	
	bool PushPairingList(TifdRef tifd, TirdRef tird, int32 distance);
	void PopPairingList(int32 pairingId, SessionRef session);

	void GetPossiblePairingList(pair<float, float> tifdLocation, vector<PossiblePairingList>& lists);

	void SendKeepAliveUpdate();
	void SendKeepAlive();

private:
	void PopTifdList(TifdRef tifd);
	void PopTirdList(TirdRef tird);

	void PopPairingList(int32 pairingId, TifdRef session);
	void PopPairingList(int32 pairingId, TirdRef session);
	
private:
	USE_LOCK;

	atomic<int32>							_totalSize = 0;
	map<std::string, TifdRef>				_tifdList;
	map<std::string, TirdRef>				_tirdList;
	
	// ListId�� Key�� �۵�
	std::map<int32, PairSessionRef>	_pairingSessions;

	fd_set _fds;
private:
	bool		_isWork = true;
	StLoraInfo	_loraInfo;
};