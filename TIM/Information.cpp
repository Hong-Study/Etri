#include "pch.h"
#include "Utils.h"

void PendingTifdListViewItem::SetInfo(StTifdData& data)
{
	version = std::format(L"{0}.{1}.{2}.{3}", data.nTifdVer[0], data.nTifdVer[1], data.nTifdVer[2], data.nTifdVer[3]);
	SetDevice(data.deviceId);
	train = to_wstring(data.trainNo);
	trainLength = to_wstring(data.trainLength);
	SetTrainStatus(data.trainStatus);
	latitude = to_wstring(data.lat);
	longitude = to_wstring(data.lon);
	altitude = to_wstring(data.alt);
	speed = to_wstring(data.speed);
	sat = to_wstring(data.sat);
	distance = to_wstring(data.distance);

	loraVersion = std::format(L"{0}.{1}.{2}.{3}", data.nLoraVer[0], data.nLoraVer[1], data.nLoraVer[2], data.nLoraVer[3]);		loraCh = to_wstring(data.nLoraCh);
	loraPower = to_wstring(data.nLoraPower);
	loraSF = to_wstring(data.nLoraSF);
	loraBW = to_wstring(data.nLoraBW);
	loraCR = to_wstring(data.nLoraCR);

	rssi = to_wstring(data.nRssi);

	// TODO
	time = std::format(L"{0}:{1}:{2}", data.stTime.hour, data.stTime.min, data.stTime.sec);
}

void PendingTifdListViewItem::SetTrainStatus(int32 type)
{
	switch (type)
	{
	case enumTrainStatus::TrainStatus_Normal:
		trainStatus = L"Normal";
		break;
	case enumTrainStatus::TrainStatus_OpenAlarm:
		trainStatus = L"OpenAlarm";
		break;
	case enumTrainStatus::TrainStatus_OpenAlarmRequest:
		trainStatus = L"OpenAlarmRequest";
		break;
	case enumTrainStatus::TrainStatus_OpenTunnelAlarm:
		trainStatus = L"OpenTunnelAlarm";
		break;
	case enumTrainStatus::TrainStatus_TunnelAlarm:
		trainStatus = L"TunnelAlarm";
		break;
	default:
		trainStatus = L"";
	}
}

void PendingTirdListViewItem::SetInfo(StTirdData& data)
{
	version = std::format(L"{0}.{1}.{2}.{3}", data.nTirdVer[0], data.nTirdVer[1], data.nTirdVer[2], data.nTirdVer[3]);
	SetDevice(data.deviceId);

	latitude = to_wstring(data.lat);
	longitude = to_wstring(data.lon);
	altitude = to_wstring(data.alt);
	speed = to_wstring(data.speed);
	sat = to_wstring(data.sat);
	loraCh = to_wstring(data.nLoraCh);
	loraPower = to_wstring(data.nLoraPower);
	loraSF = to_wstring(data.nLoraSF);
	loraBW = to_wstring(data.nLoraBW);
	loraCR = to_wstring(data.nLoraCR);

	battery = to_wstring(data.battery);
	time = std::format(L"{0}:{1}:{2}", data.stTime.hour, data.stTime.min, data.stTime.sec);
}

void MapPendingInfo::SetPos(pair<float, float> pos)
{
	SetPos(pos.first, pos.second);
}

void MapPendingInfo::SetPos(float posLat, float posLong)
{
	this->posLat = posLat;
	this->posLong = posLong;

	if (posLat == 0 || posLong == 0)
		return;

	if (CalculateDistance(posLat, posLong, centerLat, centerLong) >= 700)
	{
		SetCenter(posLat, posLong);
	}
}

void MapPairingInfo::SetPos(pair<float, float> tifd, pair<float, float> tird)
{
	SetPos(tifd.first, tifd.second, tird.first, tird.second);
}

void MapPairingInfo::SetPos(float tifdLat, float tifdLong, float tirdLat, float tirdLong)
{
	this->tifdLat = tifdLat;
	this->tifdLong = tifdLong;
	this->tirdLat = tirdLat;
	this->tirdLong = tirdLong;

	// 일단 선택
	if (tifdLat == 0.0f || tifdLong == 0.0f)
		return;

	// 여쭤보기
	if (CalculateDistance(tifdLat, tifdLong, centerLat, centerLong) >= 1000)
	{
		SetCenter(tifdLat, tifdLong);
	}
}

void MapInfo::SetCenter(float centLat, float centLong)
{
	this->centerLat = centLat;
	this->centerLong = centLong;
}
