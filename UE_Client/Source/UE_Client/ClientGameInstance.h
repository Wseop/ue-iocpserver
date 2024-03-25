// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ServerCore/Protocol.pb.h"
#include "ClientGameInstance.generated.h"

class FSocket;
class FPacketSession;
class ADevPlayer;
class AMyPlayer;

/**
 * 
 */
UCLASS()
class UE_CLIENT_API UClientGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	virtual void FinishDestroy() override;

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
	void HandleEnterGame(const Protocol::S_Enter& Payload);
	void SpawnPlayer(const Protocol::ObjectInfo& PlayerInfo);

	UFUNCTION(BlueprintCallable)
	void ExitGame();
	void HandleExitGame(const Protocol::S_Exit& Payload);
	void DespawnPlayer(const uint32 Id);
	void DespawnAll();

	void MovePlayer(const Protocol::PosInfo& PosInfo);
	void HandleMove(const Protocol::PosInfo& PosInfo);

protected:
	UPROPERTY(Editanywhere)
	TSubclassOf<AActor> PlayerClass;

private:
	FSocket* Socket = nullptr;
	TSharedPtr<FPacketSession> PacketSession = nullptr;

	bool bEntered = false;
	AMyPlayer* MyPlayer = nullptr;
	TMap<uint32, ADevPlayer*> Players;
};
