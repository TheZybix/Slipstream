// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponSpawnPoint.h"
#include "Slipstream/Weapon/WeaponBase.h"

void AWeaponSpawnPoint::BeginPlay()
{
	AActor::BeginPlay();
	if (HasAuthority()) SpawnPickup();
}

void AWeaponSpawnPoint::SpawnPickup()
{
	int32 NumOfWeapons = WeaponArray.Num();
	if (NumOfWeapons > 0)
	{
		int32 Selection = FMath::RandRange(0, NumOfWeapons - 1);
		SpawnedWeapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponArray[Selection], GetActorTransform());

		if (HasAuthority() && SpawnedWeapon)
		{
			SpawnedWeapon->WeaponEquipped.AddDynamic(this, &AWeaponSpawnPoint::StartSpawnTimer);
		}
	}
}

void AWeaponSpawnPoint::StartSpawnTimer(AActor* DestroyedActor)
{
	Super::StartSpawnTimer(DestroyedActor);
	
	AWeaponBase* EquippedWeapon = Cast<AWeaponBase>(DestroyedActor);
	if (EquippedWeapon)
	{
		EquippedWeapon->WeaponEquipped.RemoveAll(this);
	}
}