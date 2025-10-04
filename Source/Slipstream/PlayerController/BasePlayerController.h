// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BasePlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

class ASlipstreamGameState;
class ABasePlayerState;
class ABasePlayerHUD;
class UCharacterOverlay;
class ASlipstreamGameMode;
/**
 * 
 */
UCLASS()
class SLIPSTREAM_API ABasePlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDefeat(int Defeat);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDStoredAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetHUDGrenades(int32 Grenades);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	void HideTeamScores();
	void InitTeamScores();
	void SetHUDRedTeamScore(int32 RedScore, int32 MaxScore);
	void SetHUDBlueTeamScore(int32 BlueScore, int32 MaxScore);

	UFUNCTION(Client, Reliable)
	void ClientSetHUDElimination(const FString& EliminationText);
	
	virtual void OnPossess(APawn* InPawn) override;
	void CheckTimeSync(float DeltaTime);
	virtual void Tick(float DeltaTime) override;
	virtual float GetServerTime(); /* Synced with server world clock */

	virtual void ReceivedPlayer() override;

	void OnMatchStateSet(FName Statebool, bool bTeamsMatch = false);
	void HandleMatchHasStarted(bool bTeamsMatch = false);
	void HandleCooldown();

	float SingleTripTime = 0.f;

	FHighPingDelegate HighPingDelegate;
	
protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	void PollInit();
	
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

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float StartingTime, float Cooldown);

	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamScores)
	bool bShowTeamScores = false;

	UFUNCTION()
	void OnRep_ShowTeamScores();

	FString GetInfoText(const TArray<ABasePlayerState*>& Players);
	FString GetTeamsInfoText(ASlipstreamGameState* SlipstreamGameState);

	void HighPingWarning();
	void StopHighPingWarning();
	void CheckPing(float DeltaTime);
	
private:
	UPROPERTY()
	ABasePlayerHUD* PlayerHUD;

	UPROPERTY()
	ASlipstreamGameMode* SlipStreamGameMode;

	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;
	
	bool bInitializeHealth = false;
	bool bInitializeShield = false;
	bool bInitializeScore = false;
	bool bInitializeDefeat = false;
	bool bInitializeGrenades = false;
	bool bInitializeMagAmmo = false;
	bool bInitializeStoredAmmo = false;

	float HUDHealth;
	float HUDMaxHealth;
	float HUDShield;
	float HUDMaxShield;
	float HUDScore;
	int32 HUDDefeat;
	int32 HUDGrenades;
	float HUDStoredAmmo;
	float HUDMagAmmo;
	
	float PingAnimationRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;

	float HighPingRunningTime = CheckPingFrequency;
};