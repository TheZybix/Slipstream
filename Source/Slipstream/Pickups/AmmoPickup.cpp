// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"

#include "Slipstream/Characters/BasePlayerCharacter.h"
#include "Slipstream/Components/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABasePlayerCharacter* PlayerCharacter = Cast<ABasePlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		UCombatComponent* Combat = PlayerCharacter->GetCombat();
		if (Combat)
		{
			Combat->PickupAmmo(AmmoWeapons);
		}
	}
	Destroy();
}
