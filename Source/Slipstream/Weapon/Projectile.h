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

UCLASS()
class SLIPSTREAM_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastHit();

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;

	UParticleSystemComponent* TracerComponent;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticlesEnvironment;
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticlesCharacter;

	UPROPERTY()
	UParticleSystem* ImpactParticle;

	UPROPERTY(EditAnywhere)
	USoundCue* ImpactSound;
	
public:	
	virtual void Tick(float DeltaTime) override;

};


