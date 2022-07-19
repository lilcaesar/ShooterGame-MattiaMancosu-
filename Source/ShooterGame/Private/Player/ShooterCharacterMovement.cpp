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

bool UShooterCharacterMovement::CanJetpack()
{
	//If we are not in the falling state (ex. after jumping) or if we don't have enough fuel we do not activate the jetpack
	if ((MovementMode != EMovementMode::MOVE_Falling)||(JetpackCurrentFuel <= 0.f))
	{
		return false;
	}
	return true;
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

void UShooterCharacterMovement::ActivateJetpack()
{
}

void UShooterCharacterMovement::SetTeleport(bool IsPressingTeleport)
{
	bIsPressingTeleport = IsPressingTeleport;
}

void UShooterCharacterMovement::SetJetpackState(bool JetpackIsActive)
{
	bJetpackIsActive = JetpackIsActive;
}

void UShooterCharacterMovement::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(CustomMovementMode != CMM_JETPACK || MovementMode != MOVE_Custom)
	{
		JetpackCurrentFuel = FMath::Clamp<float>(JetpackCurrentFuel + (DeltaTime / JetpackMaxFuel), 0.f, JetpackMaxFuel);
	}
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

	if (bJetpackIsActive)
	{
		if (CanJetpack())
		{
			SetMovementMode(MOVE_Custom, CMM_JETPACK);
		}
	}
}

void UShooterCharacterMovement::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	if (PreviousMovementMode == MovementMode && PreviousCustomMode == CustomMovementMode)
	{
		Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	}
	else if(PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMM_JETPACK)
	{
		SetJetpackState(false);
	}
}

void UShooterCharacterMovement::PhysCustom(float deltaTime, int32 Iterations)
{
	if(CustomMovementMode == ECustomMovementMode::CMM_JETPACK)
	{
		PhysJetpacking(deltaTime, Iterations);
	}
	Super::PhysCustom(deltaTime, Iterations);
}

bool UShooterCharacterMovement::IsFalling() const
{
	return (Super::IsFalling() || (MovementMode == MOVE_Custom && CustomMovementMode == CMM_JETPACK));
}

void UShooterCharacterMovement::PhysJetpacking(float deltaTime, int32 Iterations)
{
	if (!bJetpackIsActive || JetpackCurrentFuel <= (deltaTime / JetpackMaxFuel))
	{
		SetJetpackState(false);
		SetMovementMode(EMovementMode::MOVE_Falling);
		StartNewPhysics(deltaTime, Iterations);
	}else
	{
		Velocity.Z += JetpackForce * deltaTime;
		JetpackCurrentFuel = FMath::Clamp<float>(JetpackCurrentFuel - (deltaTime / JetpackMaxFuel), 0.f, JetpackMaxFuel);
		PhysFalling(deltaTime, Iterations);
	}
}

void UShooterSavedMove_Character::Clear()
{
	FSavedMove_Character::Clear();
	
	bPressedTeleport = false;
	bJetpackActivated = false;
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
		bJetpackActivated = MovementComponent->bJetpackIsActive;
	}
}

void UShooterSavedMove_Character::PrepMoveFor(ACharacter* C)
{
	FSavedMove_Character::PrepMoveFor(C);
	
	UShooterCharacterMovement *MovementComponent = Cast<UShooterCharacterMovement>(CharacterOwner->GetCharacterMovement());
	if(MovementComponent)
	{
		MovementComponent->SetTeleport(bPressedTeleport);
		MovementComponent->SetJetpackState(bJetpackActivated);
	}
}

bool UShooterSavedMove_Character::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter,
	float MaxDelta) const
{
	if (bPressedTeleport != ((UShooterSavedMove_Character*)&NewMove)->bPressedTeleport)
		return false;
	if (bJetpackActivated != ((UShooterSavedMove_Character*)&NewMove)->bJetpackActivated)
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