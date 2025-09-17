// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraSystemInstanceController.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "WeaponMovementComponent.h"

AProjectileRocket::AProjectileRocket()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>("RocketMesh");
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	WeaponMovementComponent = CreateDefaultSubobject<UWeaponMovementComponent>(FName("WeaponMovementComponent"));
	WeaponMovementComponent->bRotationFollowsVelocity = true;
	WeaponMovementComponent->SetIsReplicated(true);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();
	bDestroyImmediately = false;
	
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
	
	ExplodeDamage();
	
	// DRAW DEBUG SPHERES, DISABLE PROJECTILE DESTROY TO VISUALIZE DAMAGE RADIUS
	/* DrawDebugSphere(GetWorld(), GetActorLocation(), InnerRadius, 8, FColor::Red, false, 10.f);
	DrawDebugSphere(GetWorld(), GetActorLocation(), OuterRadius, 8, FColor::Blue, false, 10.f); */

	StartDestroyTimer();
	if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}

	
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileRocket::MulticastHit_Implementation(UParticleSystem* ImpactParticle)
{
	Super::MulticastHit_Implementation(ImpactParticle);
	if (ProjectileMesh) ProjectileMesh->SetVisibility(false);
	if (CollisionBox) CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (TrailComponent && TrailComponent->GetSystemInstanceController()) TrailComponent->GetSystemInstanceController()->Deactivate();
}
