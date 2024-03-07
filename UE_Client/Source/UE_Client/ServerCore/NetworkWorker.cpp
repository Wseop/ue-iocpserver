// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkWorker.h"
#include "Sockets.h"
#include "PacketSession.h"
#include "ClientPacketHandler.h"

NetworkWorker::NetworkWorker(FSocket* socket, TWeakPtr<PacketSession> packetSession) :
	_socket(socket),
	_packetSession(packetSession)
{
}

NetworkWorker::~NetworkWorker()
{
}

void NetworkWorker::Destroy()
{
	_bRunning = false;
}

/* RecvWorker */

RecvWorker::RecvWorker(FSocket* socket, TWeakPtr<PacketSession> packetSession) :
	NetworkWorker(socket, packetSession)
{
	_thread = FRunnableThread::Create(this, TEXT("RecvWorkerThread"));
}

RecvWorker::~RecvWorker()
{
}

bool RecvWorker::Init()
{
	return true;
}

uint32 RecvWorker::Run()
{
	// Packet을 수신하여 Session에 전달
	while (_bRunning)
	{
		if (TSharedPtr<PacketSession> packetSession = _packetSession.Pin())
		{
			TArray<BYTE> packet;

			if (RecvPacket(packet))
			{
				packetSession->PushRecvPacket(packet);
			}
		}
	}

	return 0;
}

void RecvWorker::Stop()
{
}

void RecvWorker::Exit()
{
}

bool RecvWorker::RecvPacket(OUT TArray<BYTE>& packet)
{
	// Header 수신
	const uint32 headerSize = sizeof(PacketHeader);
	packet.AddZeroed(headerSize);
	if (RecvDataBySize(headerSize, packet.GetData()) == false)
		return false;

	// Payload 수신
	PacketHeader* header = reinterpret_cast<PacketHeader*>(packet.GetData());
	const uint32 payloadSize = header->packetSize - headerSize;
	packet.AddZeroed(payloadSize);
	if (RecvDataBySize(payloadSize, packet.GetData() + headerSize) == false)
		return false;

	return true;
}

bool RecvWorker::RecvDataBySize(uint32 dataSize, OUT BYTE* buffer)
{
	uint32 pendingDataSize = 0;

	if (_socket->HasPendingData(pendingDataSize) == false || pendingDataSize == 0)
		return false;

	// dataSize 만큼의 데이터 수신
	uint32 bufferOffset = 0;
	
	while (dataSize > 0)
	{
		int32 numOfBytes = 0;

		if (_socket->Recv(buffer + bufferOffset, dataSize, numOfBytes) == false)
			return false;

		check(numOfBytes <= static_cast<int32>(dataSize));

		bufferOffset += numOfBytes;
		dataSize -= numOfBytes;
	}

	return true;
}

/* SendWorker */

SendWorker::SendWorker(FSocket* socket, TWeakPtr<PacketSession> packetSession) :
	NetworkWorker(socket, packetSession)
{
	_thread = FRunnableThread::Create(this, TEXT("SendWorkerThread"));
}

SendWorker::~SendWorker()
{
}

bool SendWorker::Init()
{
	return true;
}

uint32 SendWorker::Run()
{
	// SendQueue에 보낼 데이터가 있으면 전송
	while (_bRunning)
	{
		if (TSharedPtr<PacketSession> packetSession = _packetSession.Pin())
		{
			TSharedPtr<SendBuffer> sendBuffer = nullptr;

			if (packetSession->PopSendBuffer(sendBuffer))
			{
				SendPacket(sendBuffer);
			}
		}
	}

	return 0;
}

void SendWorker::Stop()
{
}

void SendWorker::Exit()
{
}

bool SendWorker::SendPacket(TSharedPtr<SendBuffer> sendBuffer)
{
	return SendDataBySize(sendBuffer->GetBufferSize(), sendBuffer->Buffer());
}

bool SendWorker::SendDataBySize(uint32 dataSize, BYTE* data)
{
	// dataSize 만큼의 데이터 전송
	while (dataSize > 0)
	{
		int32 numOfBytes = 0;

		if (_socket->Send(data, dataSize, numOfBytes) == false)
			return false;

		dataSize -= numOfBytes;
	}

	return true;
}
