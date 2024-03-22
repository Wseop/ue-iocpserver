// Fill out your copyright notice in the Description page of Project Settings.


#include "DevPlayer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ADevPlayer::ADevPlayer()
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
}

ADevPlayer::~ADevPlayer()
{
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
		UpdatePlayerInfo();

		// 현재위치 -> 목적지 벡터
		FVector DirVector = FVector(CurrentInfo.x(), CurrentInfo.y(), CurrentInfo.z()) - GetActorLocation();
		if (CurrentInfo.move_state() == Protocol::MOVE_STATE_RUN)
		{
			// 목적지까지의 방향벡터를 구한 뒤 이동
			DirVector.Normalize();
			AddMovementInput(DirVector);
		}
		else if (CurrentInfo.move_state() == Protocol::MOVE_STATE_IDLE)
		{
			// 목적지와 현재 위치간 오차가 기준 초과일 경우 위치 보정
			if (DirVector.Length() > 2.0)
			{
				DirVector.Normalize();
				AddMovementInput(DirVector);
			}
		}
		
	}
}

// Called to bind functionality to input
void ADevPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

bool ADevPlayer::UpdatePlayerInfo()
{
	if (CurrentInfo.x() != NextInfo.x() ||
		CurrentInfo.y() != NextInfo.y() ||
		CurrentInfo.z() != NextInfo.z() ||
		CurrentInfo.yaw() != NextInfo.yaw() ||
		CurrentInfo.move_state() != CurrentInfo.move_state())
	{
		CurrentInfo = NextInfo;
		return true;
	}
	else
	{
		return false;
	}
}

void ADevPlayer::SetCurrentInfo(Protocol::PlayerInfo& Info, bool bForce)
{
	if (CurrentInfo.player_id() != Info.player_id() && bForce == false)
		return;

	CurrentInfo = Info;
}

void ADevPlayer::SetNextInfo(Protocol::PlayerInfo& Info, bool bForce)
{
	if (NextInfo.player_id() != Info.player_id() && bForce == false)
		return;

	NextInfo = Info;
}
