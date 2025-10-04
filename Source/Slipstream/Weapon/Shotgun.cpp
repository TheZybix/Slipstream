// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Slipstream/Characters/BasePlayerCharacter.h"
#include "Slipstream/Components/LagCompensationComponent.h"
#include "Slipstream/PlayerController/BasePlayerController.h"
#include "Slipstream/Types/WeaponTypes.h"

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeaponBase::Fire(FVector());

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("muzzle");
	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		TMap<ABasePlayerCharacter*, uint32> HitMap;
		TMap<ABasePlayerCharacter*, uint32> HeadshotHitMap;
		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(HitTarget, Start, FireHit);
			
			ABasePlayerCharacter* PlayerCharacter = Cast<ABasePlayerCharacter>(FireHit.GetActor());
			if (PlayerCharacter)
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

		TArray<ABasePlayerCharacter*> HitCharacters;
		for (auto DamageHitPair : DamageMap)
		{
			if (DamageHitPair.Key && InstigatorController)
			{
				bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
				if (HasAuthority() && bCauseAuthDamage) UGameplayStatics::ApplyDamage(DamageHitPair.Key, DamageHitPair.Value, InstigatorController, this, UDamageType::StaticClass());
				HitCharacters.Add(DamageHitPair.Key);
			}
		}
		
		if (!HasAuthority() && bUseServerSideRewind)
		{
			OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABasePlayerCharacter>(OwnerPawn) : OwnerCharacter;
			OwnerController = OwnerController == nullptr ? Cast<ABasePlayerController>(InstigatorController) : OwnerController;
			if (OwnerCharacter && OwnerCharacter->GetLagCompensation() && OwnerCharacter->IsLocallyControlled() &&OwnerController)
			{
				OwnerCharacter->GetLagCompensation()->ServerShotgunScoreRequest(HitCharacters, Start, HitTargets, OwnerController->GetServerTime() - OwnerController->SingleTripTime);
			}

		}
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("muzzle");
	if (MuzzleFlashSocket == nullptr) return;
	
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		const FVector RandVect = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVect;
		FVector ToEndLoc = EndLoc - TraceStart;
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();
		
		HitTargets.Add(ToEndLoc);
	}
}
