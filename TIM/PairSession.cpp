#include "pch.h"
#include "PairSession.h"
#include "TifdSession.h"
#include "TirdSession.h"

PairSession::PairSession(TifdRef tifd, TirdRef tird, int32 pairingId, int32 distance)
	: _tifdSession(tifd), _tirdSession(tird), _pairingId(pairingId)
{
	tifd->SetPairState(ePairState::PairState_Pair);
	tird->SetPairState(ePairState::PairState_Pair);
}

PairSession::~PairSession()
{
	_tirdSession = nullptr;
	_tifdSession = nullptr;
}

void PairSession::Disconnected()
{
	{
		_tirdSession->SetTarget(nullptr);
		_tirdSession->SetPairState(ePairState::PairState_Unpair);
	}

	{
		_tifdSession->SetTarget(nullptr);
		_tifdSession->SetPairState(ePairState::PairState_Unpair);
	}
}

void PairSession::ChangeTird(TirdRef newTird)
{
	_tirdSession = newTird;
	_tirdSession->SetPairingId(_pairingId);
	_tirdSession->SetPairState(ePairState::PairState_Pair);
	
	_tifdSession->SetTarget(_tirdSession);
	_tirdSession->SetTarget(_tifdSession);
}
