// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * Movement component meant for use with Pawns.
 */

#pragma once
#include "ShooterCharacterMovement.generated.h"

class UShooterSavedMove_Character;
class UShooterCharacterMovement;

enum ECustomMovementMode
{
	CMM_JETPACK = 0
};

UCLASS()
class UShooterCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()
	public:
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	
	//Teleport distance in centimeters, default is 1000 (10 meters)
	UPROPERTY(EditAnywhere, Category = "Teleport")
	float TeleportDistance = 1000.f;
	
	UPROPERTY(BlueprintReadWrite, Category= "Jetpack")
	float JetpackMaxFuel = 1.0;
	
	UPROPERTY(BlueprintReadWrite, Category= "Jetpack")
	float JetpackCurrentFuel;

	UPROPERTY(BlueprintReadWrite, Category= "Jetpack")
	float JetpackForce = 5000;

	virtual float GetMaxSpeed() const override;	

	bool CanJetpack();
	
	//Performs actual teleport
	void DoTeleport();
	
	void SetTeleport(bool IsPressingTeleport);
	void SetJetpackState(bool JetpackIsActive);
	void PerformJetpackStateChange(bool JetpackIsActive);

	
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void ServerSetJetpackState(bool JetpackState);
	UFUNCTION(Client, Reliable, BlueprintCallable)
		void ClientSetJetpackState(bool JetpackState);

	/* True if teleport button is being pressed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Teleport")
	bool bIsPressingTeleport;

	/* True if Jetpack is firing */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Teleport")
	bool bJetpackIsActive;
	
protected:
	//Overrides
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual bool IsFalling() const override;
	
	void PhysJetpacking(float deltaTime, int32 Iterations);
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
	uint32 bJetpackActivated:1;
};

class FNetworkPredictionData_Client_SCMovement : public FNetworkPredictionData_Client_Character
{
public:
	FNetworkPredictionData_Client_SCMovement(const UCharacterMovementComponent& ClientMovement);

	typedef FNetworkPredictionData_Client_Character Super;

	///@brief Allocates a new copy of our custom saved move
	virtual FSavedMovePtr AllocateNewMove() override;
};