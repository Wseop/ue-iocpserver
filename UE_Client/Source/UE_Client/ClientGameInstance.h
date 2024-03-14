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
	uint32 GetEnterId() { return EnterId; }
	void SetEnterId(uint32 Id) { EnterId = Id; }

	void AddPlayer(Protocol::PlayerInfo Info);
	void RemovePlayer(uint32 PlayerId);

public:
	UFUNCTION(BlueprintCallable)
	void ConnectToServer();
	
	void DisconnectFromServer();

	UFUNCTION(BlueprintCallable)
	void ProcessRecvPacket();

public:
	UFUNCTION(BlueprintCallable)
	void SendPing();

	UFUNCTION(BlueprintCallable)
	void EnterGame();

	UFUNCTION(BlueprintCallable)
	void ExitGame();

	UFUNCTION(BlueprintCallable)
	void Spawn();

	void Despawn(uint32 Id);
	void DespawnAll();

protected:
	UPROPERTY(Editanywhere)
	TSubclassOf<AActor> PlayerClass;

private:
	FSocket* Socket = nullptr;
	TSharedPtr<FPacketSession> PacketSession = nullptr;

	uint32 EnterId = 0;
	TMap<uint32, AActor*> Players;
};
