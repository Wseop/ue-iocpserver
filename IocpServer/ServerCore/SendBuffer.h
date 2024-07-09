#pragma once

class SendBuffer
{
public:
	SendBuffer(uint32 bufferSize);
	~SendBuffer();

public:
	BYTE* buffer();
	uint32 bufferSize() const { return _bufferSize; }

private:
	vector<BYTE> _buffer;
	uint32 _bufferSize = 0;
};

