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
	EPacketType PacketType = EPacketType::None;
	uint32 PayloadSize = 0;
	BYTE* Payload = HandleHeader(Packet, OUT PacketType, OUT PayloadSize);

	// PacketType에 맞는 Handler 호출
	GPacketHandler[StaticCast<uint16>(PacketType)](PacketSession, Payload, PayloadSize);
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
	Protocol::Ping Message;
	Message.ParseFromArray(Payload, PayloadSize);

	FString Log = FString::Printf(TEXT("%s"), *FString(Message.msg().c_str()));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Log);
}

void FClientPacketHandler::HandleS_Enter(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize)
{
	Protocol::S_Enter Message;
	Message.ParseFromArray(Payload, PayloadSize);

	UClientGameInstance* GameInstance = Cast<UClientGameInstance>(GWorld->GetGameInstance());

	if (GameInstance == nullptr)
		return;

	const uint32 EnterId = Message.enter_id();

	// 입장 성공
	if (Message.result())
	{
		GameInstance->SetEnterId(EnterId);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Enter Success - %d"), EnterId));

		// GameInstance에 MyPlayer 등록 및 위치 지정
		AMyPlayer* MyPlayer = Cast<AMyPlayer>(UGameplayStatics::GetPlayerController(GameInstance, 0)->GetPawn());
		
		if (MyPlayer == nullptr)
			return;

		GameInstance->SetMyPlayer(MyPlayer);

		Protocol::PlayerInfo PlayerInfo = Message.player_info();
		FVector NewLocation(PlayerInfo.x(), PlayerInfo.y(), PlayerInfo.z());

		// MyPlayer 정보 초기화
		MyPlayer->SetMyPlayer(true);
		MyPlayer->SetCurrentInfo(PlayerInfo, true);
		MyPlayer->SetNextInfo(PlayerInfo, true);
		MyPlayer->SetActorLocation(NewLocation);
	}
	// 입장 실패
	else
	{
		GameInstance->SetEnterId(0);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Enter Fail"));
	}
}

void FClientPacketHandler::HandleS_Exit(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize)
{
	Protocol::S_Exit Message;
	Message.ParseFromArray(Payload, PayloadSize);

	UClientGameInstance* GameInstance = Cast<UClientGameInstance>(GWorld->GetGameInstance());

	if (GameInstance == nullptr)
		return;

	const uint32 EnterId = Message.enter_id();

	// 퇴장 성공
	if (Message.result())
	{
		GameInstance->SetEnterId(0);
		GameInstance->DespawnAll();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Exit Success - %d"), EnterId));
	}
	// 퇴장 실패
	else
	{
		GameInstance->SetEnterId(Message.enter_id());
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Exit Fail"));
	}
}

void FClientPacketHandler::HandleS_Spawn(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize)
{
	Protocol::S_Spawn Message;
	Message.ParseFromArray(Payload, PayloadSize);

	UClientGameInstance* GameInstance = Cast<UClientGameInstance>(GWorld->GetGameInstance());

	if (GameInstance == nullptr)
		return;

	TArray<Protocol::PlayerInfo> PlayerInfos;

	for (auto& Info : Message.player_infos())
		PlayerInfos.Add(Info);

	GameInstance->Spawn(PlayerInfos);
}

void FClientPacketHandler::HandleS_Despawn(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize)
{
	Protocol::S_Despawn Message;
	Message.ParseFromArray(Payload, PayloadSize);
	
	UClientGameInstance* GameInstance = Cast<UClientGameInstance>(GWorld->GetGameInstance());

	if (GameInstance == nullptr)
		return;

	TArray<uint32> Ids;

	for (auto& Id : Message.player_ids())
		Ids.Add(Id);

	GameInstance->Despawn(Ids);
}

void FClientPacketHandler::HandleS_Move(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize)
{
	Protocol::S_Move Message;
	Message.ParseFromArray(Payload, PayloadSize);

	UClientGameInstance* GameInstance = Cast<UClientGameInstance>(GWorld->GetGameInstance());

	if (GameInstance == nullptr)
		return;

	Protocol::PlayerInfo Info = Message.player_info();

	GameInstance->UpdatePlayerInfo(Info);
}

TSharedPtr<FSendBuffer> FClientPacketHandler::MakePing()
{
	Protocol::Ping Payload;

	Payload.set_msg("Ping!");

	return MakeSendBuffer(EPacketType::Ping, &Payload);
}

TSharedPtr<FSendBuffer> FClientPacketHandler::MakeC_Enter(FString Key)
{
	Protocol::C_Enter Payload;

	Payload.set_enter_key(TCHAR_TO_ANSI(*Key));

	return MakeSendBuffer(EPacketType::C_Enter, &Payload);
}

TSharedPtr<FSendBuffer> FClientPacketHandler::MakeC_Exit(uint32 Id)
{
	Protocol::C_Exit Payload;

	Payload.set_enter_id(Id);

	return MakeSendBuffer(EPacketType::C_Exit, &Payload);
}

TSharedPtr<FSendBuffer> FClientPacketHandler::MakeC_Move(Protocol::PlayerInfo& Info)
{
	Protocol::C_Move Payload;

	Payload.mutable_player_info()->CopyFrom(Info);

	return MakeSendBuffer(EPacketType::C_Move, &Payload);
}
