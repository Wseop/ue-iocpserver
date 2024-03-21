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
	uint32 GetEnterId() { return EnterId; }
	void SetEnterId(uint32 Id) { EnterId = Id; }

	AMyPlayer* GetMyPlayer() { return MyPlayer; }
	void SetMyPlayer(AMyPlayer* Player) { MyPlayer = Player; }

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

	void Spawn(TArray<Protocol::PlayerInfo>& PlayerInfos);
	void Despawn(TArray<uint32> Ids);
	void DespawnAll();

	void SendMove(Protocol::PlayerInfo& Info);
	void UpdatePlayerInfo(Protocol::PlayerInfo& Info);

protected:
	UPROPERTY(Editanywhere)
	TSubclassOf<AActor> PlayerClass;

private:
	FSocket* Socket = nullptr;
	TSharedPtr<FPacketSession> PacketSession = nullptr;

	uint32 EnterId = 0;
	AMyPlayer* MyPlayer = nullptr;
	TMap<uint32, ADevPlayer*> Players;
};
