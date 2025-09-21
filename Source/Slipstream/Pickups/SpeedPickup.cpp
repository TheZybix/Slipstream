// Fill out your copyright notice in the Description page of Project Settings.


#include "SpeedPickup.h"

#include "Slipstream/Characters/BasePlayerCharacter.h"
#include "Slipstream/Components/BuffComponent.h"

class UBuffComponent;
class ABasePlayerCharacter;

void ASpeedPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABasePlayerCharacter* PlayerCharacter = Cast<ABasePlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		UBuffComponent* Buff = PlayerCharacter->GetBuff();
		if (Buff)
		{
			Buff->BuffSpeed(BaseSpeedBuff, CrouchSpeedBuff, SpeedBuffTime);
		}
	}
	Destroy();
}
