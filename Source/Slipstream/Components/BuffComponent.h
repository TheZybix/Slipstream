// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"

class ABasePlayerCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SLIPSTREAM_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend class ABasePlayerCharacter;

	void Heal(float HealAmount, float HealTime);
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);

	void BuffJumpVelocity(float BuffJumpVelocity, float BuffTime);
	void SetInitialJumpVelocity(float BaseJumpVelocity);

protected:
	virtual void BeginPlay() override;

	void HealRampUp(float DeltaTime);

private:
	UPROPERTY()
	ABasePlayerCharacter* Character;

	// Health Pickup
	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;

	//Speed Pickup
	FTimerHandle SpeedBuffTimer;
	void ResetSpeeds();
	float InitialBaseSpeed;
	float InitialCrouchSpeed;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);

	//Jump Pickup

	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float InitialJumpVelocity;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpVelocity(float JumpVelocity);

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
