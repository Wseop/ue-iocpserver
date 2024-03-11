// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientPacketHandler.h"
#include "PacketSession.h"
#include "Protocol.pb.h"
#include "../ClientGameInstance.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

void FClientPacketHandler::Init()
{
	for (uint16 i = 0; i < UINT16_MAX; i++)
		GPacketHandler[i] = HandleInvalid;

	GPacketHandler[static_cast<uint16>(EPacketType::Ping)] = HandlePing;
	GPacketHandler[static_cast<uint16>(EPacketType::S_Enter)] = HandleS_Enter;
	GPacketHandler[static_cast<uint16>(EPacketType::S_Exit)] = HandleS_Exit;
	GPacketHandler[static_cast<uint16>(EPacketType::S_SpawnPlayer)] = HandleS_SpawnPlayer;
	GPacketHandler[static_cast<uint16>(EPacketType::S_DespawnPlayer)] = HandleS_DespawnPlayer;
}

void FClientPacketHandler::HandlePacket(TSharedPtr<FPacketSession> PacketSession, BYTE* Packet)
{
	// 헤더 parsing
	EPacketType PacketType = EPacketType::None;
	uint32 PayloadSize = 0;
	BYTE* Payload = HandleHeader(Packet, OUT PacketType, OUT PayloadSize);

	// PacketType에 맞는 Handler 호출
	GPacketHandler[static_cast<uint16>(PacketType)](PacketSession, Payload, PayloadSize);
}

BYTE* FClientPacketHandler::HandleHeader(BYTE* Packet, OUT EPacketType& PacketType, OUT uint32& PayloadSize)
{
	FPacketHeader* Header = reinterpret_cast<FPacketHeader*>(Packet);

	// PacketType, PayloadSize 추출
	PacketType = Header->PacketType;
	PayloadSize = Header->PacketSize - sizeof(FPacketHeader);

	// Payload 반환
	return reinterpret_cast<BYTE*>(Header + 1);
}

void FClientPacketHandler::HandleInvalid(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Invalid Packet!"));
}

void FClientPacketHandler::HandlePing(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize)
{
	Protocol::Ping Ping;
	Ping.ParseFromArray(Payload, PayloadSize);

	FString Message = FString::Printf(TEXT("%s"), *FString(Ping.msg().c_str()));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Message);
}

void FClientPacketHandler::HandleS_Enter(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize)
{
	Protocol::S_Enter Enter;
	Enter.ParseFromArray(Payload, PayloadSize);

	if (Enter.result() == 1)
	{
		// 입장 성공, PlayerId 설정
		if (UClientGameInstance* GameInstance = Cast<UClientGameInstance>(GWorld->GetGameInstance()))
		{
			GameInstance->SetPlayerId(Enter.player_id());
			GameInstance->ShowPlayerId();
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Enter Fail"));
	}
}

void FClientPacketHandler::HandleS_Exit(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize)
{
	Protocol::S_Exit Exit;
	Exit.ParseFromArray(Payload, PayloadSize);

	if (Exit.result() == 1)
	{
		if (UClientGameInstance* GameInstance = Cast<UClientGameInstance>(GWorld->GetGameInstance()))
		{
			GameInstance->ProcessExit();
		}
	}
}

void FClientPacketHandler::HandleS_SpawnPlayer(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize)
{
	Protocol::S_SpawnPlayer SpawnPlayer;
	SpawnPlayer.ParseFromArray(Payload, PayloadSize);

	if (UClientGameInstance* GameInstance = Cast<UClientGameInstance>(GWorld->GetGameInstance()))
	{
		for (auto& Player : SpawnPlayer.players())
		{
			GameInstance->SpawnPlayer(Player);
		}
	}
}

void FClientPacketHandler::HandleS_DespawnPlayer(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize)
{
	Protocol::S_DespawnPlayer DespawnPlayer;
	DespawnPlayer.ParseFromArray(Payload, PayloadSize);

	if (UClientGameInstance* GameInstance = Cast<UClientGameInstance>(GWorld->GetGameInstance()))
	{
		for (auto& PlayerId : DespawnPlayer.player_ids())
		{
			GameInstance->DespawnPlayer(PlayerId);
		}
	}
}

TSharedPtr<FSendBuffer> FClientPacketHandler::MakePing()
{
	Protocol::Ping Ping;
	Ping.set_msg("Ping!");

	return MakeSendBuffer(EPacketType::Ping, &Ping);
}

TSharedPtr<FSendBuffer> FClientPacketHandler::MakeC_Enter()
{
	Protocol::C_Enter Enter;
	Enter.set_key("12345");

	return MakeSendBuffer(EPacketType::C_Enter, &Enter);
}

TSharedPtr<FSendBuffer> FClientPacketHandler::MakeC_Exit(uint64 PlayerId)
{
	Protocol::C_Exit Exit;
	Exit.set_player_id(PlayerId);

	return MakeSendBuffer(EPacketType::C_Exit, &Exit);
}
