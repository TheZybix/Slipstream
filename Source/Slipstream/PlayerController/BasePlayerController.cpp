// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlayerController.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Slipstream/HUD/BasePlayerHUD.h"
#include "Slipstream/HUD/CharacterOverlay.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Slipstream/Characters/BasePlayerCharacter.h"
#include "Slipstream/Components/CombatComponent.h"
#include "Slipstream/GameModes/SlipstreamGameMode.h"
#include "Slipstream/GameState/SlipstreamGameState.h"
#include "Slipstream/PlayerState/BasePlayerState.h"
#include "Slipstream/HUD/Announcement.h"
#include "Slipstream/Types/Announcement.h"


void ABasePlayerController::BeginPlay()
{
	Super::BeginPlay();
	PlayerHUD = Cast<ABasePlayerHUD>(GetHUD());
	ServerCheckMatchState();
}

void ABasePlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) 	TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress)	TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown)	TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (HasAuthority())
	{
		SlipStreamGameMode = SlipStreamGameMode == nullptr ? Cast<ASlipstreamGameMode>(UGameplayStatics::GetGameMode(this)) : SlipStreamGameMode;
		if (SlipStreamGameMode)
		{
			SecondsLeft = FMath::CeilToInt(SlipStreamGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}
		
	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}
	CountdownInt = SecondsLeft;
}

void ABasePlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (PlayerHUD && PlayerHUD->CharacterOverlay)
		{
			CharacterOverlay = PlayerHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				if (!bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (!bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
				if (!bInitializeScore) SetHUDScore(HUDScore);
				if (!bInitializeDefeat) SetHUDDefeat(HUDDefeat);
				if (!bInitializeMagAmmo) SetHUDWeaponAmmo(HUDMagAmmo);
				if (!bInitializeStoredAmmo) SetHUDStoredAmmo(HUDStoredAmmo);

				ABasePlayerCharacter* PlayerCharacter = Cast<ABasePlayerCharacter>(GetPawn());
				if (PlayerCharacter && PlayerCharacter->GetCombat())
				{
					if (!bInitializeGrenades) SetHUDGrenades(PlayerCharacter->GetCombat()->GetGrenades());
				}
			}
		}
	}
}

void ABasePlayerController::OnRep_ShowTeamScores()
{
	if (bShowTeamScores) InitTeamScores();
	else HideTeamScores();
}

FString ABasePlayerController::GetInfoText(const TArray<ABasePlayerState*>& Players)
{
	FString InfoTextString;
	ABasePlayerState* SlipstreamPlayerState = GetPlayerState<ABasePlayerState>();
	if (SlipstreamPlayerState == nullptr) return FString();
	if (Players.Num() == 0)
	{
		InfoTextString = FString("There is no winner.");
	}
	else if (Players.Num() == 1 && Players[0] == SlipstreamPlayerState)
	{
		InfoTextString = FString("You are the winner!");
	}
	else if (Players.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *Players[0]->GetPlayerName());
	}
	else if (Players.Num() > 1)
	{
		InfoTextString = FString("Players tied for the win: \n");
		for (auto TiedPlayer : Players)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s"), *TiedPlayer->GetPlayerName()));
		}
	}
	return InfoTextString;
}

FString ABasePlayerController::GetTeamsInfoText(ASlipstreamGameState* SlipstreamGameState)
{
	if (SlipstreamGameState == nullptr) return FString();
	FString InfoTextString;

	int32 RedTeamScore = SlipstreamGameState->RedTeamScore;
	int32 BlueTeamScore = SlipstreamGameState->BlueTeamScore;
	if (RedTeamScore > BlueTeamScore) InfoTextString = FString::Printf(TEXT("RED TEAM WON!"));
	if (RedTeamScore == BlueTeamScore) InfoTextString = FString::Printf(TEXT("DRAW!"));
	if (RedTeamScore < BlueTeamScore) InfoTextString = FString::Printf(TEXT("BLUE TEAM WON!"));
	if (RedTeamScore == 0 && BlueTeamScore == 0) InfoTextString = FString::Printf(TEXT("LEARN TO AIM"));
	
	return InfoTextString;
}

void ABasePlayerController::HighPingWarning()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<ABasePlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->HighPingImage && PlayerHUD->CharacterOverlay->HighPingAnimation)
	{
		PlayerHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		PlayerHUD->CharacterOverlay->PlayAnimation(PlayerHUD->CharacterOverlay->HighPingAnimation, 0.f, 5.f);
	}
}

