// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class FSocket;
class FPacketSession;
class FSendBuffer;

/**
 * 
 */
class UE_CLIENT_API FNetworkWorker : public FRunnable
{
public:
	FNetworkWorker(FSocket* Socket, TWeakPtr<FPacketSession> PacketSession);
	virtual ~FNetworkWorker();

public:
	void Destroy();

protected:
	FRunnableThread* Thread = nullptr;
	bool bRunning = true;
	FSocket* Socket = nullptr;
	TWeakPtr<FPacketSession> PacketSession = nullptr;
};

/* RecvWorker */
class FRecvWorker : public FNetworkWorker
{
public:
	FRecvWorker(FSocket* Socket, TWeakPtr<FPacketSession> PacketSession);
	virtual ~FRecvWorker();

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

private:
	bool RecvPacket(OUT TArray<BYTE>& Packet);
	bool RecvDataBySize(uint32 DataSize, OUT BYTE* Buffer);
};

/* SendWorker */
class FSendWorker : public FNetworkWorker
{
public:
	FSendWorker(FSocket* Socket, TWeakPtr<FPacketSession> PacketSession);
	virtual ~FSendWorker();

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;
	
private:
	bool SendPacket(TSharedPtr<FSendBuffer> SendBuffer);
	bool SendDataBySize(uint32 DataSize, BYTE* Data);
};