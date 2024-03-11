// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientPacketHandler.h"
#include "PacketSession.h"
#include "Protocol.pb.h"
#include "../ClientGameInstance.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

void ClientPacketHandler::Init()
{
	for (uint16 i = 0; i < UINT16_MAX; i++)
		GPacketHandler[i] = HandleInvalid;

	GPacketHandler[static_cast<uint16>(PacketType::Ping)] = HandlePing;
	GPacketHandler[static_cast<uint16>(PacketType::S_Enter)] = HandleS_Enter;
	GPacketHandler[static_cast<uint16>(PacketType::S_Exit)] = HandleS_Exit;
	GPacketHandler[static_cast<uint16>(PacketType::S_SpawnPlayer)] = HandleS_SpawnPlayer;
	GPacketHandler[static_cast<uint16>(PacketType::S_DespawnPlayer)] = HandleS_DespawnPlayer;
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

	if (sEnter.result() == 1)
	{
		// 입장 성공, PlayerId 설정
		if (UClientGameInstance* gameInstance = Cast<UClientGameInstance>(GWorld->GetGameInstance()))
		{
			gameInstance->SetPlayerId(sEnter.player_id());
			gameInstance->ShowPlayerId();
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Enter Fail"));
	}
}

void ClientPacketHandler::HandleS_Exit(TSharedPtr<PacketSession> packetSession, BYTE* payload, uint32 payloadSize)
{
	Protocol::S_Exit sExit;
	sExit.ParseFromArray(payload, payloadSize);

	if (sExit.result() == 1)
	{
		if (UClientGameInstance* gameInstance = Cast<UClientGameInstance>(GWorld->GetGameInstance()))
		{
			gameInstance->ProcessExit();
		}
	}
}

void ClientPacketHandler::HandleS_SpawnPlayer(TSharedPtr<PacketSession> packetSession, BYTE* payload, uint32 payloadSize)
{
	Protocol::S_SpawnPlayer spawnPlayer;
	spawnPlayer.ParseFromArray(payload, payloadSize);

	if (UClientGameInstance* gameInstance = Cast<UClientGameInstance>(GWorld->GetGameInstance()))
	{
		for (auto& player : spawnPlayer.players())
		{
			gameInstance->SpawnPlayer(player);
		}
	}
}

void ClientPacketHandler::HandleS_DespawnPlayer(TSharedPtr<PacketSession> packetSession, BYTE* payload, uint32 payloadSize)
{
	Protocol::S_DespawnPlayer despawnPlayer;
	despawnPlayer.ParseFromArray(payload, payloadSize);

	if (UClientGameInstance* gameInstance = Cast<UClientGameInstance>(GWorld->GetGameInstance()))
	{
		for (auto& playerId : despawnPlayer.player_ids())
		{
			gameInstance->DespawnPlayer(playerId);
		}
	}
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

TSharedPtr<SendBuffer> ClientPacketHandler::MakeC_Exit(uint64 playerId)
{
	Protocol::C_Exit cExit;
	cExit.set_player_id(playerId);

	return MakeSendBuffer(PacketType::C_Exit, &cExit);
}
