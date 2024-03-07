// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class UE_CLIENT_API SendBuffer
{
public:
	SendBuffer(uint32 bufferSize);
	~SendBuffer();

public:
	BYTE* Buffer();
	uint32 GetBufferSize();

private:
	TArray<BYTE> _buffer;
};
