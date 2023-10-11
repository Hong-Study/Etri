#pragma once
#include <string>
#include <array>
#include "Enums.h"
#include "Packet.h"

using std::string;
using std::wstring;
using std::to_wstring;
using std::pair;

// GUI 상의 필요한 정보들
struct PendingTifdListViewItem
{
	PendingTifdListViewItem(int32 id, int32 pos, StTifdData& data) : idNum(id), pos(pos), id(to_wstring(id))
	{
		SetInfo(data);
	}
	void SetDevice(string deviceId) { device.assign(deviceId.begin(), deviceId.end()); }
	void SetTirdId(string tidr) { tirdId.assign(tidr.begin(), tidr.end()); }
	void SetInfo(StTifdData& data);
	void SetTrainStatus(int32 type);

	int32 idNum;		// listId
	int32 pos;

	wstring id;
	wstring version;
	wstring device;

	wstring train;
	wstring trainLength;
	wstring trainStatus;
	
	wstring latitude;
	wstring longitude;
	wstring altitude;
	wstring speed;
	wstring sat;
	wstring distance;

	wstring loraVersion;
	wstring loraCh;
	wstring loraPower;
	wstring loraSF;
	wstring loraBW;
	wstring loraCR;

	wstring tirdId;
	wstring rssi;

	wstring time;
	wstring ip;
	wstring port;

	std::array<wstring, (int)TifdInfoCategory::SIZE> datas;
};

struct PendingTirdListViewItem
{
	PendingTirdListViewItem(int32 id, int32 pos, StTirdData& data) : idNum(id), pos(pos), id(to_wstring(id))
	{
		SetInfo(data);
	}
	void SetDevice(string deviceId) { device.assign(deviceId.begin(), deviceId.end()); }
	void SetInfo(StTirdData& data);

	int32 idNum;		// listId
	int32 pos;

	wstring id;
	wstring version;
	wstring device;

	wstring latitude;
	wstring longitude;
	wstring altitude;
	wstring speed;
	wstring sat;

	wstring loraCh;
	wstring loraPower;
	wstring loraSF;
	wstring loraBW;
	wstring loraCR;

	wstring battery;

	wstring time;
	wstring ip;
	wstring port;
};

struct PairingListViewItem
{
	PairingListViewItem(int32 nid, int32 pos, wstring wdistance, TifdListPtr ptifd, TirdListPtr ptird)
		: idNum(nid), pos(pos), id(to_wstring(nid)), distance(wdistance), tifd(ptifd), tird(ptird)
	{ 

	}

	int32 idNum;
	int32 pos;

	wstring id;
	wstring distance;

	TifdListPtr tifd;
	TirdListPtr tird;
};

struct InfoHandle
{
	InfoHandle(HWND d, HWND fd, HWND rd, HWND chadi = nullptr)
		: dialog(d), tifdInfo(fd), tirdInfo(rd), chandidateInfo(chadi) { }
	~InfoHandle() { }

	void SetNullptr() {
		dialog = nullptr; 
		tifdInfo = nullptr; 
		tirdInfo = nullptr; 
		chandidateInfo = nullptr;
	}
	HWND dialog;
	HWND tifdInfo;
	HWND tirdInfo;
	HWND chandidateInfo;
};

struct MapInfo
{
	void SetCenter(float centLat, float centLong);

	float centerLat = 0;
	float centerLong = 0;
};

struct MapPendingInfo : public MapInfo
{
	void SetPos(pair<float, float> pos);
	void SetPos(float posLat, float posLong);
	
	float posLat;
	float posLong;
};

struct MapPairingInfo : public MapInfo
{
	void SetPos(pair<float, float> tifd, pair<float, float> tird);
	void SetPos(float tifdLat, float tifdLong, float tirdLat, float tirdLong);

	float tifdLat;
	float tifdLong;
	float tirdLat;
	float tirdLong;
};

// TIFD �� �ĺ����� �����鿡 �ʿ��� ����
struct PossiblePairingList
{
	PossiblePairingList(TirdRef ref, int32 distance)
		: target(ref), fistDistance(distance), currentDistacne(distance), timeCount(0), isStart(false) { }
	~PossiblePairingList() { target = nullptr; }

	int32 fistDistance;
	int32 currentDistacne;
	int32 timeCount;
	bool  isStart;
	TirdRef target = nullptr;
};