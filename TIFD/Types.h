#pragma once

using BYTE = unsigned char;
using int8 = __int8;
using int16 = __int16;
using int32 = __int32;
using int64 = __int64;
using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;

// 추후에 파일로 따로 저장
#define PORT 10000
#define IP L"127.0.0.1"

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
	DeviceNone,
	DeviceRegRequest,
	DeviceTIFD,
	DeviceTIRD,
	DevicePair
};

enum ePairState
{
	PairState_Unpair = 0,
	PairState_Pair
};