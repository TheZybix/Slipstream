// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupSpawnPoint.h"

#include "Pickup.h"


APickupSpawnPoint::APickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

}

void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	StartSpawnTimer((AActor*)nullptr);
}

void APickupSpawnPoint::SpawnPickup()
{
	int32 NumOfPickups = PickupClasses.Num();
	if (NumOfPickups > 0)
	{
		int32 Selection = FMath::RandRange(0, NumOfPickups - 1);
		SpawnedPickup = GetWorld()->SpawnActor<APickup>(PickupClasses[Selection], GetActorTransform());

		if (HasAuthority() && SpawnedPickup)
		{
			SpawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnTimer);
		}
	}
}

void APickupSpawnPoint::SpawnTimerFinished()
{
	if (HasAuthority()) SpawnPickup();
}

void APickupSpawnPoint::StartSpawnTimer(AActor* DestroyedActor)
{
	GetWorldTimerManager().SetTimer(SpawnTimer, this, &APickupSpawnPoint::SpawnTimerFinished, SpawnPickupTime);
}

void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

