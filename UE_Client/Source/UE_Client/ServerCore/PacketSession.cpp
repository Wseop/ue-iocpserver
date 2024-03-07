// Fill out your copyright notice in the Description page of Project Settings.


#include "PacketSession.h"
#include "Sockets.h"
#include "NetworkWorker.h"

PacketSession::PacketSession(FSocket* socket) :
	_socket(socket)
{
}

PacketSession::~PacketSession()
{
}

void PacketSession::Run()
{
	// NetworkWorker Thread ½ÇÇà
	_recvWorkerThread = MakeShared<RecvWorker>(_socket, AsShared());
	_sendWorkerThread = MakeShared<SendWorker>(_socket, AsShared());
}

void PacketSession::Stop()
{
	if (_recvWorkerThread)
	{
		_recvWorkerThread->Destroy();
		_recvWorkerThread = nullptr;
	}

	if (_sendWorkerThread)
	{
		_sendWorkerThread->Destroy();
		_sendWorkerThread = nullptr;
	}
}

bool PacketSession::PushRecvPacket(TArray<BYTE>& packet)
{
	return _recvQueue.Enqueue(packet);
}

bool PacketSession::PopRecvPacket(OUT TArray<BYTE>& packet)
{
	return _recvQueue.Dequeue(packet);
}

bool PacketSession::PushSendBuffer(TSharedPtr<SendBuffer> sendBuffer)
{
	return _sendQueue.Enqueue(sendBuffer);
}

bool PacketSession::PopSendBuffer(OUT TSharedPtr<SendBuffer>& sendBuffer)
{
	return _sendQueue.Dequeue(sendBuffer);
}
