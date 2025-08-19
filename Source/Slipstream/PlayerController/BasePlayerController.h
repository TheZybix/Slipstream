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
	void SetHUDMatchCountdown(float CountdownTime);

	UFUNCTION(Client, Reliable)
	void ClientSetHUDElimination(const FString& EliminationText);
	
	virtual void OnPossess(APawn* InPawn) override;
	void CheckTimeSync(float DeltaTime);
	virtual void Tick(float DeltaTime) override;
	virtual float GetServerTime(); /* Synced with server world clock */

	virtual void ReceivedPlayer() override;
	
protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	
	/* Sync time between client and server */

	/* Requests current server time, passing in the time the client sent the request */
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	/* Reports the current server time to client in response to ServerRequestServerTime */
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeSeverReceivedClientRequest);

	float ClientServerDelta = 0.f; /* Difference between Client and Server Time */

	UPROPERTY(EditAnywhere, Category = "Time")
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	
private:
	UPROPERTY()
	ABasePlayerHUD* PlayerHUD;

	float MatchTime = 120.f;
	uint32 CountdownInt = 0;
};