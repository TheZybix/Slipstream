// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BasePlayerController.generated.h"

class ABasePlayerHUD;
/**
 * 
 */
UCLASS()
class SLIPSTREAM_API ABasePlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeat(int Defeat);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDStoredAmmo(int32 Ammo);

	UFUNCTION(Client, Reliable)
	void ClientSetHUDElimination(const FString& EliminationText);
	
	virtual void OnPossess(APawn* InPawn) override;
	
protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY()
	ABasePlayerHUD* PlayerHUD;
};
