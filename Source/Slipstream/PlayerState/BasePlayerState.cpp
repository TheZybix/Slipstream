// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlayerState.h"

#include "Slipstream/Characters/BasePlayerCharacter.h"
#include "Slipstream/PlayerController/BasePlayerController.h"

void ABasePlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	PlayerCharacter = PlayerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetPawn()) : PlayerCharacter;
	if (PlayerCharacter)
	{
		PlayerController = PlayerController == nullptr ? Cast<ABasePlayerController>(PlayerCharacter->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDScore(NewScore);
		}
	}
}

void ABasePlayerState::AddToScore(float ScoreAmount)
{
	NewScore = GetScore() + ScoreAmount;
	SetScore(NewScore);
	
	PlayerCharacter = PlayerCharacter == nullptr ? Cast<ABasePlayerCharacter>(GetPawn()) : PlayerCharacter;
	if (PlayerCharacter)
	{
		PlayerController = PlayerController == nullptr ? Cast<ABasePlayerController>(PlayerCharacter->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDScore(NewScore);
		}
	}
}
