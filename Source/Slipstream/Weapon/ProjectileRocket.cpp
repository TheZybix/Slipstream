// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystemInstanceController.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "WeaponMovementComponent.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>("RocketMesh");
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	WeaponMovementComponent = CreateDefaultSubobject<UWeaponMovementComponent>(FName("WeaponMovementComponent"));
	WeaponMovementComponent->bRotationFollowsVelocity = true;
	WeaponMovementComponent->SetIsReplicated(true);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();
	bDestroyImmediately = false;
	if (TrailSystem)
	{
		TrailComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(TrailSystem, GetRootComponent(), FName(), GetActorLocation(), GetActorRotation(), EAttachLocation::KeepWorldPosition, false);
	}
	if (ProjectileLoop && LoopingSoundAttenuation)
	{
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(ProjectileLoop, GetRootComponent(), FName(), GetActorLocation(), EAttachLocation::KeepWorldPosition, false, 1.f, 1.f, 0.f, LoopingSoundAttenuation, (USoundConcurrency*)nullptr, false);
	}
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner())
	{
		return;
	}
	
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority())
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(this, Damage, MinDamage, GetActorLocation(), InnerRadius, OuterRadius,
															1.f, UDamageType::StaticClass(), TArray<AActor*>(), this, FiringController);
		}
	}
	
	// DRAW DEBUG SPHERES, DISABLE PROJECTILE DESTROY TO VISUALIZE DAMAGE RADIUS
	/* DrawDebugSphere(GetWorld(), GetActorLocation(), InnerRadius, 8, FColor::Red, false, 10.f);
	DrawDebugSphere(GetWorld(), GetActorLocation(), OuterRadius, 8, FColor::Blue, false, 10.f); */

	GetWorldTimerManager().SetTimer(DestroyTimer, this, &AProjectileRocket::DestroyTimerFinished, DestroyTime, false);
	if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}

	
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileRocket::MulticastHit_Implementation(UParticleSystem* ImpactParticle)
{
	Super::MulticastHit_Implementation(ImpactParticle);
	if (RocketMesh) RocketMesh->SetVisibility(false);
	if (CollisionBox) CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (TrailComponent && TrailComponent->GetSystemInstanceController()) TrailComponent->GetSystemInstanceController()->Deactivate();
}

void AProjectileRocket::DestroyTimerFinished()
{
	Destroy();
}
