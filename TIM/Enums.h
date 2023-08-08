#pragma once

enum ePairState
{
	PairState_Unpair = 0
	, PairState_Pair
};

enum CommandType : uint16
{
	CmdType_Tifd_Info = 0x01,
	CmdType_Tird_Info = 0x02,
	CmdType_LoRa_CH = 0x03,
	CmdType_Paring = 0x04,
	CmdType_Alarm = 0x05,
	CmdType_Reg_Request = 0x06,             // category : DeviceTIFD, DeviceTIRD
	CmdType_Reg_Confirm = 0x07,
	CmdType_KeepAlive = 0xff
};

enum Device : uint8 {
	DeviceNone
	, DeviceRegRequest
	, DeviceTIFD
	, DeviceTIRD
	, DevicePair
};

enum class PendingTifdListViewCategory : uint8
{
	Information = 0
	, TableTifd_No
	, TableTifd_TrainNo
	, TableTifd_DeviceID
	, TableTifd_Lat
	, TableTifd_Lon
	, TableTifd_Speed
	, TableTifd_Sat
	, CategorySize
};

enum class PendingTirdListViewCategory : uint8
{
	Information = 0
	,TableTird_No
	, TableTird_DeviceID
	, TableTird_Lat
	, TableTird_Lon
	, TableTird_Speed
	, TableTird_Sat
	, TableTird_Battery
	, CategorySize
};

enum class PairingListViewCategory : uint8
{
	Information
	, Table_No
	, Table_TrainNo
	, Table_Type
	, Table_DeviceId
	, Table_Latitude
	, Table_Longitude
	, Table_Speed
	, Table_Sat
	, Table_Distance
	, Table_Length
	, Table_Status // Alram
	, Table_Battery
	, CategorySize
};

enum class LogListViewCategory : uint8
{
	Time
	, Contents
};

enum List
{
	TIFD_LIST = 1
	, TIRD_LIST
	, PAIRING_LIST
};

enum enumTrainStatus
{
	TrainStatus_Normal = 0,
	TrainStatus_OpenAlarm,
	TrainStatus_OpenAlarmRequest,
	TrainStatus_OpenTunnelAlarm,
	TrainStatus_TunnelAlarm
};

enum class TifdInfoCategory : uint8
{
	Version = 0
	, Device
	, PairingStatus
	, Network_Information
	, IpAddress
	, Port
	, GPS_Information
	, Time
	, Latitude
	, Longitude
	, Altitude
	, Speed
	, Satellite
	, Distance
	, LORA_Information
	, LoRaVersion
	, Channel
	, Power
	, Bandwidth
	, SpreadingFactor
	, CodingRate
	, LoRa_Receive_Information
	, TIRD_Device
	, RSSI
	, SIZE
};
///////////////////////////////////////////////////////

enum class TirdInfoCategory : uint8
{
	Version = 0
	, Device
	, PairingStatus
	, Network_Information 
	, IpAddress
	, Port
	, GPS_Information
	, Time
	, Latitude
	, Longitude
	, Altitude
	, Speed
	, Satellite
	, LORA_Information
	, Channel
	, Power
	, Bandwidth
	, SpreadingFactor
	, CodingRate
	, Battery_Information
	, Battery_Voltage
	, SIZE
};
///////////////////////////////////////////////////////