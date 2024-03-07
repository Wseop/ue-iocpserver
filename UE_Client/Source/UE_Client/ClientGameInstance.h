// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ClientGameInstance.generated.h"

class FSocket;

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

private:
	FSocket* _socket = nullptr;
};
