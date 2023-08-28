#pragma once

extern class JsonParser* GGlobalVariable;

extern int32			GServerPort;
extern int32			GMaximumDistance;
extern int32			GLowestSpeed;
extern int32			GDistanceAccuarcy;
extern int32			GSuccessCount;
extern int32			GTrainSeparationCheckDistance;
extern int32			GTrainSeparationCheckCount;

extern bool				GStart;
extern std::wstring		GServerIP;
extern std::wstring		GSpreadingFactor;
extern std::wstring		GCodingRate;
extern std::string		GLogPos;

extern std::wstring*	tifdInfoStr;
extern std::wstring*	tirdInfoStr;

extern shared_ptr<class TIMServer> TIM;
extern shared_ptr<class WinApi>	WINGUI;