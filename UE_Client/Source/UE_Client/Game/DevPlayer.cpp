// Fill out your copyright notice in the Description page of Project Settings.


#include "DevPlayer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ADevPlayer::ADevPlayer() :
	PlayerInfo(new Protocol::ObjectInfo()),
	NextPos(new Protocol::PosInfo())
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	GetCharacterMovement()->bRunPhysicsWithNoController = true;

	// Set Protocols
	CurrentPos = PlayerInfo->mutable_pos_info();
	CurrentPos->set_move_state(Protocol::MOVE_STATE_IDLE);
	NextPos->set_move_state(Protocol::MOVE_STATE_IDLE);
}

ADevPlayer::~ADevPlayer()
{
	delete NextPos;
	delete PlayerInfo;
}

// Called when the game starts or when spawned
void ADevPlayer::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ADevPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bMyPlayer == false)
	{
		bool bUpdated = UpdatePos();

		// 현재위치 -> 목적지 벡터
		FVector DirVector = FVector(CurrentPos->x(), CurrentPos->y(), CurrentPos->z()) - GetActorLocation();
		if (CurrentPos->move_state() == Protocol::MOVE_STATE_RUN)
		{
			// 목적지까지의 방향벡터를 구한 뒤 이동
			DirVector.Normalize();
			AddMovementInput(DirVector);
		}
		else if (CurrentPos->move_state() == Protocol::MOVE_STATE_IDLE)
		{
			// 목적지와 현재 위치간 오차가 기준 초과일 경우 위치 보정
			if (DirVector.Length() > 2.0)
			{
				DirVector.Normalize();
				AddMovementInput(DirVector);
			}

			SetActorRotation(FRotator(0, CurrentPos->yaw(), 0));
		}
	}
}

// Called to bind functionality to input
void ADevPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

bool ADevPlayer::UpdatePos()
{
	if (CurrentPos->x() != NextPos->x() ||
		CurrentPos->y() != NextPos->y() ||
		CurrentPos->z() != NextPos->z() ||
		CurrentPos->yaw() != NextPos->yaw() ||
		CurrentPos->move_state() != NextPos->move_state())
	{
		SetCurrentPos(*NextPos);
		return true;
	}
	else
	{
		return false;
	}
}

void ADevPlayer::SetPlayerInfo(const Protocol::ObjectInfo& Info)
{
	PlayerInfo->CopyFrom(Info);
	CurrentPos = PlayerInfo->mutable_pos_info();
	SetNextPos(Info.pos_info());
}

void ADevPlayer::SetCurrentPos(const Protocol::PosInfo& PosInfo)
{
	CurrentPos->CopyFrom(PosInfo);
}

void ADevPlayer::SetNextPos(const Protocol::PosInfo& PosInfo)
{
	NextPos->CopyFrom(PosInfo);
}
