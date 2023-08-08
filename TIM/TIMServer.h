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

	// Push ����
	bool PushPairingList(TifdRef tifd, TirdRef tird, int32 distance);
	
	// Pop ����
	void PopPendingList(SessionRef session);
	void PopPairingList(int32 pairingId, SessionRef session);

	void GetPossiblePairingList(pair<float, float> tifdLocation, vector<PossiblePairingList>& lists);
	void SendKeepAlive();

public:
	void SaveTifdCSV(StTifdData* tifd);
	void SaveTirdCSV(StTirdData* tird);
	void SavePairingCSV(StTifdData* tifdData, StTirdData* tirdData);

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

	// ListId�� Key�� �۵�
	// string�� Ű ������ �۵��ϴ� ������.
	std::map<std::string, TifdRef> _pendingTifd;
	std::map<std::string, TirdRef> _pendingTird;

	// ListId�� Key�� �۵�
	std::map<int32, PairSessionRef> _pairingSessions;

private:
	std::ofstream tifdCSV;
	std::ofstream tirdCSV;
	std::ofstream pairingCSV;

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