// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Slipstream/Characters/BasePlayerCharacter.h"

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
		FVector End = Start + (HitTarget - Start) * 1.25;
		FHitResult FireHit;
		UWorld* World = GetWorld();
		if (World)
		{
			World->LineTraceSingleByChannel(FireHit, Start, End, ECC_Visibility);
			FVector BeamEnd = End;
			if (FireHit.bBlockingHit)
			{
				BeamEnd = FireHit.ImpactPoint;
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
			if (BeamParticles)
			{
				UParticleSystemComponent* ParticleSystemComponent = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles, SocketTransform);
				if (ParticleSystemComponent)
				{
					ParticleSystemComponent->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}
	}
}