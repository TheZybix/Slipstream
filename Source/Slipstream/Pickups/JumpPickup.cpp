// Fill out your copyright notice in the Description page of Project Settings.


#include "JumpPickup.h"
#include "Slipstream/Characters/BasePlayerCharacter.h"
#include "Slipstream/Components/BuffComponent.h"

void AJumpPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABasePlayerCharacter* PlayerCharacter = Cast<ABasePlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		UBuffComponent* Buff = PlayerCharacter->GetBuff();
		if (Buff)
		{
			Buff->BuffJumpVelocity(JumpVelocityBuff, JumpBuffTime);
		}
	}
	Destroy();
}
