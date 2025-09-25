// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileBullet.generated.h"


class UWeaponMovementComponent;
/**
 * 
 */
UCLASS()
class SLIPSTREAM_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileBullet();
	
	UPROPERTY()
	float HeadshotDamage = 15.f;


protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	UPROPERTY(VisibleAnywhere)
	UWeaponMovementComponent* WeaponMovementComponent;
};