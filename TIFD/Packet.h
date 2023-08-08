#pragma once
#define HEAD_SIZE           sizeof(PktHead)

// Header
struct PktHead
{
    CommandType command;
    uint8      category;       // type
    uint8       answer;         // id
    // InputSize;
    uint32      payloadSize;
};

struct StTime
{
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
};

struct StConfig
{
    uint16 nServerPort;

    double  dDistanceForCheck;      // distance for pairing check
    double  dSpeedForCheck;         // minimum speed for pairing check
    double  dDistanceAccuracy;      // distance accuracy when pairing check
    int     nPairingCount;
};

struct StTirdData
{
    uint8_t nTirdVer[4];

    char    deviceId[12];
    float   lat;
    float   lon;
    float   alt;
    float   speed;
    int     sat;

    int 	nLoraCh;
    int8_t  nLoraPower;     // power
    uint8_t nLoraSF;        // spreading factor
    uint8_t nLoraBW;        // bandwidth
    uint8_t nLoraCR;        // coding rate

    float   battery;

    StTime  stTime;
};

struct StTifdData
{
    uint8_t nTifdVer[4];

    char    deviceId[12];
    int     trainNo;
    int     trainLength;
    int     trainStatus;

    float   lat;
    float   lon;
    float   alt;
    float   speed;
    int     sat;
    int     distance;

    uint8_t nLoraVer[4];
    int     nLoraCh;
    int8_t  nLoraPower;     // power
    uint8_t nLoraSF;        // spreading factor
    uint8_t nLoraBW;        // bandwidth
    uint8_t nLoraCR;        // coding rate

    char    nTirdRcvId[12];
    int8_t  nRssi;

    StTime  stTime;
};