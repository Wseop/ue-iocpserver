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

	// ó���� Byte��ŭ Read Ŀ�� �̵�
	bool onRead(uint32 numOfBytes);

	// �޾ƿ� Byte��ŭ Write Ŀ�� �̵�
	bool onWrite(uint32 numOfBytes);

	// Ŀ�� ����
	void cleanCursor();

private:
	const uint32 BUFFER_COUNT = 10;

	vector<BYTE> _buffer;
	uint32 _readCursor = 0;
	uint32 _writeCursor = 0;
	uint32 _bufferUnitSize = 0;
	uint32 _capacity = 0;
};

