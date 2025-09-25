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
		TMap<ABasePlayerCharacter*, uint32> HeadshotHitMap;
		for (uint32 i = 0; i < NumberOfPellets; i++)
		{
			FHitResult FireHit;
			WeaponTraceHit(HitTarget, Start, FireHit);
			ABasePlayerCharacter* PlayerCharacter = Cast<ABasePlayerCharacter>(FireHit.GetActor());
			if (PlayerCharacter && HasAuthority() && InstigatorController)
			{
				const bool bHeadshot = FireHit.BoneName.ToString() == FString("head");
				if (bHeadshot)
				{
					if (HeadshotHitMap.Contains(PlayerCharacter)) HeadshotHitMap[PlayerCharacter]++;
					else HeadshotHitMap.Emplace(PlayerCharacter, 1);
				}
				else
				{
					if (HitMap.Contains(PlayerCharacter)) HitMap[PlayerCharacter]++;
					else HitMap.Emplace(PlayerCharacter, 1);
				}
				
			}
			if (HitParticle)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
			}
		}
		TMap<ABasePlayerCharacter*, float> DamageMap;
		
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key && HasAuthority())
			{
				DamageMap.Emplace(HitPair.Key, HitPair.Value * Damage);
			}
		}
		for (auto HeadshotHitPair : HeadshotHitMap)
		{
			if (HeadshotHitPair.Key && HasAuthority())
			{
				if (DamageMap.Contains(HeadshotHitPair.Key)) DamageMap[HeadshotHitPair.Key] += HeadshotHitPair.Value * HeadshotDamage;
				else HitMap.Emplace(HeadshotHitPair.Key,  HeadshotHitPair.Value * HeadshotDamage);
			}
		}
		for (auto DamageHitPair : DamageMap)
		{
			if (DamageHitPair.Key && HasAuthority())
			{
				UGameplayStatics::ApplyDamage(DamageHitPair.Key, DamageHitPair.Value, InstigatorController, this, UDamageType::StaticClass());
			}
		}
		
	}
}
