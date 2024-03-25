#include "pch.h"
#include "RecvBuffer.h"

RecvBuffer::RecvBuffer(uint32 bufferSize) :
    _bufferSize(bufferSize)
{
    _buffer.resize(bufferSize * BUFFER_COUNT);
}

RecvBuffer::~RecvBuffer()
{
}

BYTE* RecvBuffer::ReadPos()
{
    return &_buffer[_readPos];
}

BYTE* RecvBuffer::WritePos()
{
    return &_buffer[_writePos];
}

uint32 RecvBuffer::DataSize()
{
    assert(_readPos <= _writePos);
    return _writePos - _readPos;
}

uint32 RecvBuffer::FreeSize()
{
    uint32 capacity = static_cast<uint32>(_buffer.size());
    assert(_writePos <= capacity);
    return capacity - _writePos;
}

bool RecvBuffer::OnRead(uint32 numOfBytes)
{
    if (numOfBytes > DataSize())
    {
        spdlog::error("Buffer Read Overflow");
        return false;
    }

    _readPos += numOfBytes;
    return true;
}

bool RecvBuffer::OnWrite(uint32 numOfBytes)
{
    if (numOfBytes > FreeSize())
    {
        spdlog::error("Buffer Write Overflow");
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
