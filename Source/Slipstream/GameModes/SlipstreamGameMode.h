// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Slipstream/PlayerController/BasePlayerController.h"
#include "SlipstreamGameMode.generated.h"
class ABasePlayerController;
class ABasePlayerCharacter;


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
	void ShowEliminationText(FString EliminationMsg);
	virtual void PlayerEliminated(ABasePlayerCharacter* EliminatedPlayer, ABasePlayerController* EliminatedPlayerController, ABasePlayerController* AttackerPlayerController);
	virtual void RequestRespawn(ACharacter* EliminatedPlayer, AController* EliminatedPlayerController);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;

private:
	FTimerHandle EliminationTextTimer;
	float EliminationTextTimerDelay = 5.f;
	void EliminationTimerFinished();
	float CountdownTime = 0.f;
};
