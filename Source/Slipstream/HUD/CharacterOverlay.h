// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

class UImage;
class UProgressBar;
class UTextBlock;
/**
 * 
 */
UCLASS()
class SLIPSTREAM_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmmoAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StoredAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountdownTimer;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GrenadeText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* BlueTeamText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RedTeamText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* BlueTeamBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* RedTeamBar;

	UPROPERTY(meta = (BindWidget))
	UImage* HighPingImage;

	UPROPERTY(meta =(BindWidgetAnim), Transient)
	UWidgetAnimation* HighPingAnimation;
};
