// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class FSocket;
class FRecvWorker;
class FSendWorker;
class FSendBuffer;

/**
 * 
 */
class UE_CLIENT_API FPacketSession : public TSharedFromThis<FPacketSession>
{
public:
	FPacketSession(FSocket* Socket);
	~FPacketSession();

public:
	void Run();
	void Stop();

	// Recv Queue
	bool PushRecvPacket(TArray<BYTE>& Packet);
	bool PopRecvPacket(OUT TArray<BYTE>& Packet);

	// Send Queue
	bool PushSendBuffer(TSharedPtr<FSendBuffer> SendBuffer);
	bool PopSendBuffer(OUT TSharedPtr<FSendBuffer>& SendBuffer);

private:
	FSocket* Socket = nullptr;

	// NetworkWorker
	TSharedPtr<FRecvWorker> RecvWorkerThread = nullptr;
	TSharedPtr<FSendWorker> SendWorkerThread = nullptr;

	// Recv, Send Queue
	TQueue<TArray<BYTE>> RecvQueue;
	TQueue<TSharedPtr<FSendBuffer>> SendQueue;
};
