// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class FSocket;
class RecvWorker;
class SendWorker;
class SendBuffer;

/**
 * 
 */
class UE_CLIENT_API PacketSession : public TSharedFromThis<PacketSession>
{
public:
	PacketSession(FSocket* socket);
	~PacketSession();

public:
	void Run();
	void Stop();

	// Recv Queue
	bool PushRecvPacket(TArray<BYTE>& packet);
	bool PopRecvPacket(OUT TArray<BYTE>& packet);

	// Send Queue
	bool PushSendBuffer(TSharedPtr<SendBuffer> sendBuffer);
	bool PopSendBuffer(OUT TSharedPtr<SendBuffer>& sendBuffer);

private:
	FSocket* _socket = nullptr;

	// NetworkWorker
	TSharedPtr<RecvWorker> _recvWorkerThread = nullptr;
	TSharedPtr<SendWorker> _sendWorkerThread = nullptr;

	// Recv, Send Queue
	TQueue<TArray<BYTE>> _recvQueue;
	TQueue<TSharedPtr<SendBuffer>> _sendQueue;
};
