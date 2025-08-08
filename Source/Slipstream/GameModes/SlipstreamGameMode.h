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
	void ShowEliminationText(FString EliminationMsg);
	virtual void PlayerEliminated(ABasePlayerCharacter* EliminatedPlayer, ABasePlayerController* EliminatedPlayerController, ABasePlayerController* AttackerPlayerController);
	virtual void RequestRespawn(ACharacter* EliminatedPlayer, AController* EliminatedPlayerController);

private:
	FTimerHandle EliminationTextTimer;
	float EliminationTextTimerDelay = 5.f;
	void EliminationTimerFinished();
};