void ABasePlayerController::StopHighPingWarning()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<ABasePlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->HighPingImage && PlayerHUD->CharacterOverlay->HighPingAnimation)
	{
		PlayerHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if (PlayerHUD->CharacterOverlay->IsAnimationPlaying(PlayerHUD->CharacterOverlay->HighPingAnimation)) PlayerHUD->CharacterOverlay->StopAnimation(PlayerHUD->CharacterOverlay->HighPingAnimation);
	}
}

void ABasePlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

void ABasePlayerController::CheckPing(float DeltaTime)
{
	PlayerState = PlayerState == nullptr ? GetPlayerState<ABasePlayerState>() : PlayerState;
	if(GEngine && PlayerState) GEngine->AddOnScreenDebugMessage(2, 5.f, FColor::Red, FString::Printf(TEXT("Player ping: %hhu"), PlayerState->GetCompressedPing() * 4));
	
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		//PlayerState = PlayerState == nullptr ? GetPlayerState<ABasePlayerState>() : PlayerState;
		if (PlayerState)
		{
			if (PlayerState->GetCompressedPing() * 4 > HighPingThreshold)
			{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0.f;
	}
	if (PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->HighPingAnimation && PlayerHUD->CharacterOverlay->IsAnimationPlaying(PlayerHUD->CharacterOverlay->HighPingAnimation))
	{
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void ABasePlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
                                                                  float TimeSeverReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimeSeverReceivedClientRequest + SingleTripTime;
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
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ABasePlayerController::SetHUDShield(float Shield, float MaxShield)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<ABasePlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->ShieldBar && PlayerHUD->CharacterOverlay->ShieldText)
	{
		const float ShieldPercent = Shield / MaxShield;
		PlayerHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);

		FString ShieldText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		PlayerHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
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
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
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
	else
	{
		bInitializeDefeat = true;
		HUDDefeat = Defeat;
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
	else
	{
		HUDMagAmmo = Ammo;
		bInitializeMagAmmo = true;
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
	else
	{
		HUDStoredAmmo = Ammo;
		bInitializeStoredAmmo = true;
	}
}

void ABasePlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<ABasePlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->MatchCountdownTimer)
	{
		if (CountdownTime < 0.f)
		{
			PlayerHUD->CharacterOverlay->MatchCountdownTimer->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime/60);
		int32 Seconds = CountdownTime - Minutes * 60;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PlayerHUD->CharacterOverlay->MatchCountdownTimer->SetText(FText::FromString(CountdownText));
	}
}

void ABasePlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<ABasePlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD && PlayerHUD->Announcement && PlayerHUD->Announcement->WarmupTime)
	{
		if (CountdownTime < 0.f)
		{
			PlayerHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime/60);
		int32 Seconds = CountdownTime - Minutes * 60;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PlayerHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void ABasePlayerController::SetHUDGrenades(int32 Grenades)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<ABasePlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->GrenadeText)
	{
		FString GrenadeText = FString::Printf(TEXT("%d"), Grenades);
		PlayerHUD->CharacterOverlay->GrenadeText->SetText(FText::FromString(GrenadeText));
	}
	else
	{
		HUDGrenades = Grenades;
	}
}

void ABasePlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABasePlayerController, MatchState);
	DOREPLIFETIME(ABasePlayerController, bShowTeamScores);
}

void ABasePlayerController::HideTeamScores()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<ABasePlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->RedTeamBar && PlayerHUD->CharacterOverlay->RedTeamText)
	{
		PlayerHUD->CharacterOverlay->RedTeamBar->SetVisibility(ESlateVisibility::Hidden);
		PlayerHUD->CharacterOverlay->RedTeamText->SetText(FText());
	}
	if (PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->BlueTeamBar && PlayerHUD->CharacterOverlay->BlueTeamText)
	{
		PlayerHUD->CharacterOverlay->BlueTeamBar->SetVisibility(ESlateVisibility::Hidden);
		PlayerHUD->CharacterOverlay->BlueTeamText->SetText(FText());
	}
}

void ABasePlayerController::InitTeamScores()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<ABasePlayerHUD>(GetHUD()) : PlayerHUD;
	FString Zero ("0");
	if (PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->RedTeamBar && PlayerHUD->CharacterOverlay->RedTeamText)
	{
		PlayerHUD->CharacterOverlay->RedTeamBar->SetVisibility(ESlateVisibility::Visible);
		PlayerHUD->CharacterOverlay->RedTeamBar->SetPercent(0.f);
		PlayerHUD->CharacterOverlay->RedTeamText->SetText(FText::FromString(Zero));
	}
	if (PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->BlueTeamBar && PlayerHUD->CharacterOverlay->BlueTeamText)
	{
		PlayerHUD->CharacterOverlay->BlueTeamBar->SetVisibility(ESlateVisibility::Visible);
		PlayerHUD->CharacterOverlay->BlueTeamBar->SetPercent(0.f);
		PlayerHUD->CharacterOverlay->BlueTeamText->SetText(FText::FromString(Zero));
	}
}

