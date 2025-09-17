// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Slipstream/Types/TurningInPlace.h"
#include "Slipstream/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "BasePlayerCharacter.generated.h"

enum class ECombatState : uint8;
class UCombatComponent;
class AWeaponBase;
class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
class ABasePlayerController;
class ABasePlayerState;

UCLASS()
class SLIPSTREAM_API ABasePlayerCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABasePlayerCharacter();
	void InitializeMappingContext();

	virtual void Destroyed() override;
	virtual void Restart() override;

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_ReplicatedMovement() override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayDeathMontage();

	void Elim();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	UPROPERTY()
	ABasePlayerState* BasePlayerState;

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void RotateInPlace(float DeltaTime);

	void AimOffset(float DeltaTime);
	void SimProxiesTurn();
	virtual void Jump() override;
	void PlayHitReactMontage();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);
	void UpdateHUDHealth();
	
	/* Poll for any relevant classes and initialize HUD */
	void PollInit();

	void InitializeMaterials();

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputMappingContext> InputMappingContext;
	
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> MovementAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> EquipAction;
	
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> TriggerAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> ReloadAction;
	
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> CameraComponent;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void JumpKeyPressed(const FInputActionValue& Value);
	void EquipKeyPressed(const FInputActionValue& Value);
	void CrouchKeyPressed(const FInputActionValue& Value);
	void AimKeyPressed(const FInputActionValue& Value);
	void TriggerKeyPressed(const FInputActionValue& Value);
	void ReloadKeyPressed(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeaponBase* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeaponBase* LastWeapon);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Combat")
	UCombatComponent* CombatComponent;

	UFUNCTION(Server, Reliable)
	void ServerEquipKeyPressed();

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	/* Animaton Montages */

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* AssaultRifleMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* RocketLauncherMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* ShotgunMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* DeathMontage;

	void HideCameraIfCharacterClose();
	void CalculateAO_Pitch();

	UPROPERTY(EditAnywhere, Category = "Camera")
	float HideCameraThreshold = 20.f;

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();

	/* Player health */
	UPROPERTY(EditAnywhere, Category = "PlayerStats")
	float MaxHealth = 100.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "PlayerStats")
	float Health = 100.f;

	UPROPERTY()
	ABasePlayerController* PlayerController;

	UFUNCTION()
	void OnRep_Health();

	bool bIsDead = false;

	FTimerHandle ElimTimer;
	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;
	void ElimTimerFinished();

	/* DissolveEffect */
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();
	
	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UPROPERTY(VisibleAnywhere, Category = "Dissolve")
	UMaterialInstanceDynamic* DynamicDissolveHeadSkin;

	UPROPERTY(VisibleAnywhere, Category = "Dissolve")
	UMaterialInstanceDynamic* DynamicDissolveGloves;

	UPROPERTY(VisibleAnywhere, Category = "Dissolve")
	UMaterialInstanceDynamic* DynamicDissolveBodySkin;

	UPROPERTY(VisibleAnywhere, Category = "Dissolve")
	UMaterialInstanceDynamic* DynamicDissolveBikini;

	UPROPERTY(VisibleAnywhere, Category = "Dissolve")
	UMaterialInstanceDynamic* DynamicDissolveShirts;

	UPROPERTY(VisibleAnywhere, Category = "Dissolve")
	UMaterialInstanceDynamic* DynamicDissolvePants;

	UPROPERTY(VisibleAnywhere, Category = "Dissolve")
	UMaterialInstanceDynamic* DynamicDissolveShoes;
	
	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> DynamicDissolveMaterials;
	

	UPROPERTY(EditAnywhere, Category = "Dissolve")
	UMaterialInstance* DissolveHeadSkin;

	UPROPERTY(EditAnywhere, Category = "Dissolve")
	UMaterialInstance* DissolveGloves;

	UPROPERTY(EditAnywhere, Category = "Dissolve")
	UMaterialInstance* DissolveBodySkin;

	UPROPERTY(EditAnywhere, Category = "Dissolve")
	UMaterialInstance* DissolveBikini;

	UPROPERTY(EditAnywhere, Category = "Dissolve")
	UMaterialInstance* DissolveShirts;

	UPROPERTY(EditAnywhere, Category = "Dissolve")
	UMaterialInstance* DissolvePants;

	UPROPERTY(EditAnywhere, Category = "Dissolve")
	UMaterialInstance* DissolveShoes;

	UPROPERTY()
	TArray<UMaterialInstance*> DissolveMaterials;

	UPROPERTY()
	TArray<FName> MaterialSlots;

	/* Elimimination effect */
	UPROPERTY(EditAnywhere)
	UParticleSystem* EliminationBotParticles;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* EliminationBotParticlesComponent;

	UPROPERTY(EditAnywhere)
	USoundBase* EliminationBotSound;

	bool bIsMappingContextAdded = false;
	
public:
	void SetOverlappingWeapon(AWeaponBase* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();

	FORCEINLINE float GetAOYaw() const { return AO_Yaw; }
	FORCEINLINE float GetAOPitch() const { return AO_Pitch; }
	AWeaponBase* GetEquippedWeapon();

	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FVector GetHitTarget() const;

	FORCEINLINE UCameraComponent* GetCameraComponent() const { return CameraComponent; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsDead() const { return bIsDead; }

	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetCombat() const { return CombatComponent; }

	FORCEINLINE bool CheckDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE UAnimMontage* GetShotgunMontage() const { return ShotgunMontage; }
};





