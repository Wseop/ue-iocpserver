// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class UE_CLIENT_API FSendBuffer
{
public:
	FSendBuffer(uint32 BufferSize);
	~FSendBuffer();

public:
	BYTE* GetBuffer();
	uint32 GetBufferSize();

private:
	TArray<BYTE> Buffer;
};
