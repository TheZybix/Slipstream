// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupSpawnPoint.h"
#include "WeaponSpawnPoint.generated.h"

class AWeaponBase;
/**
 * 
 */
UCLASS()
class SLIPSTREAM_API AWeaponSpawnPoint : public APickupSpawnPoint
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void SpawnPickup() override;
	virtual void StartSpawnTimer(AActor* DestroyedActor) override;
	
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AWeaponBase>> WeaponArray;

	UPROPERTY()
	AWeaponBase* SpawnedWeapon;
};
