// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000

class AWeaponBase;
class ABasePlayerCharacter;
class ABasePlayerController;
class ABasePlayerHUD;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SLIPSTREAM_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	friend class ABasePlayerCharacter;
	void EquipWeapon(AWeaponBase* WeaponToEquip);
	
protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void TriggerKeyPressed(bool bPressed);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs (FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

private:
	ABasePlayerCharacter* Character;
	ABasePlayerController* PlayerController;
	ABasePlayerHUD* PlayerHUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeaponBase* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bTriggerKeyPressed;

	/* HUD and Crosshairs */
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;

	FVector HitTarget;

public:	

};
