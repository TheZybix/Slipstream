// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletSleeve.generated.h"

class USkeletalMeshComponent;
class USoundBase;

UCLASS()
class SLIPSTREAM_API ABulletSleeve : public AActor
{
	GENERATED_BODY()
	
public:	
	ABulletSleeve();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* SleeveMesh;

	UPROPERTY(EditAnywhere)
	float EjectionImpulse;

	UPROPERTY(EditAnywhere)
	USoundBase* ShellSound;
};
