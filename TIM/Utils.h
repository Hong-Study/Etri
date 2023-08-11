#pragma once
#include <cmath>
#include "SendBuffer.h"

using std::make_shared;

int32			CalculateDistance(float rat1, float lon1, float rat2, float lon2);
int32			CalculateDistance(pair<float, float>& tifd, pair<float, float>& tird);

std::wstring	StringToWstring(string str);
std::string		WstringToString(std::wstring str);

SendBufferRef	MakeSendBuffer(CommandType type);
SendBufferRef	MakeSendPairingBuffer(char* deviceId, int32 length);
SendBufferRef	MakeSendUnPairingBuffer();
SendBufferRef	MakeSendLoraBuffer(int32 ch);


template<typename T>
SendBufferRef MakeSendBuffer(const T data, CommandType type, uint8 answer, uint8 category)
{
	int32 dataSize = sizeof(T);
	int32 totalSize = dataSize + HEAD_SIZE;
	SendBufferRef sendBuf = make_shared<SendBuffer>(totalSize);

	PktHead* head = reinterpret_cast<PktHead*>(sendBuf->Data());
	head->command = type;
	head->answer = answer;
	head->category = category;
	head->payloadSize = dataSize;

	T* datas = reinterpret_cast<T*>(&head[1]);
	memcpy(datas, &data, sizeof(T));

	return sendBuf;
}

template<typename T>
SendBufferRef MakeSendBuffer(const T* data, CommandType type, uint8 answer, uint8 category)
{
	int32 dataSize = sizeof(T);
	int32 totalSize = dataSize + HEAD_SIZE;
	SendBufferRef sendBuf = make_shared<SendBuffer>(totalSize);

	PktHead* head = reinterpret_cast<PktHead*>(sendBuf->Data());
	head->command = type;
	head->answer = answer;
	head->category = category;
	head->payloadSize = dataSize;

	T* datas = reinterpret_cast<T*>(&head[1]);
	memcpy(datas, data, sizeof(T));

	return sendBuf;
}