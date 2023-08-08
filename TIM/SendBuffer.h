#pragma once

class SendBuffer
{
public:
	SendBuffer(uint32 size) : allocSize(size), buffer(new BYTE[size]) { }
	~SendBuffer() { delete[] buffer; }

	BYTE* Data() { return buffer; }
	uint32 Size() { return allocSize; }

private:
	BYTE* buffer;
	uint32 allocSize;
};

