// Fill out your copyright notice in the Description page of Project Settings.


#include "SlipstreamGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Slipstream/Characters/BasePlayerCharacter.h"
#include "Slipstream/GameState/SlipstreamGameState.h"
#include "Slipstream/PlayerController/BasePlayerController.h"
#include "Slipstream/PlayerState/BasePlayerState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ASlipstreamGameMode::ASlipstreamGameMode()
{
	bDelayedStart = true;
}

void ASlipstreamGameMode::BeginPlay()
{
	Super::BeginPlay();
	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ASlipstreamGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABasePlayerController* PlayerController = Cast<ABasePlayerController>(*It);
		if (PlayerController)
		{
			PlayerController->OnMatchStateSet(MatchState, bTeamsMatch);
		}
	}
}


void ASlipstreamGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f) StartMatch();
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f) SetMatchState(MatchState::Cooldown);
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime<= 0.f) RestartGame();
	}
}

void ASlipstreamGameMode::PlayerEliminated(ABasePlayerCharacter* EliminatedPlayer,
                                           ABasePlayerController* EliminatedPlayerController, ABasePlayerController* AttackerPlayerController)
{
	ABasePlayerState* AttackerPlayerState = AttackerPlayerController ? Cast<ABasePlayerState>(AttackerPlayerController->PlayerState) : nullptr;
	ABasePlayerState* EliminatedPlayerState = EliminatedPlayerController ? Cast<ABasePlayerState>(EliminatedPlayerController->PlayerState) : nullptr;
	ASlipstreamGameState* SlipstreamGameState = GetGameState<ASlipstreamGameState>();

	if (AttackerPlayerState && AttackerPlayerState != EliminatedPlayerState && SlipstreamGameState)
	{
		TArray<ABasePlayerState*> PlayersCurrentlyInLead;
		for (auto LeadPlayer: SlipstreamGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInLead.Add(LeadPlayer);
		}
		
		AttackerPlayerState->AddToScore(1.f);
		SlipstreamGameState->UpdateTopScore(AttackerPlayerState);
		
		if (SlipstreamGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			ABasePlayerCharacter* Leader = Cast<ABasePlayerCharacter>(AttackerPlayerState->GetPawn());
			if (Leader)
			{
				Leader->MulticastGainTheLead();
			}
		}

		for (int32 i = 0; i < PlayersCurrentlyInLead.Num(); i++)
		{
			if (!SlipstreamGameState->TopScoringPlayers.Contains(PlayersCurrentlyInLead[i]))
			{
				ABasePlayerCharacter* Loser = Cast<ABasePlayerCharacter>(PlayersCurrentlyInLead[i]->GetPawn());
				if (Loser)
				{
					Loser->MulticastLostTheLead();
				}
			}
		}
	}
	
	if (EliminatedPlayer && EliminatedPlayerState)
	{
		EliminatedPlayer->Elim(false);
		EliminatedPlayerState->AddToDefeat(1);
	}
	if (EliminatedPlayerState && AttackerPlayerState)
	{
		CreateElimMessage(AttackerPlayerState, EliminatedPlayerState);
	}
}

void ASlipstreamGameMode::RequestRespawn(ACharacter* EliminatedPlayer, AController* EliminatedPlayerController)
{
	if (EliminatedPlayer)
	{
		EliminatedPlayer->Reset();
		EliminatedPlayer->Destroy();
	}
	if (EliminatedPlayerController)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), FoundActors);

		int32 Selection = FMath::RandRange(0, FoundActors.Num() - 1);
		RestartPlayerAtPlayerStart(EliminatedPlayerController, FoundActors[Selection]);
	}
}

void ASlipstreamGameMode::PlayerLeftGame(ABasePlayerState* PlayerState)
{
	if (PlayerState == nullptr) return;
	ASlipstreamGameState* SlipstreamGameState = GetGameState<ASlipstreamGameState>();
	if (SlipstreamGameState && SlipstreamGameState->TopScoringPlayers.Contains(PlayerState))
	{
		SlipstreamGameState->TopScoringPlayers.Remove(PlayerState);
	}
	ABasePlayerCharacter* LeavingPlayer = Cast<ABasePlayerCharacter>(PlayerState->GetPawn());
	if (LeavingPlayer)
	{
		LeavingPlayer->Elim(true);
	}
}

float ASlipstreamGameMode::CalculateDamage(AController* Attacker, AController* Victim, float Damage)
{
	ABasePlayerState* AttackerState = Attacker->GetPlayerState<ABasePlayerState>();
	ABasePlayerState* VictimState = Victim->GetPlayerState<ABasePlayerState>();
	if (AttackerState == nullptr || VictimState == nullptr)
	{
		return Damage;
	}
	if (AttackerState == VictimState)
	{
		return 0.f;
	}
	return Damage;
}

void ASlipstreamGameMode::CreateElimMessage(ABasePlayerState* AttackerState, ABasePlayerState* EliminatedState)
{
	FString AttackerName = AttackerState->GetPlayerName();
	FString EliminatedPlayerName = EliminatedState->GetPlayerName();
	APlayerController* AttackerController = AttackerState->GetPlayerController();
	APlayerController* EliminatedController = EliminatedState->GetPlayerController();
	if (KillMessages.IsEmpty()) KillMessages.Add("eliminated");
	KillMessage = KillMessages[FMath::RandRange(0, KillMessages.Num() - 1)];
	FString EliminationMsg;
		
	if (AttackerName.IsEmpty() || EliminatedPlayerName.IsEmpty()) return;
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ABasePlayerController* PC = Cast<ABasePlayerController>(It->Get()))
		{
			if (PC == AttackerController && PC != EliminatedController) EliminationMsg = FString::Printf(TEXT("You %s %s!"), *KillMessage, *EliminatedPlayerName);
			else if (PC != AttackerController && PC == EliminatedController) EliminationMsg = FString::Printf(TEXT("%s %s you!"), *AttackerName, *KillMessage);
			else if (PC != AttackerController && PC != EliminatedController) EliminationMsg = FString::Printf(TEXT("%s %s %s!"), *AttackerName, *KillMessage, *EliminatedPlayerName);
			else if (PC == AttackerController && PC == EliminatedController) EliminationMsg = FString::Printf(TEXT("You %s yourself!"), *KillMessage);
			else EliminationMsg = FString::Printf(TEXT("ERROR MESSAGE: No Players found"));
			
			if (!EliminationMsg.IsEmpty()) PC->ClientSetHUDElimination(EliminationMsg);
		}
	}
}
