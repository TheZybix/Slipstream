// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponMovementComponent.h"

UProjectileMovementComponent::EHandleBlockingHitResult UWeaponMovementComponent::HandleBlockingHit(
	const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
	return EHandleBlockingHitResult::AdvanceNextSubstep;
}

void UWeaponMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	//Projectiles should not stop
}
