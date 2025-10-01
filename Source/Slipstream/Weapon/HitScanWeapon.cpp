// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Slipstream/Characters/BasePlayerCharacter.h"
#include "Slipstream/Types/WeaponTypes.h"
#include "DrawDebugHelpers.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("muzzle");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FHitResult FireHit;
		WeaponTraceHit(HitTarget, Start, FireHit);
		ABasePlayerCharacter* PlayerCharacter = Cast<ABasePlayerCharacter>(FireHit.GetActor());
		if (PlayerCharacter && HasAuthority() && InstigatorController)
		{
			float DamageToCause = Damage;
			if (FireHit.BoneName.ToString() == FString("head"))
			{
				DamageToCause = HeadshotDamage;
			}
			UGameplayStatics::ApplyDamage(PlayerCharacter, DamageToCause, InstigatorController, this, UDamageType::StaticClass());
		}
		if (HitParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
		}
	}
}

void AHitScanWeapon::WeaponTraceHit(const FVector& HitTarget, const FVector& TraceStart, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FVector TraceEnd = TraceStart + (HitTarget - TraceStart) * 1.25f;
		World->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_Visibility);
		FVector BeamEnd = TraceEnd;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		else
		{
			OutHit.ImpactPoint = TraceEnd;
		}

		DrawDebugSphere(World, BeamEnd, 15.f, 12, FColor::Red, false, 20.f);
		
		if (BeamParticles)
		{
			UParticleSystemComponent* ParticleSystemComponent = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles, TraceStart, FRotator::ZeroRotator, true);
			if (ParticleSystemComponent)
			{
				ParticleSystemComponent->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}
