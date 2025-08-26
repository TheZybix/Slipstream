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
			PlayerController->OnMatchStateSet(MatchState);
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
		AttackerPlayerState->AddToScore(1.f);
		SlipstreamGameState->UpdateTopScore(AttackerPlayerState);
	}
	
	if (EliminatedPlayer && EliminatedPlayerState)
	{
		EliminatedPlayer->Elim();
		EliminatedPlayerState->AddToDefeat(1);
	}
	if (EliminatedPlayerState && AttackerPlayerState)
	{
		FString AttackerName = AttackerPlayerState->GetPlayerName();
		FString EliminatedPlayerName = EliminatedPlayerState->GetPlayerName();
		if (!AttackerName.IsEmpty() && !EliminatedPlayerName.IsEmpty())
		{
			FString EliminationMsg = FString::Printf(TEXT("%s eliminated %s"), *AttackerName, *EliminatedPlayerName);
			ShowEliminationText(EliminationMsg);
			
			GetWorldTimerManager().ClearTimer(EliminationTextTimer);
			GetWorldTimerManager().SetTimer(EliminationTextTimer, this, &ASlipstreamGameMode::EliminationTimerFinished, EliminationTextTimerDelay, false);
		}
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


void ASlipstreamGameMode::EliminationTimerFinished()
{
	ShowEliminationText(FString(""));
}

void ASlipstreamGameMode::ShowEliminationText(FString EliminationMsg)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ABasePlayerController* PC = Cast<ABasePlayerController>(It->Get()))
		{
			PC->ClientSetHUDElimination(EliminationMsg);
		}
	}
}
