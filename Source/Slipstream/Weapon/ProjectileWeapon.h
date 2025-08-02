// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "ProjectileWeapon.generated.h"

class AProjectile;
/**
 * 
 */
UCLASS()
class SLIPSTREAM_API AProjectileWeapon : public AWeaponBase
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;
protected:
private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> ProjectileClass;
};
