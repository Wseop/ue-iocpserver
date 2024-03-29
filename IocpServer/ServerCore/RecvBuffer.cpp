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

    // 처리할 데이터가 없으면 커서만 초기화
    if (dataSize == 0)
    {
        _readPos = 0;
        _writePos = 0;
    }
    // 여유 공간이 적으면 남은 데이터를 맨앞으로 복사하여 공간 확보
    else if (FreeSize() < _bufferSize)
    {
        ::memcpy(_buffer.data(), ReadPos(), dataSize);
        _readPos = 0;
        _writePos = dataSize;
    }
}
