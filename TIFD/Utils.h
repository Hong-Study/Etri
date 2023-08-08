#pragma once
#include "Types.h"
#include "Packet.h"

// 동적 메모리 정리는 어떻게 할지 결정해야함.
template<typename T>
BYTE* MakeSendBuffer(T data, CommandType type, uint8 answer, uint8 category)
{
	int32 dataSize = sizeof(T);
	int32 totalSize = dataSize + 8;
	BYTE* sendBuf = new BYTE[totalSize];

	PktHead* head = reinterpret_cast<PktHead*>(sendBuf);
	head->command = type;
	head->answer = answer;
	head->category = category;
	head->payloadSize = dataSize;

	T* datas = reinterpret_cast<T*>(&head[1]);
	memcpy(datas, &data, sizeof(T));

	return sendBuf;
}