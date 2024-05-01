// Fill out your copyright notice in the Description page of Project Settings.


#include "PacketSession.h"
#include "Sockets.h"
#include "NetworkWorker.h"

FPacketSession::FPacketSession(FSocket* Socket) :
	Socket(Socket)
{
}

FPacketSession::~FPacketSession()
{
}

void FPacketSession::Run()
{
	// NetworkWorker Thread ½ÇÇà
	RecvWorkerThread = MakeShared<FRecvWorker>(Socket, AsShared());
	SendWorkerThread = MakeShared<FSendWorker>(Socket, AsShared());
}

void FPacketSession::Stop()
{
	if (RecvWorkerThread)
	{
		RecvWorkerThread->Destroy();
		RecvWorkerThread = nullptr;
	}

	if (SendWorkerThread)
	{
		SendWorkerThread->Destroy();
		SendWorkerThread = nullptr;
	}
}

bool FPacketSession::PushRecvPacket(TArray<BYTE>& Packet)
{
	return RecvQueue.Enqueue(Packet);
}

bool FPacketSession::PopRecvPacket(OUT TArray<BYTE>& Packet)
{
	return RecvQueue.Dequeue(Packet);
}

bool FPacketSession::PushSendBuffer(TSharedPtr<FSendBuffer> SendBuffer)
{
	return SendQueue.Enqueue(SendBuffer);
}

bool FPacketSession::PopSendBuffer(OUT TSharedPtr<FSendBuffer>& SendBuffer)
{
	return SendQueue.Dequeue(SendBuffer);
}
