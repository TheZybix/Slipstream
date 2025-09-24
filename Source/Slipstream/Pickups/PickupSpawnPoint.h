// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

class APickup;

UCLASS()
class SLIPSTREAM_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickupSpawnPoint();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<APickup>> PickupClasses;

	UPROPERTY()
	APickup* SpawnedPickup;

	virtual void SpawnPickup();
	void SpawnTimerFinished();

	UFUNCTION()
	virtual void StartSpawnTimer(AActor* DestroyedActor);

	FTimerHandle SpawnTimer;

	UPROPERTY(EditAnywhere)
	float SpawnPickupTime;

private:


public:	
	virtual void Tick(float DeltaTime) override;
};
