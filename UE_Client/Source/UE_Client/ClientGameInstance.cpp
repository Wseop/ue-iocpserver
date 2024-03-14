﻿// Fill out your copyright notice in the Description page of Project Settings.


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
	TArray<BYTE> RecvPacket;

	while (PacketSession->PopRecvPacket(RecvPacket))
	{
		FClientPacketHandler::HandlePacket(PacketSession, RecvPacket.GetData());
	}
}

void UClientGameInstance::SendPing()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Ping!"));

	PacketSession->PushSendBuffer(FClientPacketHandler::MakePing());
}

void UClientGameInstance::EnterGame()
{
	if (EnterId == 0)
	{
		// 중복 입장 방지
		EnterId = 1;
		PacketSession->PushSendBuffer(FClientPacketHandler::MakeC_Enter("123"));
	}
}

void UClientGameInstance::ExitGame()
{
	if (EnterId != 0)
	{
		PacketSession->PushSendBuffer(FClientPacketHandler::MakeC_Exit(EnterId));

		// 중복 퇴장 방지
		EnterId = 0;
	}
}

void UClientGameInstance::Spawn()
{
	// 입장된 상태일때 스폰 가능
	if (EnterId != 0)
	{
		PacketSession->PushSendBuffer(FClientPacketHandler::MakeC_Spawn(1));
	}
}

void UClientGameInstance::Despawn(uint32 Id)
{
	AActor** Player = Players.Find(Id);

	if (Player == nullptr)
		return;

	GWorld->DestroyActor(*Player);
	Players.Remove(Id);
}

void UClientGameInstance::DespawnAll()
{
	for (auto Element : Players)
	{
		GWorld->DestroyActor(Element.Value);
	}
	Players.Reset();
}

void UClientGameInstance::AddPlayer(Protocol::PlayerInfo Info)
{
	FVector Location(Info.x(), Info.y(), Info.z());
	AActor* Player = GWorld->SpawnActor(PlayerClass, &Location);

	Players.Add(Info.player_id(), Player);
}

void UClientGameInstance::RemovePlayer(uint32 PlayerId)
{
	Players.Remove(PlayerId);
}
