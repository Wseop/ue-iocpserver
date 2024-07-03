#include "pch.h"
#include "RecvBuffer.h"

RecvBuffer::RecvBuffer(uint32 bufferUnitSize) :
    _bufferUnitSize(bufferUnitSize),
    _capacity(bufferUnitSize* BUFFER_COUNT)
{
    _buffer.resize(_capacity);
}

RecvBuffer::~RecvBuffer()
{
}

BYTE* RecvBuffer::data()
{
    assert(_readCursor < _capacity);
    return &_buffer[_readCursor];
}

BYTE* RecvBuffer::buffer()
{
    assert(_writeCursor < _capacity);
    return &_buffer[_writeCursor];
}

uint32 RecvBuffer::dataSize() const
{
    assert(_readCursor <= _writeCursor);
    return _writeCursor - _readCursor;
}

uint32 RecvBuffer::bufferSize() const
{
    assert(_writeCursor <= _capacity);
    return _capacity - _writeCursor;
}

bool RecvBuffer::onRead(uint32 numOfBytes)
{
    if (dataSize() < numOfBytes)
    {
        spdlog::error("[RecvBuffer] Read Overflow");
        return false;
    }

    _readCursor += numOfBytes;
    return true;
}

bool RecvBuffer::onWrite(uint32 numOfBytes)
{
    if (bufferSize() < numOfBytes)
    {
        spdlog::error("[RecvBuffer] Write Overflow");
        return false;
    }

    _writeCursor += numOfBytes;
    return true;
}

void RecvBuffer::cleanCursor()
{
    uint32 _dataSize = dataSize();

    // ���� �����Ͱ� ������ 0���� �ʱ�ȭ
    if (_dataSize == 0)
    {
        _readCursor = 0;
        _writeCursor = 0;
    }
    // ������ ���������� �����ϸ� �����͸� �� ������ ���ܼ� ���� Ȯ��
    else if (bufferSize() < _bufferUnitSize)
    {
        ::memcpy(_buffer.data(), data(), _dataSize);
        _readCursor = 0;
        _writeCursor = _dataSize;
    }
}
