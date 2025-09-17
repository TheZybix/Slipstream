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
			UGameplayStatics::ApplyDamage(PlayerCharacter, Damage, InstigatorController, this, UDamageType::StaticClass());
		}
		if (HitParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
		}
	}
}

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	FVector RandVect = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	FVector EndLoc = SphereCenter + RandVect;
	FVector ToEndLoc = EndLoc - TraceStart;

	/* DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Yellow, true);
	DrawDebugSphere(GetWorld(), EndLoc, 15.f, 6, FColor::Green, true);
	DrawDebugLine(GetWorld(), TraceStart, TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size(), FColor::Black, true); */
	
	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}

void AHitScanWeapon::WeaponTraceHit(const FVector& HitTarget, const FVector& TraceStart, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FVector TraceEnd = bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f;
		World->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_Visibility);
		FVector BeamEnd = TraceEnd;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
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
