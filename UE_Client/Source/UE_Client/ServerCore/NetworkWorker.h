// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class FSocket;
class PacketSession;
class SendBuffer;

/**
 * 
 */
class UE_CLIENT_API NetworkWorker : public FRunnable
{
public:
	NetworkWorker(FSocket* socket, TWeakPtr<PacketSession> packetSession);
	virtual ~NetworkWorker();

public:
	void Destroy();

protected:
	FRunnableThread* _thread = nullptr;
	bool _bRunning = true;
	FSocket* _socket = nullptr;
	TWeakPtr<PacketSession> _packetSession = nullptr;
};

/* RecvWorker */
class RecvWorker : public NetworkWorker
{
public:
	RecvWorker(FSocket* socket, TWeakPtr<PacketSession> packetSession);
	virtual ~RecvWorker();

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

private:
	bool RecvPacket(OUT TArray<BYTE>& packet);
	bool RecvDataBySize(uint32 dataSize, OUT BYTE* buffer);
};

/* SendWorker */
class SendWorker : public NetworkWorker
{
public:
	SendWorker(FSocket* socket, TWeakPtr<PacketSession> packetSession);
	virtual ~SendWorker();

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

public:
	bool SendPacket(TSharedPtr<SendBuffer> sendBuffer);
	
private:
	bool SendDataBySize(uint32 dataSize, BYTE* data);
};