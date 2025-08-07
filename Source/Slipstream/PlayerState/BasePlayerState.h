// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BasePlayerState.generated.h"

class ABasePlayerController;
class ABasePlayerCharacter;
/**
 * 
 */
UCLASS()
class SLIPSTREAM_API ABasePlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void OnRep_Score() override;
	void AddToScore(float ScoreAmount);

private:
	ABasePlayerCharacter* PlayerCharacter;
	ABasePlayerController* PlayerController;

	float NewScore;
};
