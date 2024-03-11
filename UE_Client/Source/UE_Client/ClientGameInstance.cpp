// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientGameInstance.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Common/TcpSocketBuilder.h"
#include "ServerCore/PacketSession.h"
#include "ServerCore/ClientPacketHandler.h"

void UClientGameInstance::FinishDestroy()
{
	DisconnectFromServer();

	Super::FinishDestroy();
}

void UClientGameInstance::ShowPlayerId()
{
	if (PlayerId == 0)
		return;

	FString Message = FString::Printf(TEXT("Player Id : %d"), PlayerId);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Message);
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
		UE_LOG(LogTemp, Log, TEXT("서버 연결 성공"));

		FClientPacketHandler::Init();

		PacketSession = MakeShared<FPacketSession>(Socket);
		PacketSession->Run();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("서버 연결 실패"));
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
	TArray<BYTE> RecvPacket;

	while (PacketSession->PopRecvPacket(RecvPacket))
	{
		FClientPacketHandler::HandlePacket(PacketSession, RecvPacket.GetData());
	}
}

void UClientGameInstance::SendPing()
{
	UE_LOG(LogTemp, Log, TEXT("Ping!"));

	PacketSession->PushSendBuffer(FClientPacketHandler::MakePing());
}

void UClientGameInstance::EnterGameRoom()
{
	if (PlayerId != 0)
		return;

	PacketSession->PushSendBuffer(FClientPacketHandler::MakeC_Enter());
}

void UClientGameInstance::SpawnPlayer(Protocol::PlayerInfo Player)
{
	// 중복 체크
	if (Players.Find(Player.player_id()) != nullptr)
		return;

	FVector SpawnLocation(Player.x(), Player.y(), Player.z());
	AActor* SpawnedPlayer = GWorld->SpawnActor(PlayerClass, &SpawnLocation);

	Players.Add(Player.player_id(), SpawnedPlayer);
}

void UClientGameInstance::ExitGameRoom()
{
	if (PlayerId == 0)
		return;

	PacketSession->PushSendBuffer(FClientPacketHandler::MakeC_Exit(PlayerId));
}

void UClientGameInstance::DespawnPlayer(uint64 Id)
{
	if (Players.Find(Id) == nullptr)
		return;

	if (GWorld->DestroyActor(Players[Id]))
		Players.Remove(Id);
}

void UClientGameInstance::ProcessExit()
{
	// player 정리
	for (auto& Elem : Players)
	{
		GWorld->DestroyActor(Elem.Value);
	}
	Players.Reset();

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Player %d Leave"), PlayerId));
	
	PlayerId = 0;
}
