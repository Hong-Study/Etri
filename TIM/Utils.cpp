#include "pch.h"
#include "Utils.h"

using namespace std;

// ������ ������ (����: km)
const double EARTH_RADIUS = 6371.0;

// �������� ������ ���� ����
const double M_PI = 3.14159265358979323846;

// �������� ��ȯ
float toRadians(float degree) {
    return degree * M_PI / 180.0;
}

// Haversine ������ ����Ͽ� �� ���� ������ �Ÿ� ��� (���� ������ ��ȯ)
int32 CalculateDistance(float lat1, float lon1, float lat2, float lon2) {
    float dLat = toRadians(lat2 - lat1);
    float dLon = toRadians(lon2 - lon1);

    float a = std::sin(dLat / 2) * std::sin(dLat / 2) +
        std::cos(toRadians(lat1)) * std::cos(toRadians(lat2)) *
        std::sin(dLon / 2) * std::sin(dLon / 2);

    float c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

    int32 distance = EARTH_RADIUS * c * 1000.0; // ų�ι��͸� ���ͷ� ��ȯ
    return abs(distance);
}

int32 CalculateDistance(pair<float, float>& tifd, pair<float, float>& tird)
{
    return CalculateDistance(tifd.first, tird.second, tird.first, tird.second);
}

std::tm GetLocalTime()
{
    auto now = std::chrono::system_clock::now();

    // ���� �ð��� std::time_t Ÿ������ ��ȯ�մϴ�.
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    // ������ ������ ������ std::localtime_s �Լ��� ����Ͽ� ��¥�� �ð� ������ �и��մϴ�.
    std::tm local_time;
    localtime_s(&local_time, &now_time);

    return local_time;
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

SendBufferRef MakeSendLoraBuffer(uint8 ch)
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