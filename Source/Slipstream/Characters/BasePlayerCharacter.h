// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "BasePlayerCharacter.generated.h"

class UCombatComponent;
class AWeaponBase;
class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;

UCLASS()
class SLIPSTREAM_API ABasePlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABasePlayerCharacter();
	void InitializeMappingContext();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

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
	
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> CameraComponent;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Jump(const FInputActionValue& Value);
	void EquipKeyPressed(const FInputActionValue& Value);
	void Crouch(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeaponBase* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeaponBase* LastWeapon);

	UPROPERTY(VisibleAnywhere, Category = "Combat")
	UCombatComponent* CombatComponent;

	UFUNCTION(Server, Reliable)
	void ServerEquipKeyPressed();
	
public:
	void SetOverlappingWeapon(AWeaponBase* Weapon);
	bool IsWeaponEquipped();

};
