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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const override;

	virtual void OnRep_Score() override;

	UFUNCTION()
	virtual void OnRep_Defeat();
	
	void AddToScore(float ScoreAmount);
	void AddToDefeat(int32 DefeatAmount);


private:
	UPROPERTY()
	ABasePlayerCharacter* PlayerCharacter;
	UPROPERTY()
	ABasePlayerController* PlayerController;

	float NewScore;

	UPROPERTY(ReplicatedUsing = OnRep_Defeat)
	int32 Defeat;
};
