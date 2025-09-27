// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SlipstreamGameMode.h"
#include "TeamDeathMatchGameMode.generated.h"

/**
 * 
 */
UCLASS()
class SLIPSTREAM_API ATeamDeathMatchGameMode : public ASlipstreamGameMode
{
	GENERATED_BODY()

public:
	ATeamDeathMatchGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float Damage) override;
	virtual void PlayerEliminated(ABasePlayerCharacter* EliminatedPlayer, ABasePlayerController* EliminatedPlayerController, ABasePlayerController* AttackerPlayerController) override;

protected:
	virtual void HandleMatchHasStarted() override;
};
