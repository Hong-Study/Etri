#pragma once

class PairSession
{
public:
	PairSession(TifdRef tifd, TirdRef tird, int32 pairingId, int32 distance);
	~PairSession();

	void		SetTird(TirdRef session) { _tirdSession = session; }
	void		SetTifd(TifdRef session) { _tifdSession = session; }

	TirdRef		GetTirdSession() { return _tirdSession; }
	TifdRef		GetTifdSession() { return _tifdSession; }

	int32		GetPairingId() { return _pairingId; }
	bool		CheckSessions() { return ((_tirdSession != nullptr) && (_tifdSession != nullptr)); }

	// Clear
	void		Disconnected();
	void		ChangeTird(TirdRef newTird);

private:
	TirdRef _tirdSession;
	TifdRef _tifdSession;

	int32	_pairingId;
};