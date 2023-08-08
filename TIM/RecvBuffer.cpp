#include "pch.h"
#include "RecvBuffer.h"

void RecvBuffer::Clean()
{
	if (readPos == writePos)
	{
		readPos = writePos = 0;
	}
	else
	{
		int32 dataSize = DataSize();
		if (dataSize <= bufferSize)
		{
			memcpy(&buffer[0], &buffer[readPos], dataSize);
			readPos = 0;
			writePos = dataSize;
		}
	}
}

bool RecvBuffer::OnRead(int32 numOfBytes)
{
	if (numOfBytes > DataSize())
		return false;

	readPos += numOfBytes;
	if (readPos > writePos || readPos > allocSize)
		return false;
	return true;
}

bool RecvBuffer::OnWrite(int32 numOfBytes)
{
	if (numOfBytes >= FreeSize())
		return false;

	writePos += numOfBytes;

	if (writePos > allocSize)
		return false;

	return true;
}
