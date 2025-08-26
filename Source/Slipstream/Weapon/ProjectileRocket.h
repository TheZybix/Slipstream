// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class SLIPSTREAM_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();
	
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	UPROPERTY(EditDefaultsOnly)
	float MinDamage = 25.f;

	UPROPERTY(EditDefaultsOnly)
	float InnerRadius = 150.f;

	UPROPERTY(EditDefaultsOnly)
	float OuterRadius = 500.f;


private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMesh;
};
