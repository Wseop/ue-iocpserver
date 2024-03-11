// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ServerCore/Protocol.pb.h"
#include "ClientGameInstance.generated.h"

class FSocket;
class FPacketSession;

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
	void SetPlayerId(uint64 Id) { PlayerId = Id; }

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

	void SpawnPlayer(Protocol::PlayerInfo Player);

	UFUNCTION(BlueprintCallable)
	void ExitGameRoom();

	void DespawnPlayer(uint64 PlayerId);
	void ProcessExit();

protected:
	UPROPERTY(Editanywhere)
	TSubclassOf<AActor> PlayerClass;

private:
	FSocket* Socket = nullptr;
	TSharedPtr<FPacketSession> PacketSession = nullptr;

	uint64 PlayerId = 0;
	TMap<uint64, AActor*> Players;
};
