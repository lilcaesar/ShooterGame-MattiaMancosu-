// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Player/ShooterCharacterMovement.h"

//----------------------------------------------------------------------//
// UPawnMovementComponent
//----------------------------------------------------------------------//
UShooterCharacterMovement::UShooterCharacterMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsPressingTeleport = false;
}


class FNetworkPredictionData_Client* UShooterCharacterMovement::GetPredictionData_Client() const
{
	if (!ClientPredictionData)
	{
		UShooterCharacterMovement* MutableThis = const_cast<UShooterCharacterMovement*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_SCMovement(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

float UShooterCharacterMovement::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	const AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(PawnOwner);
	if (ShooterCharacterOwner)
	{
		if (ShooterCharacterOwner->IsTargeting())
		{
			MaxSpeed *= ShooterCharacterOwner->GetTargetingSpeedModifier();
		}
		if (ShooterCharacterOwner->IsRunning())
		{
			MaxSpeed *= ShooterCharacterOwner->GetRunningSpeedModifier();
		}
	}

	return MaxSpeed;
}

void UShooterCharacterMovement::DoTeleport()
{
	FVector CharacterForwardVector = CharacterOwner->GetActorForwardVector();
	FRotator CharacterRotation = CharacterOwner->GetActorRotation();
	FHitResult Hit;
	//Perform teleport using UMovementComponent::SafeMoveUpdatedComponent
	SafeMoveUpdatedComponent(CharacterForwardVector*TeleportDistance, CharacterRotation, true, Hit, ETeleportType::TeleportPhysics);

	SetTeleport(false);
}

void UShooterCharacterMovement::SetTeleport(bool IsPressingTeleport)
{
	bIsPressingTeleport = IsPressingTeleport;
}

void UShooterCharacterMovement::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bIsPressingTeleport = (Flags & UShooterSavedMove_Character::FLAG_Custom_0) != 0;
}

void UShooterCharacterMovement::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation,
	const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	
	if(bIsPressingTeleport)
	{
		DoTeleport();
	}
}

void UShooterSavedMove_Character::Clear()
{
	FSavedMove_Character::Clear();
	
	bPressedTeleport = false;
}

uint8 UShooterSavedMove_Character::GetCompressedFlags() const
{
	uint8 Result = FSavedMove_Character::GetCompressedFlags();

	//Adding custom flags
	if(bPressedTeleport)
	{
		Result |= FLAG_Custom_0;
	}

	return Result;
}

void UShooterSavedMove_Character::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel,
	FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);
	
	UShooterCharacterMovement *MovementComponent = Cast<UShooterCharacterMovement>(CharacterOwner->GetCharacterMovement());
	if(MovementComponent)
	{
		bPressedTeleport = MovementComponent->bIsPressingTeleport;
	}
}

void UShooterSavedMove_Character::PrepMoveFor(ACharacter* C)
{
	FSavedMove_Character::PrepMoveFor(C);
	
	UShooterCharacterMovement *MovementComponent = Cast<UShooterCharacterMovement>(CharacterOwner->GetCharacterMovement());
	if(MovementComponent)
	{
		MovementComponent->SetTeleport(bPressedTeleport);
	}
}

bool UShooterSavedMove_Character::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter,
	float MaxDelta) const
{
	if (bPressedTeleport != ((UShooterSavedMove_Character*)&NewMove)->bPressedTeleport)
		return false;
	
	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}


FNetworkPredictionData_Client_SCMovement::FNetworkPredictionData_Client_SCMovement(const UCharacterMovementComponent& ClientMovement)
: Super(ClientMovement)
{

}

FSavedMovePtr FNetworkPredictionData_Client_SCMovement::AllocateNewMove()
{
	return FSavedMovePtr(new UShooterSavedMove_Character());
}