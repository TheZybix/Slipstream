// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Slipstream/Characters/BasePlayerCharacter.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	AWeaponBase::Fire(HitTarget);
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("muzzle");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		TMap<ABasePlayerCharacter*, uint32> HitMap;
		for (uint32 i = 0; i < NumberOfPellets; i++)
		{
			FHitResult FireHit;
			WeaponTraceHit(HitTarget, Start, FireHit);
			ABasePlayerCharacter* PlayerCharacter = Cast<ABasePlayerCharacter>(FireHit.GetActor());
			if (PlayerCharacter && HasAuthority() && InstigatorController)
			{
				if (HitMap.Contains(PlayerCharacter))
				{
					HitMap[PlayerCharacter]++;
				}
				else
				{
					HitMap.Emplace(PlayerCharacter, 1);
				}
			}
			if (HitParticle)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
			}
		}

		for (auto HitPair : HitMap)
		{
			if (HitPair.Key && HasAuthority() && InstigatorController)
			{
				UGameplayStatics::ApplyDamage(HitPair.Key, Damage * HitPair.Value, InstigatorController, this, UDamageType::StaticClass());
			}
		}
	}
}
