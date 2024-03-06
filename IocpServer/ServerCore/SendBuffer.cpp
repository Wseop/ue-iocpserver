#include "pch.h"
#include "SendBuffer.h"

SendBuffer::SendBuffer(uint32 bufferSize)
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

uint32 SendBuffer::GetBufferSize()
{
    return static_cast<uint32>(_buffer.size());
}
