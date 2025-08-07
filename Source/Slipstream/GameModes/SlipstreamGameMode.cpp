// Fill out your copyright notice in the Description page of Project Settings.


#include "SlipstreamGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Slipstream/Characters/BasePlayerCharacter.h"
#include "Slipstream/PlayerController/BasePlayerController.h"
#include "Slipstream/PlayerState/BasePlayerState.h"

void ASlipstreamGameMode::PlayerEliminated(ABasePlayerCharacter* EliminatedPlayer,
	ABasePlayerController* EliminatedPlayerController, ABasePlayerController* AttackerPlayerController)
{
	ABasePlayerState* AttackerPlayerState = AttackerPlayerController ? Cast<ABasePlayerState>(AttackerPlayerController->PlayerState) : nullptr;
	ABasePlayerState* EliminatedPlayerState = EliminatedPlayerController ? Cast<ABasePlayerState>(EliminatedPlayerController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != EliminatedPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}
	
	if (EliminatedPlayer) EliminatedPlayer->Elim();
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
