// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlayerController.h"

#include "Components/ProgressBar.h"
#include "Slipstream/HUD/BasePlayerHUD.h"
#include "Slipstream/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Slipstream/Characters/BasePlayerCharacter.h"


void ABasePlayerController::BeginPlay()
{
	Super::BeginPlay();
	PlayerHUD = Cast<ABasePlayerHUD>(GetHUD());
}

void ABasePlayerController::SetHUDTime()
{
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());
	if (CountdownInt != SecondsLeft) SetHUDMatchCountdown(MatchTime - GetServerTime());
	CountdownInt = SecondsLeft;
}

void ABasePlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
	float TimeSeverReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeSeverReceivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void ABasePlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ABasePlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<ABasePlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->HealthBar && PlayerHUD->CharacterOverlay->HealthText)
	{
		const float HealthPercent = Health / MaxHealth;
		PlayerHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);

		FString HealthText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		PlayerHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void ABasePlayerController::SetHUDScore(float Score)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<ABasePlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->ScoreAmount)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		PlayerHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
}

void ABasePlayerController::SetHUDDefeat(int Defeat)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<ABasePlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->DefeatAmount)
	{
		FString DefeatText = FString::Printf(TEXT("%d"), Defeat);
		PlayerHUD->CharacterOverlay->DefeatAmount->SetText(FText::FromString(DefeatText));
	}
}

void ABasePlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<ABasePlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->AmmoAmount)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		PlayerHUD->CharacterOverlay->AmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ABasePlayerController::SetHUDStoredAmmo(int32 Ammo)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<ABasePlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->StoredAmmoAmount)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		PlayerHUD->CharacterOverlay->StoredAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ABasePlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	
	PlayerHUD = PlayerHUD == nullptr ? Cast<ABasePlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->MatchCountdownTimer)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime/60);
		int32 Seconds = CountdownTime - Minutes * 60;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PlayerHUD->CharacterOverlay->MatchCountdownTimer->SetText(FText::FromString(CountdownText));
	}
}

void ABasePlayerController::ClientSetHUDElimination_Implementation(const FString& EliminationText)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<ABasePlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->EliminationText)
	{
		PlayerHUD->CharacterOverlay->EliminationText->SetText(FText::FromString(EliminationText));
	}
}


void ABasePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ABasePlayerCharacter* PlayerCharacter = Cast<ABasePlayerCharacter>(InPawn);
	if (PlayerCharacter)
	{
		SetHUDHealth(PlayerCharacter->GetHealth(), PlayerCharacter->GetMaxHealth());
	}
}

void ABasePlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ABasePlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();
	CheckTimeSync(DeltaTime);
}

float ABasePlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABasePlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}
