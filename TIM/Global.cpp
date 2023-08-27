#include "pch.h"
#include "Global.h"
#include "JsonParser.h"
#include "TIMServer.h"
#include "WinApi.h"

JsonParser*		GParser = nullptr;

int32			GServerPort = 10000;
int32			GMaximumDistance = 1000;
int32			GLowestSpeed = 20;
int32			GDistanceAccuarcy = 100;
int32			GSuccessCount = 5;
int32			GOverCount = 10;
bool			GStart = false;
std::wstring	GServerIP = L"";
std::string		GLogPos = "";
std::wstring	GCodingRate = L"";
std::wstring	GSpreadingFactor = L"";

std::wstring*	tifdInfoStr = nullptr;
std::wstring*	tirdInfoStr = nullptr;

shared_ptr<class TIMServer>	TIM = nullptr;
shared_ptr<class WinApi>	WINGUI = nullptr;

class GlobalVariables
{
public:
	GlobalVariables()
	{
		GParser = new JsonParser("config.json");
		GParser->Init();

		if (GParser->StartParsing() == false)
			return;

		// 체크 필요
		if (GParser->GetValue("ServerPort", GServerPort) == false)
			return;
		if (GParser->GetValue("MaximumDistance", GMaximumDistance) == false)
			return;
		if (GParser->GetValue("LowestSpeed", GLowestSpeed) == false)
			return;
		if (GParser->GetValue("DistanceAccuarcy", GDistanceAccuarcy) == false)
			return;
		if (GParser->GetValue("SuccessCount", GSuccessCount) == false)
			return;
		if (GParser->GetValue("OverCount", GOverCount) == false)
			return;
		if (GParser->GetValue("ServerIP", GServerIP) == false)
			return;
		if (GParser->GetValue("LogPos", GLogPos) == false)
			return;

		int32 inputData = 0;
		if (GParser->GetValue("SpreadingFactor", inputData) == false)
			return;
		GSpreadingFactor = to_wstring(inputData);
		
		if (GParser->GetValue("CodingRate", inputData) == false)
			return;
		GCodingRate = to_wstring(inputData);

		tifdInfoStr = new std::wstring[]{
			L"Version", L"Device", L"PairingStatus", L"Network Information", L"IpAddress", L"Port", L"GPS Information", L"Time"
			, L"Latitude", L"Longitude", L"Altitude", L"Speed", L"Satellite", L"Distance", L"LORA Information", L"LoRaVersion"
			, L"Channel", L"Power", L"Bandwidth", L"SpreadingFactor", L"CodingRate", L"LoRa Receive Information"
			, L"TIRD Device" , L"RSSI" };
		tirdInfoStr = new std::wstring[]{
			L"Version", L"Device", L"PairingStatus", L"Network Information", L"IpAddress", L"Port", L"GPS Information"
			, L"Time", L"Latitude", L"Longitude", L"Altitude", L"Speed", L"Satellite", L"LORA Information"
			, L"Channel", L"Power", L"Bandwidth", L"SpreadingFactor", L"CodingRate", L"Battery Information"
			, L"Battery Voltage" };

		TIM = make_shared<TIMServer>();
		WINGUI = make_shared<WinApi>();
	}

	~GlobalVariables()
	{
		delete GParser;
	}

}GGlobalVariables;