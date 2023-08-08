#include "pch.h"
#include "Utils.h"

using namespace std;

// 지구의 반지름 (단위: km)
const double EARTH_RADIUS = 6371.0;

// 수동으로 원주율 값을 정의
const double M_PI = 3.14159265358979323846;

// 라디안으로 변환
float toRadians(float degree) {
    return degree * M_PI / 180.0;
}

// Haversine 공식을 사용하여 두 지점 사이의 거리 계산 (미터 단위로 변환)
int32 CalculateDistance(float lat1, float lon1, float lat2, float lon2) {
    float dLat = toRadians(lat2 - lat1);
    float dLon = toRadians(lon2 - lon1);

    float a = std::sin(dLat / 2) * std::sin(dLat / 2) +
        std::cos(toRadians(lat1)) * std::cos(toRadians(lat2)) *
        std::sin(dLon / 2) * std::sin(dLon / 2);

    float c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

    int32 distance = EARTH_RADIUS * c * 1000.0; // 킬로미터를 미터로 변환
    return abs(distance);
}

int32 CalculateDistance(pair<float, float>& tifd, pair<float, float>& tird)
{
    return CalculateDistance(tifd.first, tird.second, tird.first, tird.second);
}

int32 GetTestDistance(float p1, float p2)
{
    float distance = (p1 - p2) * 1000;
    int normal = static_cast<int32>(distance);

    return abs(normal);
}

std::wstring StringToWstring(string str)
{
    return std::wstring(str.begin(), str.end());
}

std::string WstringToString(std::wstring str)
{
    return std::string(str.begin(), str.end());
}

SendBufferRef MakeSendBuffer(CommandType type)
{
    SendBufferRef sendBuf = make_shared<SendBuffer>(HEAD_SIZE);

    PktHead* head = reinterpret_cast<PktHead*>(sendBuf->Data());
    head->command = type;
    head->answer = 0;
    head->category = DeviceNone;
    head->payloadSize = 0;

    return sendBuf;
}

SendBufferRef MakeSendPairingBuffer(char* deviceId, int32 length)
{
    SendBufferRef sendBuf = make_shared<SendBuffer>(HEAD_SIZE + 16);

    PktHead* head = reinterpret_cast<PktHead*>(sendBuf->Data());
    head->command = CmdType_Paring;
    head->answer = 0;
    head->category = ePairState::PairState_Pair;
    head->payloadSize = 16;

    memcpy((sendBuf->Data() + HEAD_SIZE), deviceId, 12);
    memcpy((sendBuf->Data() + HEAD_SIZE + 12), &length, 4);

    return sendBuf;
}

SendBufferRef MakeSendUnPairingBuffer()
{
    SendBufferRef sendBuf = make_shared<SendBuffer>(HEAD_SIZE);

    PktHead* head = reinterpret_cast<PktHead*>(sendBuf->Data());
    head->command = CmdType_Paring;
    head->answer = 0;
    head->category = ePairState::PairState_Unpair;
    head->payloadSize = 0;

    return sendBuf;
}
SendBufferRef MakeSendLoraBuffer(int32 ch)
{
    SendBufferRef sendBuf = make_shared<SendBuffer>(HEAD_SIZE + 1);

    PktHead* head = reinterpret_cast<PktHead*>(sendBuf->Data());
    head->command = CmdType_LoRa_CH;
    head->answer = 0;
    head->category = 0;
    head->payloadSize = 1;

    memcpy((sendBuf->Data() + HEAD_SIZE), &ch, 1);

    return sendBuf;
}