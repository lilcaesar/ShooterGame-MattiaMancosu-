// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShooterPickup_Ammo.h"
#include "ShooterPickup_WeaponWithAmmo.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class SHOOTERGAME_API AShooterPickup_WeaponWithAmmo : public AShooterPickup_Ammo
{
	GENERATED_BODY()
public:
	AShooterPickup_WeaponWithAmmo(const FObjectInitializer& ObjectInitializer);
	AShooterPickup_WeaponWithAmmo(const FObjectInitializer& ObjectInitializer, int32 Clips, int32 RemainingAmmo);
	
	virtual void Tick(float DeltaTime) override;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category=Pickup)
	int32 RemainingAmmoInClip;
	
	UPROPERTY(EditDefaultsOnly, Category=Pickup)
	float SelfDestructTimer = 10;
	
	/** give pickup */
	virtual void GivePickupTo(AShooterCharacter* Pawn) override;
};
