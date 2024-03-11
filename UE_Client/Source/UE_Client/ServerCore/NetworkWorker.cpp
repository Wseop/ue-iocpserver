// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkWorker.h"
#include "Sockets.h"
#include "PacketSession.h"
#include "ClientPacketHandler.h"

FNetworkWorker::FNetworkWorker(FSocket* Socket, TWeakPtr<FPacketSession> PacketSession) :
	Socket(Socket),
	PacketSession(PacketSession)
{
}

FNetworkWorker::~FNetworkWorker()
{
}

void FNetworkWorker::Destroy()
{
	bRunning = false;
}

/* RecvWorker */

FRecvWorker::FRecvWorker(FSocket* Socket, TWeakPtr<FPacketSession> PacketSession) :
	FNetworkWorker(Socket, PacketSession)
{
	Thread = FRunnableThread::Create(this, TEXT("RecvWorkerThread"));
}

FRecvWorker::~FRecvWorker()
{
}

bool FRecvWorker::Init()
{
	return true;
}

uint32 FRecvWorker::Run()
{
	// Packet을 수신하여 Session에 전달
	while (bRunning)
	{
		if (TSharedPtr<FPacketSession> Session = PacketSession.Pin())
		{
			TArray<BYTE> Packet;

			if (RecvPacket(Packet))
			{
				Session->PushRecvPacket(Packet);
			}
		}
	}

	return 0;
}

void FRecvWorker::Stop()
{
}

void FRecvWorker::Exit()
{
}

bool FRecvWorker::RecvPacket(OUT TArray<BYTE>& Packet)
{
	// Header 수신
	const uint32 HeaderSize = sizeof(FPacketHeader);
	Packet.AddZeroed(HeaderSize);
	if (RecvDataBySize(HeaderSize, Packet.GetData()) == false)
		return false;

	// Payload 수신
	FPacketHeader* Header = reinterpret_cast<FPacketHeader*>(Packet.GetData());
	const uint32 PayloadSize = Header->PacketSize - HeaderSize;
	Packet.AddZeroed(PayloadSize);
	if (RecvDataBySize(PayloadSize, Packet.GetData() + HeaderSize) == false)
		return false;

	return true;
}

bool FRecvWorker::RecvDataBySize(uint32 DataSize, OUT BYTE* Buffer)
{
	uint32 PendingDataSize = 0;

	if (Socket->HasPendingData(PendingDataSize) == false || PendingDataSize == 0)
		return false;

	// dataSize 만큼의 데이터 수신
	uint32 BufferOffset = 0;
	
	while (DataSize > 0)
	{
		int32 NumOfBytes = 0;

		if (Socket->Recv(Buffer + BufferOffset, DataSize, NumOfBytes) == false)
			return false;

		check(NumOfBytes <= static_cast<int32>(DataSize));

		BufferOffset += NumOfBytes;
		DataSize -= NumOfBytes;
	}

	return true;
}

/* SendWorker */

FSendWorker::FSendWorker(FSocket* Socket, TWeakPtr<FPacketSession> PacketSession) :
	FNetworkWorker(Socket, PacketSession)
{
	Thread = FRunnableThread::Create(this, TEXT("SendWorkerThread"));
}

FSendWorker::~FSendWorker()
{
}

bool FSendWorker::Init()
{
	return true;
}

uint32 FSendWorker::Run()
{
	// SendQueue에 보낼 데이터가 있으면 전송
	while (bRunning)
	{
		if (TSharedPtr<FPacketSession> Session = PacketSession.Pin())
		{
			TSharedPtr<FSendBuffer> SendBuffer = nullptr;

			if (Session->PopSendBuffer(SendBuffer))
			{
				SendPacket(SendBuffer);
			}
		}
	}

	return 0;
}

void FSendWorker::Stop()
{
}

void FSendWorker::Exit()
{
}

bool FSendWorker::SendPacket(TSharedPtr<FSendBuffer> SendBuffer)
{
	return SendDataBySize(SendBuffer->GetBufferSize(), SendBuffer->GetBuffer());
}

bool FSendWorker::SendDataBySize(uint32 DataSize, BYTE* Data)
{
	// dataSize 만큼의 데이터 전송
	while (DataSize > 0)
	{
		int32 NumOfBytes = 0;

		if (Socket->Send(Data, DataSize, NumOfBytes) == false)
			return false;

		DataSize -= NumOfBytes;
	}

	return true;
}
