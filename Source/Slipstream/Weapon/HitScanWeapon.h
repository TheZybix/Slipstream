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
	void WeaponTraceHit(const FVector& HitTarget, const FVector& TraceStart, FHitResult& OutHit);
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* HitParticle;
		
	UPROPERTY(EditAnywhere)
	float Damage = 15.f;


private: 
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;
};
