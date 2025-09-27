// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamDeathMatchGameMode.h"
#include "Slipstream/PlayerState/BasePlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Slipstream/GameState/SlipstreamGameState.h"

ATeamDeathMatchGameMode::ATeamDeathMatchGameMode()
{
	bTeamsMatch = true;
}

void ATeamDeathMatchGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	ASlipstreamGameState* SGameState = Cast<ASlipstreamGameState>(UGameplayStatics::GetGameState(this));
	if (SGameState)
	{
		ABasePlayerState* PlayerState = NewPlayer->GetPlayerState<ABasePlayerState>();
		if (PlayerState && PlayerState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (SGameState->BlueTeam.Num() >= SGameState->RedTeam.Num())
			{
				SGameState->RedTeam.AddUnique(PlayerState);
				PlayerState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				SGameState->BlueTeam.AddUnique(PlayerState);
				PlayerState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamDeathMatchGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	ASlipstreamGameState* SGameState = Cast<ASlipstreamGameState>(UGameplayStatics::GetGameState(this));
	ABasePlayerState* PlayerState = Exiting->GetPlayerState<ABasePlayerState>();
	if (SGameState && PlayerState)
	{
		if (SGameState->RedTeam.Contains(PlayerState)) SGameState->RedTeam.Remove(PlayerState);
		if (SGameState->BlueTeam.Contains(PlayerState)) SGameState->BlueTeam.Remove(PlayerState);
	}
}

float ATeamDeathMatchGameMode::CalculateDamage(AController* Attacker, AController* Victim, float Damage)
{
	ABasePlayerState* AttackerState = Attacker->GetPlayerState<ABasePlayerState>();
	ABasePlayerState* VictimState = Victim->GetPlayerState<ABasePlayerState>();
	if (AttackerState == nullptr || VictimState == nullptr)
	{
		return Damage;
	}
	if (AttackerState == VictimState || AttackerState->GetTeam() == VictimState->GetTeam())
	{
		return 0.f;
	}
	return Damage;
}

void ATeamDeathMatchGameMode::PlayerEliminated(ABasePlayerCharacter* EliminatedPlayer,
	ABasePlayerController* EliminatedPlayerController, ABasePlayerController* AttackerPlayerController)
{
	Super::PlayerEliminated(EliminatedPlayer, EliminatedPlayerController, AttackerPlayerController);
	
	ASlipstreamGameState* SGameState = Cast<ASlipstreamGameState>(UGameplayStatics::GetGameState(this));
	ABasePlayerState* AttackerPlayerState = AttackerPlayerController ? Cast<ABasePlayerState>(AttackerPlayerController->PlayerState) : nullptr;
	if (SGameState && AttackerPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			SGameState->BlueTeamScores();
		}
		if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			SGameState->RedTeamScores();
		}
	}
}

void ATeamDeathMatchGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	/* Add Players to teams */
	ASlipstreamGameState* SGameState = Cast<ASlipstreamGameState>(UGameplayStatics::GetGameState(this));
	if (SGameState)
	{
		for (auto PState: SGameState->PlayerArray)
		{
			ABasePlayerState* PlayerState = Cast<ABasePlayerState>(PState.Get());
			if (PlayerState && PlayerState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (SGameState->BlueTeam.Num() >= SGameState->RedTeam.Num())
				{
					SGameState->RedTeam.AddUnique(PlayerState);
					PlayerState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					SGameState->BlueTeam.AddUnique(PlayerState);
					PlayerState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}
