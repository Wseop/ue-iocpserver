#pragma once

class SendBuffer
{
public:
	SendBuffer(uint32 bufferSize);
	~SendBuffer();

public:
	BYTE* Buffer();
	uint32 GetBufferSize();

private:
	vector<BYTE> _buffer;
};

