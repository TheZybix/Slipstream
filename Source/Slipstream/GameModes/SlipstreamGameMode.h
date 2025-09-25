// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Slipstream/PlayerController/BasePlayerController.h"
#include "SlipstreamGameMode.generated.h"

namespace MatchState
{
	extern SLIPSTREAM_API const FName Cooldown; //Match duration as been reached, display endgame information
}

class ABasePlayerController;
class ABasePlayerCharacter;
class ABasePlayerState;

/**
 * 
 */
UCLASS()
class SLIPSTREAM_API ASlipstreamGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ASlipstreamGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(ABasePlayerCharacter* EliminatedPlayer, ABasePlayerController* EliminatedPlayerController, ABasePlayerController* AttackerPlayerController);
	virtual void RequestRespawn(ACharacter* EliminatedPlayer, AController* EliminatedPlayerController);
	void PlayerLeftGame(ABasePlayerState* PlayerState);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;

	void CreateElimMessage(ABasePlayerState* AttackerState, ABasePlayerState* EliminatedState);

	UPROPERTY(EditDefaultsOnly)
	TArray<FString> KillMessages;

	FString KillMessage;

public:
	FORCEINLINE float GetCountdownTime() { return CountdownTime; }
};
