// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientPacketHandler.h"
#include "PacketSession.h"
#include "Protocol.pb.h"
#include "Kismet/GameplayStatics.h"
#include "../ClientGameInstance.h"
#include "../Game/MyPlayer.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

void FClientPacketHandler::Init()
{
	for (uint16 i = 0; i < UINT16_MAX; i++)
		GPacketHandler[i] = HandleInvalid;

	GPacketHandler[StaticCast<uint16>(EPacketType::Ping)] = HandlePing;
	GPacketHandler[StaticCast<uint16>(EPacketType::S_Enter)] = HandleS_Enter;
	GPacketHandler[StaticCast<uint16>(EPacketType::S_Exit)] = HandleS_Exit;
	GPacketHandler[StaticCast<uint16>(EPacketType::S_Spawn)] = HandleS_Spawn;
	GPacketHandler[StaticCast<uint16>(EPacketType::S_Despawn)] = HandleS_Despawn;
	GPacketHandler[StaticCast<uint16>(EPacketType::S_Move)] = HandleS_Move;
}

void FClientPacketHandler::HandlePacket(TSharedPtr<FPacketSession> PacketSession, BYTE* Packet)
{
	// 헤더 parsing
	EPacketType PacketType;
	uint32 PayloadSize = 0;
	BYTE* Payload = HandleHeader(Packet, OUT PacketType, OUT PayloadSize);

	// PacketType에 맞는 Handler 호출
	GPacketHandler[StaticCast<uint16>(PacketType)](PacketSession, Payload, PayloadSize);
}

BYTE* FClientPacketHandler::HandleHeader(BYTE* Packet, OUT EPacketType& PacketType, OUT uint32& PayloadSize)
{
	// PacketType, PayloadSize 추출
	FPacketHeader* Header = reinterpret_cast<FPacketHeader*>(Packet);
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
	Protocol::Ping Response;
	Response.ParseFromArray(Payload, PayloadSize);

	FString Log = FString::Printf(TEXT("%s"), *FString(Response.msg().c_str()));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Log);
}

void FClientPacketHandler::HandleS_Enter(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize)
{
	Protocol::S_Enter EnterPacket;
	EnterPacket.ParseFromArray(Payload, PayloadSize);

	UClientGameInstance* GameInstance = Cast<UClientGameInstance>(GWorld->GetGameInstance());
	if (GameInstance == nullptr)
		return;
	GameInstance->HandleEnterGame(EnterPacket);
}

void FClientPacketHandler::HandleS_Exit(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize)
{
	Protocol::S_Exit ExitPacket;
	ExitPacket.ParseFromArray(Payload, PayloadSize);

	UClientGameInstance* GameInstance = Cast<UClientGameInstance>(GWorld->GetGameInstance());
	if (GameInstance == nullptr)
		return;
	GameInstance->HandleExitGame(ExitPacket);
}

void FClientPacketHandler::HandleS_Spawn(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize)
{
	Protocol::S_Spawn SpawnPacket;
	SpawnPacket.ParseFromArray(Payload, PayloadSize);

	UClientGameInstance* GameInstance = Cast<UClientGameInstance>(GWorld->GetGameInstance());
	if (GameInstance == nullptr)
		return;

	GameInstance->SpawnPlayer(SpawnPacket);
}

void FClientPacketHandler::HandleS_Despawn(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize)
{
	Protocol::S_Despawn DespawnPacket;
	DespawnPacket.ParseFromArray(Payload, PayloadSize);
	
	UClientGameInstance* GameInstance = Cast<UClientGameInstance>(GWorld->GetGameInstance());
	if (GameInstance == nullptr)
		return;
	for (const uint32 Id : DespawnPacket.object_ids())
		GameInstance->DespawnPlayer(Id);
}

void FClientPacketHandler::HandleS_Move(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize)
{
	Protocol::S_Move MovePacket;
	MovePacket.ParseFromArray(Payload, PayloadSize);

	UClientGameInstance* GameInstance = Cast<UClientGameInstance>(GWorld->GetGameInstance());
	if (GameInstance == nullptr)
		return;
	GameInstance->HandleMove(MovePacket.pos_info());
}