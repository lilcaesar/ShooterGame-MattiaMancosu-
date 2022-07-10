// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Player/ShooterCharacterMovement.h"

//----------------------------------------------------------------------//
// UPawnMovementComponent
//----------------------------------------------------------------------//
UShooterCharacterMovement::UShooterCharacterMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
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

FSavedMovePtr UShooterCharacterMovement::AllocateNewMove()
{
	return FSavedMovePtr(new UShooterSavedMove_Character());
}

bool UShooterCharacterMovement::DoTeleport(bool bReplayingMoves)
{
	if ( CharacterOwner)
	{
		bIsPressingTeleport = true;
		return true;
	}
	
	return false;
}

void UShooterCharacterMovement::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	
	AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(CharacterOwner);
	bool bWasPressingTeleport = ShooterCharacterOwner->bPressedTeleport;
	ShooterCharacterOwner->bPressedTeleport = ((Flags & UShooterSavedMove_Character::FLAG_Custom_0) != 0);
	if (CharacterOwner->GetLocalRole() == ROLE_Authority)
	{
		bIsPressingTeleport = ShooterCharacterOwner->bPressedTeleport;
		if (bIsPressingTeleport && !bWasPressingTeleport)
		{
			ShooterCharacterOwner->OnStartTeleport();
		}else if(!bIsPressingTeleport)
		{
			ShooterCharacterOwner->OnStopTeleport();
		}
	}
}

void UShooterCharacterMovement::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation,
	const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	
	AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(CharacterOwner);
	bIsPressingTeleport = ShooterCharacterOwner->bPressedTeleport;
	if(bIsPressingTeleport)
	{
		FVector CharacterForwardVector = CharacterOwner->GetActorForwardVector();
		FRotator CharacterRotation = CharacterOwner->GetActorRotation();
		FHitResult Hit;
		//Perform teleport using UMovementComponent::SafeMoveUpdatedComponent
		SafeMoveUpdatedComponent(CharacterForwardVector*TeleportDistance, CharacterRotation, true, Hit, ETeleportType::TeleportPhysics);
		bIsPressingTeleport = false;
		ShooterCharacterOwner->bPressedTeleport = false;
	}
}

uint8 UShooterSavedMove_Character::GetCompressedFlags() const
{
	uint8 Result = 0;
	//Preserving original implementation
	if (bPressedJump)
	{
		Result |= FLAG_JumpPressed;
	}

	if (bWantsToCrouch)
	{
		Result |= FLAG_WantsToCrouch;
	}

	//Adding custom flags
	if(bPressedTeleport)
	{
		Result |= FLAG_Custom_0;
	}

	return Result;
}
