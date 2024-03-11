// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ServerCore/Protocol.pb.h"
#include "ClientGameInstance.generated.h"

class FSocket;
class PacketSession;

/**
 * 
 */
UCLASS()
class UE_CLIENT_API UClientGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void FinishDestroy() override;

public:
	void SetPlayerId(uint64 playerId) { _playerId = playerId; }

	UFUNCTION(BlueprintCallable)
	void ShowPlayerId();

public:
	UFUNCTION(BlueprintCallable)
	void ConnectToServer();

	void DisconnectFromServer();

	UFUNCTION(BlueprintCallable)
	void ProcessRecvPacket();

	UFUNCTION(BlueprintCallable)
	void SendPing();

	UFUNCTION(BlueprintCallable)
	void EnterGameRoom();

	void SpawnPlayer(Protocol::PlayerInfo player);

	UFUNCTION(BlueprintCallable)
	void ExitGameRoom();

	void DespawnPlayer(uint64 playerId);
	void ProcessExit();

protected:
	UPROPERTY(Editanywhere)
	TSubclassOf<AActor> _playerClass;

private:
	FSocket* _socket = nullptr;
	TSharedPtr<PacketSession> _packetSession = nullptr;

	uint64 _playerId = 0;
	TMap<uint64, AActor*> _players;
};
