// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"


class UNiagaraSystem;
class UNiagaraComponent;
class UWeaponMovementComponent;
/**
 * 
 */
UCLASS()
class SLIPSTREAM_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();
	virtual void BeginPlay() override;
	
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void MulticastHit_Implementation(UParticleSystem* ImpactParticle) override;

	UPROPERTY(EditDefaultsOnly)
	float MinDamage = 25.f;

	UPROPERTY(EditDefaultsOnly)
	float InnerRadius = 150.f;

	UPROPERTY(EditDefaultsOnly)
	float OuterRadius = 500.f;
	
	UPROPERTY(EditAnywhere)
	UNiagaraSystem* TrailSystem;

	UNiagaraComponent* TrailComponent;

	UPROPERTY(EditAnywhere)
	USoundBase* ProjectileLoop;

	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;

	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;
	
	void DestroyTimerFinished();

	UPROPERTY(VisibleAnywhere)
	UWeaponMovementComponent* WeaponMovementComponent;

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMesh;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;
};
