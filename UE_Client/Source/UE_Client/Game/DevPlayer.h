// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../ServerCore/Struct.pb.h"
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
	bool UpdatePos();

public:
	bool IsMyPlayer() { return bMyPlayer; }
	void SetMyPlayer(bool bValue) { bMyPlayer = bValue; }

	Protocol::ObjectInfo* GetPlayerInfo() const { return PlayerInfo; }
	void SetPlayerInfo(const Protocol::ObjectInfo& Info);

	Protocol::PosInfo* GetCurrentPos() const { return CurrentPos; }
	void SetCurrentPos(const Protocol::PosInfo& PosInfo);

	Protocol::PosInfo* GetNextPos() const { return NextPos; }
	void SetNextPos(const Protocol::PosInfo& PosInfo);

protected:
	const float TICK_SEND_MOVE = 0.2f;
	float CurrentTickSendMove = TICK_SEND_MOVE;

	bool bMyPlayer = false;
	Protocol::ObjectInfo* PlayerInfo = nullptr;
	Protocol::PosInfo* CurrentPos = nullptr;
	Protocol::PosInfo* NextPos = nullptr;
};
