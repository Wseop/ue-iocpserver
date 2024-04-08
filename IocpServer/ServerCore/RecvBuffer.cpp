#include "pch.h"
#include "RecvBuffer.h"

RecvBuffer::RecvBuffer(uint32 bufferSize) :
    _bufferSize(bufferSize),
    _capacity(bufferSize* BUFFER_COUNT)
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

uint32 RecvBuffer::DataSize() const
{
    assert(_readPos <= _writePos);
    return _writePos - _readPos;
}

uint32 RecvBuffer::FreeSize() const
{
    assert(_writePos <= _capacity);
    return _capacity - _writePos;
}

bool RecvBuffer::OnRead(uint32 numOfBytes)
{
    if (DataSize() < numOfBytes)
    {
        spdlog::error("RecvBuffer : Read Overflow");
        return false;
    }

    _readPos += numOfBytes;
    return true;
}

bool RecvBuffer::OnWrite(uint32 numOfBytes)
{
    if (FreeSize() < numOfBytes)
    {
        spdlog::error("RecvBuffer : Write Overflow");
        return false;
    }

    _writePos += numOfBytes;
    return true;
}

void RecvBuffer::Clean()
{
    uint32 dataSize = DataSize();

    // 읽을 데이터가 없으면 0으로 초기화
    if (dataSize == 0)
    {
        _readPos = 0;
        _writePos = 0;
    }
    // 버퍼의 여유공간이 부족하면 데이터를 맨 앞으로 땡겨서 공간 확보
    else if (FreeSize() < _bufferSize)
    {
        ::memcpy(_buffer.data(), ReadPos(), dataSize);
        _readPos = 0;
        _writePos = dataSize;
    }
}
