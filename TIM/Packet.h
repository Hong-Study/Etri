#pragma once

#define NO_MSG_CHECK_TIME   1500
#define HEAD_SIZE           sizeof(PktHead)

// Header
// 원래이름 : struct StPacket
struct PktHead
{
    uint16      command;
	uint8       category;       // type
	uint8       answer;         // id
	// InputSize;
	uint32      payloadSize;
};

#define LORA_MAX_CH         13
#define LORA_DEFAULT_CH     1

struct StLoraInfo
{
    int32 ch = 2;
    bool bUse[LORA_MAX_CH] = { false };
};

struct StTime
{
    uint8 hour;
    uint8 min;
    uint8 sec;
};

struct StTirdData
{
    uint8_t nTirdVer[4];

    char    deviceId[12];
    float   lat;
    float   lon;
    float   alt;
    float   speed;
    int32   sat;

    int32 	nLoraCh;
    int8    nLoraPower;     // power
    uint8   nLoraSF;        // spreading factor
    uint8   nLoraBW;        // bandwidth
    uint8   nLoraCR;        // coding rate

    float   battery;

    StTime  stTime;
};

struct StTifdData
{
    uint8_t nTifdVer[4];

    char    deviceId[12];
    int32   trainNo;
    int32   trainLength;
    int32   trainStatus;

    float   lat;
    float   lon;
    float   alt;
    float   speed;
    int32   sat;
    int32   distance;

    uint8   nLoraVer[4];
    int32   nLoraCh;
    int8    nLoraPower;     // power
    uint8   nLoraSF;        // spreading factor
    uint8   nLoraBW;        // bandwidth
    uint8   nLoraCR;        // coding rate

    char    nTirdRcvId[12];
    int8    nRssi;

    StTime  stTime;
};

struct CommandPairingPacket
{
    char deviceId[12];
    int32 length;
};