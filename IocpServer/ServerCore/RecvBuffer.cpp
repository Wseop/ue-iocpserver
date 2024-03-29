#include "pch.h"
#include "RecvBuffer.h"

RecvBuffer::RecvBuffer(uint32 bufferSize) :
    _bufferSize(bufferSize),
    _capacity(bufferSize * BUFFER_COUNT)
{
    _buffer.resize(_capacity);
}

RecvBuffer::~RecvBuffer()
{
}

BYTE* RecvBuffer::ReadPos()
{
    assert(_readPos < _capacity);
    return &_buffer[_readPos];
}

BYTE* RecvBuffer::WritePos()
{
    assert(_writePos < _capacity);
    return &_buffer[_writePos];
}

uint32 RecvBuffer::DataSize()
{
    assert(_readPos <= _writePos);
    return _writePos - _readPos;
}

uint32 RecvBuffer::FreeSize()
{
    assert(_writePos < _capacity);
    return _capacity - _writePos - 1;
}

bool RecvBuffer::OnRead(uint32 numOfBytes)
{
    if (numOfBytes > DataSize())
    {
        spdlog::error("RecvBuffer : Buffer Read Overflow");
        return false;
    }

    _readPos += numOfBytes;
    return true;
}

bool RecvBuffer::OnWrite(uint32 numOfBytes)
{
    if (numOfBytes > FreeSize())
    {
        spdlog::error("RecvBuffer : Buffer Write Overflow");
        return false;
    }

    _writePos += numOfBytes;
    return true;
}

void RecvBuffer::Clean()
{
    uint32 dataSize = DataSize();

    // ó���� �����Ͱ� ������ Ŀ���� �ʱ�ȭ
    if (dataSize == 0)
    {
        _readPos = 0;
        _writePos = 0;
    }
    // ���� ������ ������ ���� �����͸� �Ǿ����� �����Ͽ� ���� Ȯ��
    else if (FreeSize() < _bufferSize)
    {
        ::memcpy(_buffer.data(), ReadPos(), dataSize);
        _readPos = 0;
        _writePos = dataSize;
    }
}
