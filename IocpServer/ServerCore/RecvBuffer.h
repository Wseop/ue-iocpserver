#pragma once

class RecvBuffer
{
public:
	RecvBuffer(uint32 bufferSize);
	~RecvBuffer();

public:
	BYTE* ReadPos();
	BYTE* WritePos();

	uint32 DataSize() const;
	uint32 FreeSize() const;

	// 처리한 Byte만큼 Read 커서 이동
	bool OnRead(uint32 numOfBytes);

	// 받아온 Byte만큼 Write 커서 이동
	bool OnWrite(uint32 numOfBytes);

	// 커서 정리
	void Clean();

private:
	const uint32 BUFFER_COUNT = 10;

	vector<BYTE> _buffer;
	uint32 _readPos = 0;
	uint32 _writePos = 0;
	uint32 _bufferSize = 0;
	uint32 _capacity = 0;
};

