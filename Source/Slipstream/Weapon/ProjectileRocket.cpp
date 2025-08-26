// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>("RocketMesh");
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn)
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
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
