#pragma once

class RecvBuffer
{
public:
	RecvBuffer(uint32 bufferSize);
	~RecvBuffer();

public:
	BYTE* data();
	BYTE* buffer();

	uint32 dataSize() const;
	uint32 bufferSize() const;

	// 처리한 Byte만큼 Read 커서 이동
	bool onRead(uint32 numOfBytes);

	// 받아온 Byte만큼 Write 커서 이동
	bool onWrite(uint32 numOfBytes);

	// 커서 정리
	void cleanCursor();

private:
	const uint32 BUFFER_COUNT = 10;

	vector<BYTE> _buffer;
	uint32 _readCursor = 0;
	uint32 _writeCursor = 0;
	uint32 _bufferUnitSize = 0;
	uint32 _capacity = 0;
};

