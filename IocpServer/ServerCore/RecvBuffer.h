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

	// ó���� Byte��ŭ Read Ŀ�� �̵�
	bool OnRead(uint32 numOfBytes);

	// �޾ƿ� Byte��ŭ Write Ŀ�� �̵�
	bool OnWrite(uint32 numOfBytes);

	// Ŀ�� ����
	void Clean();

private:
	const uint32 BUFFER_COUNT = 10;

	vector<BYTE> _buffer;
	uint32 _readPos = 0;
	uint32 _writePos = 0;
	uint32 _bufferSize = 0;
	uint32 _capacity = 0;
};

