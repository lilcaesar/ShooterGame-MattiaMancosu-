// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterPickup_WeaponWithAmmo.h"
#include "Weapons/ShooterWeapon.h"
#include "OnlineSubsystemUtils.h"

AShooterPickup_WeaponWithAmmo::AShooterPickup_WeaponWithAmmo(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	AmmoClips = 0;
	RemainingAmmoInClip = 0;
}

AShooterPickup_WeaponWithAmmo::AShooterPickup_WeaponWithAmmo(const FObjectInitializer& ObjectInitializer, int32 Clips, int32 RemainingAmmo) : Super(ObjectInitializer), RemainingAmmoInClip(RemainingAmmo)
{
	AmmoClips = Clips;
}

void AShooterPickup_WeaponWithAmmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SelfDestructTimer -= DeltaTime;
	if(SelfDestructTimer<0)
	{
		Destroy();
	}
}

void AShooterPickup_WeaponWithAmmo::GivePickupTo(AShooterCharacter* Pawn)
{
	AShooterWeapon* Weapon = (Pawn ? Pawn->FindWeapon(WeaponType) : NULL);
	if (Weapon)
	{
		//Adding RemainingAmmoInClip to the original implementation
		int32 Qty = AmmoClips * Weapon->GetAmmoPerClip() + RemainingAmmoInClip;
		Weapon->GiveAmmo(Qty);

		// Fire event for collected ammo
		if (Pawn)
		{
			const UWorld* World = GetWorld();
			const IOnlineEventsPtr Events = Online::GetEventsInterface(World);
			const IOnlineIdentityPtr Identity = Online::GetIdentityInterface(World);

			if (Events.IsValid() && Identity.IsValid())
			{							
				AShooterPlayerController* PC = Cast<AShooterPlayerController>(Pawn->Controller);
				if (PC)
				{
					ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PC->Player);

					if (LocalPlayer)
					{
						const int32 UserIndex = LocalPlayer->GetControllerId();
						TSharedPtr<const FUniqueNetId> UniqueID = Identity->GetUniquePlayerId(UserIndex);			
						if (UniqueID.IsValid())
						{
							FVector Location = Pawn->GetActorLocation();

							FOnlineEventParms Params;		

							Params.Add( TEXT( "SectionId" ), FVariantData( (int32)0 ) ); // unused
							Params.Add( TEXT( "GameplayModeId" ), FVariantData( (int32)1 ) ); // @todo determine game mode (ffa v tdm)
							Params.Add( TEXT( "DifficultyLevelId" ), FVariantData( (int32)0 ) ); // unused

							Params.Add( TEXT( "ItemId" ), FVariantData( (int32)Weapon->GetAmmoType() + 1 ) ); // @todo come up with a better way to determine item id, currently health is 0 and ammo counts from 1
							Params.Add( TEXT( "AcquisitionMethodId" ), FVariantData( (int32)0 ) ); // unused
							Params.Add( TEXT( "LocationX" ), FVariantData( Location.X ) );
							Params.Add( TEXT( "LocationY" ), FVariantData( Location.Y ) );
							Params.Add( TEXT( "LocationZ" ), FVariantData( Location.Z ) );
							Params.Add( TEXT( "ItemQty" ), FVariantData( (int32)Qty ) );		

							Events->TriggerEvent(*UniqueID, TEXT("CollectPowerup"), Params);
						}
					}
				}
			}
		}
	}
	//Once picked it cannot respawn and should be destroyed
	Destroy();
}
