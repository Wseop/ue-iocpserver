// Fill out your copyright notice in the Description page of Project Settings.


#include "SendBuffer.h"

FSendBuffer::FSendBuffer(uint32 BufferSize)
{
	Buffer.AddZeroed(BufferSize);
}

FSendBuffer::~FSendBuffer()
{
}

BYTE* FSendBuffer::GetBuffer()
{
	return Buffer.GetData();
}

uint32 FSendBuffer::GetBufferSize()
{
	return static_cast<uint32>(Buffer.Num());
}
