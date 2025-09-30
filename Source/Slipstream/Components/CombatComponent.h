// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Slipstream/HUD/BasePlayerHUD.h"
#include "Slipstream/Types/CombatState.h"
#include "CombatComponent.generated.h"

enum class ECombatState : uint8;
enum class EWeaponType : uint8;
class AWeaponBase;
class ABasePlayerCharacter;
class ABasePlayerController;
class ABasePlayerHUD;
class AProjectile;

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
	void SwapWeapon();
	void Reload();

	UFUNCTION(BlueprintCallable)
	void SetCombatState(ECombatState NewCombatState);

	void TriggerKeyPressed(bool bPressed);

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenadeFinished(const FVector_NetQuantize& Target);

	void JumpToShotgunEnd();

	void PickupAmmo(TMap<EWeaponType, int32> Ammo);

	void SpawnDefaultWeapon();

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();
	
	void Fire();
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs (FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();
	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();

	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> GrenadeClass;

	void DropEquippedWeapon();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void AttachActorToBack(AActor* ActorToAttach);
	void PlayEquipWeaponSound(AWeaponBase* WeaponToEquip);

	void EquipPrimaryWeapon(AWeaponBase* WeaponToEquip);
	void EquipSecondaryWeapon(AWeaponBase* WeaponToEquip);


private:
	ABasePlayerCharacter* Character;
	ABasePlayerController* PlayerController;
	ABasePlayerHUD* PlayerHUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeaponBase* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeaponBase* SecondaryWeapon;

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
	float CrosshairAimFactor;
	float CrosshairShootingFactor;
	float CrosshairTargetFactor;
	bool bIsTargeting;
	
	FHUDPackage HUDPackage;
	FVector HitTarget;

	/* Aiming and FOV */
	float DefaultFOV; //FOV when not aiming
	float CurrentFOV;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedInterpSpeed = 20.f;

	void InterpolateFOV(float DeltaTime);

	/* Automatic fire */
	FTimerHandle FireTimer;
	bool CanFire(); 
	
	bool bCanFire = true;

	void StartFireTimer();
	void FireTimerFinished();

	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	int32 AmountToReload();

	void ShowAttachedGrenade(bool bShowGrenade);

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 0;

	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 4;

	UFUNCTION()
	void OnRep_Grenades();

	void UpdateHUDGrenades();

	/* Default Weapon */
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeaponBase> DefaultWeapon;

public:
	FORCEINLINE ECombatState GetCombatState() const { return CombatState; }
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
	bool ShouldSwapWeapons();
};