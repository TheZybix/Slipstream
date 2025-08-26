// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "SlipstreamGameState.generated.h"


class ABasePlayerState;
/**
 * 
 */
UCLASS()
class SLIPSTREAM_API ASlipstreamGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(ABasePlayerState* ScoringPlayer);
	
	UPROPERTY(Replicated)
	TArray<ABasePlayerState*> TopScoringPlayers;


protected:


private:
	float TopScore = 0.f;
};