void ABasePlayerController::SetHUDRedTeamScore(int32 RedScore, int32 MaxScore)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<ABasePlayerHUD>(GetHUD()) : PlayerHUD;
	FString ScoreText = FString::Printf(TEXT("%d"), RedScore);
	float Percent = (float)RedScore / MaxScore;
	if (PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->RedTeamBar && PlayerHUD->CharacterOverlay->RedTeamText)
	{
		PlayerHUD->CharacterOverlay->RedTeamBar->SetPercent(Percent);
		PlayerHUD->CharacterOverlay->RedTeamText->SetText(FText::FromString(ScoreText));
	}
}

void ABasePlayerController::SetHUDBlueTeamScore(int32 BlueScore, int32 MaxScore)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<ABasePlayerHUD>(GetHUD()) : PlayerHUD;
	FString ScoreText = FString::Printf(TEXT("%d"), BlueScore);
	float Percent = (float)BlueScore / MaxScore;
	if (PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->BlueTeamBar && PlayerHUD->CharacterOverlay->BlueTeamText)
	{
		PlayerHUD->CharacterOverlay->BlueTeamBar->SetPercent(Percent);
		PlayerHUD->CharacterOverlay->BlueTeamText->SetText(FText::FromString(ScoreText));
	}
}

void ABasePlayerController::ClientSetHUDElimination_Implementation(const FString& EliminationText)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<ABasePlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD)
	{
		PlayerHUD->AddElimAnnouncement(EliminationText);
	}
}


void ABasePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ABasePlayerCharacter* PlayerCharacter = Cast<ABasePlayerCharacter>(InPawn);
	if (PlayerCharacter)
	{
		SetHUDHealth(PlayerCharacter->GetHealth(), PlayerCharacter->GetMaxHealth());
		SetHUDShield(PlayerCharacter->GetShield(), PlayerCharacter->GetMaxShield());
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
	PollInit();

	CheckPing(DeltaTime);
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

void ABasePlayerController::OnMatchStateSet(FName State, bool bTeamsMatch)
{
	MatchState = State;
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);	
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABasePlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABasePlayerController::HandleMatchHasStarted(bool bTeamsMatch)
{
	if (HasAuthority()) bShowTeamScores = bTeamsMatch;
	PlayerHUD = PlayerHUD == nullptr ? Cast<ABasePlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD)
	{
		if (PlayerHUD->CharacterOverlay == nullptr) PlayerHUD->AddCharacterOverlay();
		if (PlayerHUD->Announcement)
		{
			PlayerHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		if (!HasAuthority()) return;
		if (bTeamsMatch) InitTeamScores();
		else HideTeamScores();
	}
}

void ABasePlayerController::HandleCooldown()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<ABasePlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD)
	{
		PlayerHUD->CharacterOverlay->RemoveFromParent();
		if (PlayerHUD->Announcement && PlayerHUD->Announcement->AnnouncementText && PlayerHUD->Announcement->InfoText)
		{
			PlayerHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText = Announcement::NewMatchStartsIn;
			PlayerHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			ASlipstreamGameState* SlipstreamGameState = Cast<ASlipstreamGameState>(UGameplayStatics::GetGameState(this));
			ABasePlayerState* SlipstreamPlayerState = GetPlayerState<ABasePlayerState>();
			if (SlipstreamGameState)
			{
				TArray<ABasePlayerState*> TopScoringPlayers = SlipstreamGameState->TopScoringPlayers;
				FString InfoTextString = bShowTeamScores ? GetTeamsInfoText(SlipstreamGameState) : GetInfoText(TopScoringPlayers);
				
				PlayerHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	ABasePlayerCharacter* PlayerCharacter = Cast<ABasePlayerCharacter>(GetPawn());
	if (PlayerCharacter && PlayerCharacter->GetCombat())
	{
		PlayerCharacter->bDisableGameplay = true;
		PlayerCharacter->GetCombat()->TriggerKeyPressed(false);
	}
}

void ABasePlayerController::ServerCheckMatchState_Implementation()
{
	ASlipstreamGameMode* GameMode = Cast<ASlipstreamGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, LevelStartingTime, CooldownTime);

		if (PlayerHUD && MatchState == MatchState::WaitingToStart)
		{
			PlayerHUD->AddAnnouncement();
		}
	}
}

void ABasePlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float StartingTime, float Cooldown)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	
	if (PlayerHUD && MatchState == MatchState::WaitingToStart)
	{
		PlayerHUD->AddAnnouncement();
	}
}