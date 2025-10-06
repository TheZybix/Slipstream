// Fill out your copyright notice in the Description page of Project Settings.


#include "SlipstreamGameState.h"
#include "Net/UnrealNetwork.h"
#include "Slipstream/PlayerState/BasePlayerState.h"
#include "Slipstream/PlayerController/BasePlayerController.h"

void ASlipstreamGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASlipstreamGameState, TopScoringPlayers);
	DOREPLIFETIME(ASlipstreamGameState, RedTeamScore);
	DOREPLIFETIME(ASlipstreamGameState, BlueTeamScore);
}

void ASlipstreamGameState::UpdateTopScore(ABasePlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void ASlipstreamGameState::RedTeamScores()
{
	++RedTeamScore;
	ABasePlayerController* PlayerController = Cast<ABasePlayerController>(GetWorld()->GetFirstPlayerController());
	if (PlayerController)
	{
		PlayerController->SetHUDRedTeamScore(RedTeamScore, TeamDeathMatchMaxScore);
	}
}

void ASlipstreamGameState::BlueTeamScores()
{
	++BlueTeamScore;
	ABasePlayerController* PlayerController = Cast<ABasePlayerController>(GetWorld()->GetFirstPlayerController());
	if (PlayerController)
	{
		PlayerController->SetHUDBlueTeamScore(BlueTeamScore, TeamDeathMatchMaxScore);
	}
}

void ASlipstreamGameState::OnRep_RedTeamScore()
{
	ABasePlayerController* PlayerController = Cast<ABasePlayerController>(GetWorld()->GetFirstPlayerController());
	if (PlayerController)
	{
		PlayerController->SetHUDRedTeamScore(RedTeamScore, TeamDeathMatchMaxScore);
	}
}

void ASlipstreamGameState::OnRep_BlueTeamScore()
{
	ABasePlayerController* PlayerController = Cast<ABasePlayerController>(GetWorld()->GetFirstPlayerController());
	if (PlayerController)
	{
		PlayerController->SetHUDBlueTeamScore(BlueTeamScore, TeamDeathMatchMaxScore);
	}
}
