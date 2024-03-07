// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientPacketHandler.h"
#include "PacketSession.h"
#include "Protocol.pb.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

void ClientPacketHandler::Init()
{
	for (uint16 i = 0; i < UINT16_MAX; i++)
		GPacketHandler[i] = HandleInvalid;

	GPacketHandler[static_cast<uint16>(PacketType::Ping)] = HandlePing;
}

void ClientPacketHandler::HandlePacket(TSharedPtr<PacketSession> packetSession, BYTE* packet)
{
	// ��� parsing
	PacketType packetType = PacketType::None;
	uint32 payloadSize = 0;
	BYTE* payload = HandleHeader(packet, OUT packetType, OUT payloadSize);

	// PacketType�� �´� Handler ȣ��
	GPacketHandler[static_cast<uint16>(packetType)](packetSession, payload, payloadSize);
}

BYTE* ClientPacketHandler::HandleHeader(BYTE* packet, OUT PacketType& packetType, OUT uint32& payloadSize)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(packet);

	// PacketType, PayloadSize ����
	packetType = header->packetType;
	payloadSize = header->packetSize - sizeof(PacketHeader);

	// Payload ��ȯ
	return reinterpret_cast<BYTE*>(header + 1);
}

void ClientPacketHandler::HandleInvalid(TSharedPtr<PacketSession> packetSession, BYTE* payload, uint32 payloadSize)
{
	UE_LOG(LogTemp, Log, TEXT("Invalid Packet!"));
}

void ClientPacketHandler::HandlePing(TSharedPtr<PacketSession> packetSession, BYTE* payload, uint32 payloadSize)
{
	Protocol::Ping ping;
	ping.ParseFromArray(payload, payloadSize);

	UE_LOG(LogTemp, Log, TEXT("%s"), *FString(ping.msg().c_str()));
}

TSharedPtr<SendBuffer> ClientPacketHandler::MakePing()
{
	Protocol::Ping ping;
	ping.set_msg(TCHAR_TO_ANSI(*FString("Ping!")));

	return MakeSendBuffer(PacketType::Ping, &ping);
}
