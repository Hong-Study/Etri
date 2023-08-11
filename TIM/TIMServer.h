#pragma once
#include "ThreadManager.h"

class TIMServer
{
	SINGLETON(TIMServer)
public:
	void Init();
	void Clear();

	template<typename T>
	bool PushNewSession(T session);

	// Push 관련
	bool PushPairingList(TifdRef tifd, TirdRef tird, int32 distance);
	
	// Pop 관련
	void PopPendingList(SessionRef session);
	void PopPairingList(int32 pairingId, SessionRef session);

	void GetPossiblePairingList(pair<float, float> tifdLocation, vector<PossiblePairingList>& lists);
	void SendKeepAlive();

private:
	bool PushPendingList(TifdRef tifd, std::string deviceId);
	bool PushPendingList(TirdRef tird, std::string deviceId);

	void PopPendingList(TifdRef tifd);
	void PopPendingList(TirdRef tird);

	void PopPairingList(int32 pairingId, TifdRef session);
	void PopPairingList(int32 pairingId, TirdRef session);
	
private:
	enum { TIFD, TIRD, PAIRING, LOCK_SIZE};
	USE_MANY_LOCK(LOCK_SIZE);

	// ListId가 Key로 작동
	// string이 키 값으로 작동하니 느리다.
	std::map<std::string, TifdRef> _pendingTifd;
	std::map<std::string, TirdRef> _pendingTird;

	// ListId가 Key로 작동
	std::map<int32, PairSessionRef> _pairingSessions;

private:
	StLoraInfo _loraInfo;
};

template<typename T>
inline bool TIMServer::PushNewSession(T session)
{
	if (PushPendingList(session, session->GetDeviceIdToString()) == true)
	{
		SendBufferRef ref = MakeSendBuffer(CommandType::CmdType_Reg_Confirm);
		session->Send(ref);
		THREAD->Push([=]() {
			session->UpdateRecv();
			});

		return true;
	}
	return false;
}