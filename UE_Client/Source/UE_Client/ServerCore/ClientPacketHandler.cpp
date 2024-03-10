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
	GPacketHandler[static_cast<uint16>(PacketType::S_Enter)] = HandleS_Enter;
}

void ClientPacketHandler::HandlePacket(TSharedPtr<PacketSession> packetSession, BYTE* packet)
{
	// 헤더 parsing
	PacketType packetType = PacketType::None;
	uint32 payloadSize = 0;
	BYTE* payload = HandleHeader(packet, OUT packetType, OUT payloadSize);

	// PacketType에 맞는 Handler 호출
	GPacketHandler[static_cast<uint16>(packetType)](packetSession, payload, payloadSize);
}

BYTE* ClientPacketHandler::HandleHeader(BYTE* packet, OUT PacketType& packetType, OUT uint32& payloadSize)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(packet);

	// PacketType, PayloadSize 추출
	packetType = header->packetType;
	payloadSize = header->packetSize - sizeof(PacketHeader);

	// Payload 반환
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

void ClientPacketHandler::HandleS_Enter(TSharedPtr<PacketSession> packetSession, BYTE* payload, uint32 payloadSize)
{
	Protocol::S_Enter sEnter;
	sEnter.ParseFromArray(payload, payloadSize);

	// 입장 성공 여부 출력
	FString result;
	
	if (sEnter.result() == 1)
		result = FString("Enter Success");
	else
		result = FString("Enter Failed");

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, result);
}

TSharedPtr<SendBuffer> ClientPacketHandler::MakePing()
{
	Protocol::Ping ping;
	ping.set_msg("Ping!");

	return MakeSendBuffer(PacketType::Ping, &ping);
}

TSharedPtr<SendBuffer> ClientPacketHandler::MakeC_Enter()
{
	Protocol::C_Enter cEnter;
	cEnter.set_key("12345");

	return MakeSendBuffer(PacketType::C_Enter, &cEnter);
}
