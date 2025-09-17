// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileGrenade.generated.h"

class UProjectileMovementComponent;
/**
 * 
 */
UCLASS()
class SLIPSTREAM_API AProjectileGrenade : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileGrenade();
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;
	virtual void DestroyTimerFinished() override;

	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* MovementComponent;

private:
	UPROPERTY(EditAnywhere)
	USoundBase* BounceSound;
};
