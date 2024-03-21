// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../ServerCore/Protocol.pb.h"
#include "DevPlayer.generated.h"

UCLASS()
class UE_CLIENT_API ADevPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADevPlayer();
	virtual ~ADevPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	bool UpdatePlayerInfo();

public:
	bool IsMyPlayer() { return bMyPlayer; }
	void SetMyPlayer(bool bValue) { bMyPlayer = bValue; }

	Protocol::PlayerInfo& GetCurrentInfo() { return CurrentInfo; }
	void SetCurrentInfo(Protocol::PlayerInfo& Info, bool bForce);

	Protocol::PlayerInfo& GetNextInfo() { return NextInfo; }
	void SetNextInfo(Protocol::PlayerInfo& Info, bool bForce);

protected:
	const float TICK_SEND_MOVE = 0.2f;
	float CurrentTickSendMove = TICK_SEND_MOVE;

	bool bMyPlayer = false;

	Protocol::PlayerInfo CurrentInfo;
	Protocol::PlayerInfo NextInfo;
};
