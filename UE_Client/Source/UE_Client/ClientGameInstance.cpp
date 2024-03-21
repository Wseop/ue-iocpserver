// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientGameInstance.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Common/TcpSocketBuilder.h"
#include "ServerCore/PacketSession.h"
#include "ServerCore/ClientPacketHandler.h"
#include "Game/DevPlayer.h"
#include "Game/MyPlayer.h"

void UClientGameInstance::Init()
{
	Super::Init();
}

void UClientGameInstance::FinishDestroy()
{
	DisconnectFromServer();

	Super::FinishDestroy();
}

void UClientGameInstance::ConnectToServer()
{
	Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(TEXT("Stream"), TEXT("ClientSocket"));

	FIPv4Address Ip;
	FIPv4Address::Parse(TEXT("127.0.0.1"), Ip);

	TSharedRef<FInternetAddr> InternetAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	InternetAddr->SetIp(Ip.Value);
	InternetAddr->SetPort(7777);

	bool bConnected = Socket->Connect(*InternetAddr);

	if (bConnected)
	{
		// 서버 연결 성공, 세션 시작
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("서버 연결 성공"));

		FClientPacketHandler::Init();

		PacketSession = MakeShared<FPacketSession>(Socket);
		PacketSession->Run();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("서버 연결 실패"));
	}
}

void UClientGameInstance::DisconnectFromServer()
{
	if (PacketSession)
	{
		PacketSession->Stop();
		PacketSession = nullptr;
	}

	if (Socket)
	{
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
		Socket = nullptr;
	}
}

void UClientGameInstance::ProcessRecvPacket()
{
	if (PacketSession == nullptr)
		return;

	TArray<BYTE> RecvPacket;

	while (PacketSession->PopRecvPacket(RecvPacket))
	{
		FClientPacketHandler::HandlePacket(PacketSession, RecvPacket.GetData());
	}
}

void UClientGameInstance::SendPing()
{
	if (PacketSession == nullptr)
		return;

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Ping!"));

	PacketSession->PushSendBuffer(FClientPacketHandler::MakePing());
}

void UClientGameInstance::EnterGame()
{
	if (PacketSession == nullptr)
		return;

	if (EnterId == 0)
	{
		// 중복 입장 방지
		EnterId = 1;
		PacketSession->PushSendBuffer(FClientPacketHandler::MakeC_Enter("123"));
	}
}

void UClientGameInstance::ExitGame()
{
	if (PacketSession == nullptr)
		return;

	if (EnterId != 0)
	{
		const uint32 Id = EnterId;

		// 중복 퇴장 방지
		EnterId = 0;
		PacketSession->PushSendBuffer(FClientPacketHandler::MakeC_Exit(Id));
	}
}

void UClientGameInstance::Spawn(TArray<Protocol::PlayerInfo>& PlayerInfos)
{
	for (auto& Info : PlayerInfos)
	{
		const uint32 PlayerId = Info.player_id();

		if (Players.Find(PlayerId) != nullptr)
			continue;

		FVector Location(Info.x(), Info.y(), Info.z());
		
		ADevPlayer* SpawnedPlayer = Cast<ADevPlayer>(GWorld->SpawnActor(PlayerClass, &Location));
		SpawnedPlayer->SetCurrentInfo(Info, true);
		SpawnedPlayer->SetNextInfo(Info, true);
		
		Players.Add(PlayerId, SpawnedPlayer);
	}
}

void UClientGameInstance::Despawn(TArray<uint32> Ids)
{
	for (uint32 Id : Ids)
	{
		ADevPlayer** Player = Players.Find(Id);

		if (Player == nullptr)
			continue;

		GWorld->DestroyActor(*Player);
		Players.Remove(Id);
	}
}

void UClientGameInstance::DespawnAll()
{
	for (auto& Element : Players)
	{
		GWorld->DestroyActor(Element.Value);
	}
	Players.Reset();
}

void UClientGameInstance::SendMove(Protocol::PlayerInfo& Info)
{
	if (PacketSession == nullptr || EnterId == 0)
		return;

	PacketSession->PushSendBuffer(FClientPacketHandler::MakeC_Move(Info));
}

void UClientGameInstance::UpdatePlayerInfo(Protocol::PlayerInfo& Info)
{
	ADevPlayer** Player = Players.Find(Info.player_id());

	if (Player == nullptr)
		return;

	(*Player)->SetNextInfo(Info, false);
}
