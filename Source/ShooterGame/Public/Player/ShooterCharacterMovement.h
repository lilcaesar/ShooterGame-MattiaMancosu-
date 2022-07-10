// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * Movement component meant for use with Pawns.
 */

#pragma once
#include "ShooterCharacterMovement.generated.h"

UCLASS()
class UShooterCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()

	//Teleport distance in centimeters, default is 1000 (10 meters)
	UPROPERTY(EditAnywhere, Category = "Teleport")
	float TeleportDistance = 1000.f;

	virtual float GetMaxSpeed() const override;

	virtual FSavedMovePtr AllocateNewMove();
	
	/**
	* Perform Teleport. Called by Character when a teleport has been detected because ShooterCharacter->bPressedTeleport was true. Checks ShooterCharacter->CanTeleport().
	* @param	bReplayingMoves: true if this is being done as part of replaying moves on a locally controlled client after a server correction.
	* @return	True if the teleport was triggered successfully.
	*/
	virtual bool DoTeleport(bool bReplayingMoves);
	
protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	/* True if teleport button is been pressed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Teleport")
	bool bIsPressingTeleport;
};

class UShooterSavedMove_Character : public FSavedMove_Character
{
	virtual uint8 GetCompressedFlags() const override;
	
	uint32 bPressedTeleport:1;
};
