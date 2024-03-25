// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientGameInstance.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Common/TcpSocketBuilder.h"
#include "ServerCore/PacketSession.h"
#include "ServerCore/ClientPacketHandler.h"
#include "Game/DevPlayer.h"
#include "Game/MyPlayer.h"
#include "Kismet/GameplayStatics.h"

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
	
	Protocol::Ping Payload;
	Payload.set_msg("Ping!");
	PacketSession->PushSendBuffer(FClientPacketHandler::MakePing(&Payload));

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Ping!"));
}

void UClientGameInstance::EnterGame()
{
	if (PacketSession == nullptr)
		return;

	if (bEntered == false)
	{
		bEntered = true;

		Protocol::C_Enter Payload;
		PacketSession->PushSendBuffer(FClientPacketHandler::MakeC_Enter(&Payload));
	}
}

void UClientGameInstance::HandleEnterGame(const Protocol::S_Enter& Payload)
{
	if (Payload.result())
	{
		// Set MyPlayer
		const Protocol::ObjectInfo& PlayerInfo = Payload.my_object_info();
		MyPlayer = Cast<AMyPlayer>(UGameplayStatics::GetPlayerController(this, 0)->GetPawn());
		if (MyPlayer == nullptr)
			return;
		MyPlayer->SetPlayerInfo(PlayerInfo);
		
		const Protocol::PosInfo& PosInfo = PlayerInfo.pos_info();
		MyPlayer->SetActorLocation(FVector(PosInfo.x(), PosInfo.y(), PosInfo.z()));

		// Spawn Other Players
		for (const Protocol::ObjectInfo& Info : Payload.other_object_infos())
			SpawnPlayer(Info);

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Enter Success")));
	}
	else
	{
		bEntered = false;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Enter Fail"));
	}
}

void UClientGameInstance::SpawnPlayer(const Protocol::ObjectInfo& PlayerInfo)
{
	const uint32 PlayerId = PlayerInfo.object_id();
	if (Players.Find(PlayerId) != nullptr)
		return;

	const Protocol::PosInfo& Pos = PlayerInfo.pos_info();
	FVector Location(Pos.x(), Pos.y(), Pos.z());

	ADevPlayer* NewPlayer = Cast<ADevPlayer>(GWorld->SpawnActor(PlayerClass, &Location));
	NewPlayer->SetPlayerInfo(PlayerInfo);
	Players.Add(PlayerId, NewPlayer);
}

void UClientGameInstance::ExitGame()
{
	if (PacketSession == nullptr)
		return;

	if (bEntered)
	{
		bEntered = false;

		Protocol::C_Exit Payload;
		PacketSession->PushSendBuffer(FClientPacketHandler::MakeC_Exit(&Payload));
	}
}

void UClientGameInstance::HandleExitGame(const Protocol::S_Exit& Payload)
{
	if (Payload.result())
	{
		DespawnAll();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Exit Success")));
	}
	else
	{
		bEntered = true;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Exit Fail"));
	}
}

void UClientGameInstance::DespawnPlayer(const uint32 Id)
{
	ADevPlayer** Player = Players.Find(Id);
	if (Player == nullptr)
		return;
	GWorld->DestroyActor(*Player);
	Players.Remove(Id);
}

void UClientGameInstance::DespawnAll()
{
	for (auto& Element : Players)
		GWorld->DestroyActor(Element.Value);
	Players.Reset();
}

void UClientGameInstance::MovePlayer(const Protocol::PosInfo& PosInfo)
{
	if (PacketSession == nullptr || bEntered == false)
		return;

	Protocol::C_Move Payload;
	Payload.mutable_pos_info()->CopyFrom(PosInfo);
	PacketSession->PushSendBuffer(FClientPacketHandler::MakeC_Move(&Payload));
}

void UClientGameInstance::HandleMove(const Protocol::PosInfo& PosInfo)
{
	ADevPlayer** Player = Players.Find(PosInfo.object_id());
	if (Player == nullptr)
		return;
	(*Player)->SetNextPos(PosInfo);
}