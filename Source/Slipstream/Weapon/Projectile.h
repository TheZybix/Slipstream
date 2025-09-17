// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;
class UParticleSystem;
class UParticleSystemComponent;
class USoundCue;
class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class SLIPSTREAM_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();

protected:
	virtual void BeginPlay() override;

	void StartDestroyTimer();
	virtual void DestroyTimerFinished();
	void ExplodeDamage();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastHit(UParticleSystem* ImpactParticle);

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovement;
	
	UPROPERTY(EditAnywhere)
	float Damage;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticlesEnvironment;
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticlesCharacter;
	
	UPROPERTY(EditAnywhere)
	USoundCue* ImpactSound;

	bool bDestroyImmediately = true;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* TrailSystem;

	UPROPERTY()
	UNiagaraComponent* TrailComponent;

	void SpawnTrailSystem();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(EditDefaultsOnly)
	float MinDamage = 25.f;

	UPROPERTY(EditDefaultsOnly)
	float InnerRadius = 150.f;

	UPROPERTY(EditDefaultsOnly)
	float OuterRadius = 500.f;

private:
	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;

	UPROPERTY()
	UParticleSystemComponent* TracerComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;

public:	

};


