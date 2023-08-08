#pragma once

class RecvBuffer
{
	enum { BUFFER_COUNT = 10 };
public:
	RecvBuffer(uint32 size) : bufferSize(size), allocSize(size * BUFFER_COUNT), readPos(0), writePos(0)
	{
		buffer.resize(allocSize);
	}
	~RecvBuffer() {  }

	void	Clean();
	bool	OnRead(int32 numOfBytes);
	bool	OnWrite(int32 numOfBytes);
	BYTE*	ReadPos() { return &buffer[readPos]; }
	BYTE*	WritePos() { return &buffer[writePos]; }
	int32	DataSize() { return writePos - readPos; }
	int32	FreeSize() { return allocSize - writePos; }

private:
	vector<BYTE> buffer;
	int32 bufferSize;
	int32 allocSize;
	int32 readPos;
	int32 writePos;
};

