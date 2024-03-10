// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
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
	UFUNCTION(BlueprintCallable)
	void ConnectToServer();

	void DisconnectFromServer();

	UFUNCTION(BlueprintCallable)
	void ProcessRecvPacket();

	UFUNCTION(BlueprintCallable)
	void SendPing();

	UFUNCTION(BlueprintCallable)
	void EnterGameRoom();

private:
	FSocket* _socket = nullptr;
	TSharedPtr<PacketSession> _packetSession = nullptr;
};
