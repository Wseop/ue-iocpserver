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
	if (_playerId == 0)
		return;

	FString message = FString::Printf(TEXT("Player Id : %d"), _playerId);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, message);
}

void UClientGameInstance::ConnectToServer()
{
	_socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(TEXT("Stream"), TEXT("ClientSocket"));

	FIPv4Address ip;
	FIPv4Address::Parse(TEXT("127.0.0.1"), ip);

	TSharedRef<FInternetAddr> internetAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	internetAddr->SetIp(ip.Value);
	internetAddr->SetPort(7777);

	bool bConnected = _socket->Connect(*internetAddr);

	if (bConnected)
	{
		// 서버 연결 성공, 세션 시작
		UE_LOG(LogTemp, Log, TEXT("서버 연결 성공"));

		ClientPacketHandler::Init();

		_packetSession = MakeShared<PacketSession>(_socket);
		_packetSession->Run();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("서버 연결 실패"));
	}
}

void UClientGameInstance::DisconnectFromServer()
{
	if (_packetSession)
	{
		_packetSession->Stop();
		_packetSession = nullptr;
	}

	if (_socket)
	{
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(_socket);
		_socket = nullptr;
	}
}

void UClientGameInstance::ProcessRecvPacket()
{
	TArray<BYTE> recvPacket;

	while (_packetSession->PopRecvPacket(recvPacket))
	{
		ClientPacketHandler::HandlePacket(_packetSession, recvPacket.GetData());
	}
}

void UClientGameInstance::SendPing()
{
	UE_LOG(LogTemp, Log, TEXT("Ping!"));

	_packetSession->PushSendBuffer(ClientPacketHandler::MakePing());
}

void UClientGameInstance::EnterGameRoom()
{
	if (_playerId != 0)
		return;

	_packetSession->PushSendBuffer(ClientPacketHandler::MakeC_Enter());
}

void UClientGameInstance::SpawnPlayer(Protocol::PlayerInfo player)
{
	UWorld* world = GetWorld();

	if (world == nullptr)
		return;

	// 중복 체크
	if (_players.Find(player.player_id()) != nullptr)
		return;

	FVector spawnLocation(player.x(), player.y(), player.z());
	AActor* spawnedPlayer = world->SpawnActor(_playerClass, &spawnLocation);

	_players.Add(player.player_id(), spawnedPlayer);
}

void UClientGameInstance::ExitGameRoom()
{
	if (_playerId == 0)
		return;

	_packetSession->PushSendBuffer(ClientPacketHandler::MakeC_Exit(_playerId));
}

void UClientGameInstance::DespawnPlayer(uint64 playerId)
{
	if (_players.Find(playerId) == nullptr)
		return;

	if (UWorld* world = GetWorld())
	{
		if (world->DestroyActor(_players[playerId]))
			_players.Remove(playerId);
	}
}

void UClientGameInstance::ProcessExit()
{
	// player 정리
	for (auto& p : _players)
	{
		GWorld->DestroyActor(p.Value);
	}
	_players.Reset();

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Player %d Leave"), _playerId));
	
	_playerId = 0;
}
