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
	enum class EGunType
	{
		ERifle,
		ERocketLauncher,
	};
	
	AShooterPickup_WeaponWithAmmo(const FObjectInitializer& ObjectInitializer);
	void InitialisePickupParameters(int32 RemainingAmmo, EGunType GunType, AShooterWeapon* HeldWeapon);
	virtual void OnConstruction(const FTransform& Transform) override;
	/** Called after SelfDestructTimer seconds */
	void SelfDestroy();
	
	/** check if pawn can use this pickup */
	virtual bool CanBePickedUp(AShooterCharacter* TestPawn) const override;
	
	UPROPERTY(EditDefaultsOnly, Category=Pickup)
	int32 RemainingAmmoInClip;
	
	/** FX component for Rifle and RocketLauncher*/
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	UParticleSystem* RiflePSC;
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	UParticleSystem* RocketLauncherPSC;
	
	UPROPERTY(EditDefaultsOnly, Category=Pickup)
	float SelfDestructTimer = 10;
	
	/** Handle of SelfDestroy timer */
	FTimerHandle TimerHandle_SelfDestroy;

protected:	
	/** give pickup */
	virtual void GivePickupTo(AShooterCharacter* Pawn) override;
};
