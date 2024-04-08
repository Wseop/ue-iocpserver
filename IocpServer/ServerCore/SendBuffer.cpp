#include "pch.h"
#include "SendBuffer.h"

SendBuffer::SendBuffer(uint32 bufferSize) :
    _bufferSize(bufferSize)
{
    _buffer.resize(bufferSize);
}

SendBuffer::~SendBuffer()
{
}

BYTE* SendBuffer::Buffer()
{
    return _buffer.data();
}
