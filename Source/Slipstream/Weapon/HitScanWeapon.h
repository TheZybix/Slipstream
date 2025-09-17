// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "HitScanWeapon.generated.h"


class UParticleSystem;
/**
 * 
 */
UCLASS()
class SLIPSTREAM_API AHitScanWeapon : public AWeaponBase
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;

protected:
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);
	void WeaponTraceHit(const FVector& HitTarget, const FVector& TraceStart, FHitResult& OutHit);
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* HitParticle;

	UPROPERTY(EditAnywhere)
	float Damage = 15.f;

private: 
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;

	/* Trace end with scatter */
	UPROPERTY(EditAnywhere, Category = "Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Scatter")
	float SphereRadius = 800.f;

	UPROPERTY(EditAnywhere, Category = "Scatter")
	bool bUseScatter = false;
};
