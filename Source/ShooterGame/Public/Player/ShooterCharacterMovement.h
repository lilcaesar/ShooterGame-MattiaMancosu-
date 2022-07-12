// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * Movement component meant for use with Pawns.
 */

#pragma once
#include "ShooterCharacterMovement.generated.h"

class UShooterSavedMove_Character;
class UShooterCharacterMovement;

UCLASS()
class UShooterCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()
	public:
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	
	//Teleport distance in centimeters, default is 1000 (10 meters)
	UPROPERTY(EditAnywhere, Category = "Teleport")
	float TeleportDistance = 1000.f;

	virtual float GetMaxSpeed() const override;	

	//Performs actual teleport
	virtual void DoTeleport();
	
	void SetTeleport(bool IsPressingTeleport);

	/* True if teleport button is being pressed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Teleport")
	bool bIsPressingTeleport;
	
protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
};

class UShooterSavedMove_Character : public FSavedMove_Character
{
public:
	virtual void Clear() override;
	virtual uint8 GetCompressedFlags() const override;
	virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
	virtual void PrepMoveFor(ACharacter* C) override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
	
	uint32 bPressedTeleport:1;
};

class FNetworkPredictionData_Client_SCMovement : public FNetworkPredictionData_Client_Character
{
public:
	FNetworkPredictionData_Client_SCMovement(const UCharacterMovementComponent& ClientMovement);

	typedef FNetworkPredictionData_Client_Character Super;

	///@brief Allocates a new copy of our custom saved move
	virtual FSavedMovePtr AllocateNewMove() override;
};