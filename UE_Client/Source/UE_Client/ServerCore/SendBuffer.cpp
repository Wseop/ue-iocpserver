// Fill out your copyright notice in the Description page of Project Settings.


#include "SendBuffer.h"

SendBuffer::SendBuffer(uint32 bufferSize)
{
	_buffer.AddZeroed(bufferSize);
}

SendBuffer::~SendBuffer()
{
}

BYTE* SendBuffer::Buffer()
{
	return _buffer.GetData();
}

uint32 SendBuffer::GetBufferSize()
{
	return static_cast<uint32>(_buffer.Num());
}
