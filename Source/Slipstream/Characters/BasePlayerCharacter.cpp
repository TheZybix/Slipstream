// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Slipstream/Components/CombatComponent.h"
#include "Slipstream/Weapon/WeaponBase.h"
#include "BaseCharacterAnimInstance.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Slipstream/Slipstream.h"
#include "Slipstream/GameModes/SlipstreamGameMode.h"
#include "Slipstream/PlayerController/BasePlayerController.h"
#include "TimerManager.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Slipstream/Components/BuffComponent.h"
#include "Slipstream/Components/LagCompensationComponent.h"
#include "Slipstream/GameModes/TeamDeathMatchGameMode.h"
#include "Slipstream/GameState/SlipstreamGameState.h"
#include "Slipstream/HUD/ReturnToMainMenu.h"
#include "Slipstream/PlayerState/BasePlayerState.h"
#include "Slipstream/Types/WeaponTypes.h"

ABasePlayerCharacter::ABasePlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm Component"));
	SpringArmComponent->SetupAttachment(GetMesh());
	SpringArmComponent->TargetArmLength = 600;
	SpringArmComponent->bUsePawnControlRotation = true;
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	CombatComponent->SetIsReplicated(true);

	BuffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("Buff"));
	BuffComponent->SetIsReplicated(true);

	LagCompensationComponent = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 1000.f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 3.0f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Dissolve Timeline"));


	/* Hitboxes for server side rewind */
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitBoxes.Add(FName("head"), head);
	
	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitBoxes.Add(FName("pelvis"), pelvis);
	
	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitBoxes.Add(FName("spine_02"), spine_02);
	
	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitBoxes.Add(FName("spine_03"), spine_03);
	
	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitBoxes.Add(FName("upperarm_l"), upperarm_l);
	
	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitBoxes.Add(FName("upperarm_r"), upperarm_r);
	
	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitBoxes.Add(FName("lowerarm_l"), lowerarm_l);
	
	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitBoxes.Add(FName("lowerarm_r"), lowerarm_r);
	
	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitBoxes.Add(FName("hand_l"), hand_l);
	
	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitBoxes.Add(FName("hand_r"), hand_r);
	
	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitBoxes.Add(FName("thigh_l"), thigh_l);
	
	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitBoxes.Add(FName("thigh_r"), thigh_r);
	
	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitBoxes.Add(FName("calf_l"), calf_l);
	
	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitBoxes.Add(FName("calf_r"), calf_r);
	
	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitBoxes.Add(FName("foot_l"), foot_l);
	
	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitBoxes.Add(FName("foot_r"), foot_r);

	for (auto Box : HitBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ABasePlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABasePlayerCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABasePlayerCharacter, Health);
	DOREPLIFETIME(ABasePlayerCharacter, Shield);
	DOREPLIFETIME(ABasePlayerCharacter, bDisableGameplay)
}

void ABasePlayerCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void ABasePlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComponent) CombatComponent->Character = this;
	if (BuffComponent)
	{
		BuffComponent->Character = this;
		BuffComponent->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		BuffComponent->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if (LagCompensationComponent)
	{
		LagCompensationComponent->Character = this;
		if (PlayerController) LagCompensationComponent->PlayerController = Cast<ABasePlayerController>(Controller);
	}
}

void ABasePlayerCharacter::PlayFireMontage(bool bAiming)
{
	if (!CombatComponent || !CombatComponent->EquippedWeapon) return;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		
		/*FName SectionName;
		 *SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		 *AnimInstance->Montage_JumpToSection(SectionName);*/
	}
}

void ABasePlayerCharacter::PlayReloadMontage()
{
	if (!CombatComponent || !CombatComponent->EquippedWeapon) return;
		
	UAnimMontage* MontageToPlay;
	switch (CombatComponent->EquippedWeapon->GetWeaponType())
	{
	case EWeaponType::EWT_AssaultRifle:
		MontageToPlay = AssaultRifleMontage;
		break;
	case EWeaponType::EWT_RocketLauncher:
		MontageToPlay = RocketLauncherMontage;
		break;
	case EWeaponType::EWT_Pistol:
		MontageToPlay = AssaultRifleMontage;
		break;
	case EWeaponType::EWT_SMG:
		MontageToPlay = AssaultRifleMontage;
		break;
	case EWeaponType::EWT_Shotgun:
		MontageToPlay = ShotgunMontage;
		break;
	case EWeaponType::EWT_SniperRifle:
		MontageToPlay = AssaultRifleMontage;
		break;
	case EWeaponType::EWT_GrenadeLauncher:
		MontageToPlay = AssaultRifleMontage;
		break;

	case EWeaponType::EWT_Max:
		return;
			
	default:
		return;
	}
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && MontageToPlay)
	{
		AnimInstance->Montage_Play(MontageToPlay);
		FName SectionName = FName("Reload");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABasePlayerCharacter::PlayDeathMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}
}

void ABasePlayerCharacter::PlaySwapWeaponMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapWeaponMontage)
	{
		AnimInstance->Montage_Play(SwapWeaponMontage);
	}
}

void ABasePlayerCharacter::Elim(bool bPlayerLeftGame)
{
	MulticastElim(bPlayerLeftGame);
	SetCustomDepthStencilForNonLocalPlayers(false);
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		if (CombatComponent->EquippedWeapon->bDestroyWeapon) CombatComponent->EquippedWeapon->Destroy();
		else CombatComponent->EquippedWeapon->Dropped();

		if (CombatComponent->SecondaryWeapon && CombatComponent->SecondaryWeapon->bDestroyWeapon) CombatComponent->SecondaryWeapon->Destroy();
		else if (CombatComponent->SecondaryWeapon) CombatComponent->SecondaryWeapon->Dropped();
	}
}

void ABasePlayerCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	SetCustomDepthStencilForNonLocalPlayers(false);
	if (PlayerController)
	{
		PlayerController->SetHUDWeaponAmmo(0);
	}
	bIsDead = true;
	PlayDeathMontage();

	/* Dissolve character */
	InitializeMaterials();
	for (int32 Selection = 0; Selection < DissolveMaterials.Num(); Selection++)
	{
		if (DissolveMaterials[Selection])
		{
			UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(DissolveMaterials[Selection], this);
			DynamicDissolveMaterials.Add(DynamicMaterial);
			int32 Slot = GetMesh()->GetMaterialIndex(MaterialSlots[Selection]);
			GetMesh()->SetMaterial(Slot, DynamicDissolveMaterials[Selection]);
			DynamicDissolveMaterials[Selection]->SetScalarParameterValue("DissolveAmount", -0.5f);
			DynamicDissolveMaterials[Selection]->SetScalarParameterValue("GlowAmount", 150.f);
			UE_LOG(LogTemp, Warning, TEXT("Updating Material Slot %d: %s"), Selection, *DynamicDissolveMaterials[Selection]->GetName());
		}
	}
	StartDissolve();

	/* Disable character movement */
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	
	bDisableGameplay = true;
	GetCharacterMovement()->DisableMovement();
	if (CombatComponent)
	{
		CombatComponent->TriggerKeyPressed(false);
	}
	/* Disable collision */
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/* Spawn elimination bot */
	if (EliminationBotParticles)
	{
		FVector SpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		EliminationBotParticlesComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EliminationBotParticles, SpawnPoint, GetActorRotation());
	}
	if (EliminationBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, EliminationBotSound, GetActorLocation());
	}
	if (IsLocallyControlled() && CombatComponent && CombatComponent->bAiming && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle) ShowSniperScopeWidget(false);

	if (CrownComponent) CrownComponent->DestroyComponent();
	
	GetWorldTimerManager().SetTimer(ElimTimer, this, &ABasePlayerCharacter::ElimTimerFinished, ElimDelay);
}

void ABasePlayerCharacter::ElimTimerFinished()
{
	SlipstreamGameMode = SlipstreamGameMode == nullptr ? GetWorld()->GetAuthGameMode<ASlipstreamGameMode>() : SlipstreamGameMode;
	if (SlipstreamGameMode && !bLeftGame)
	{
		SlipstreamGameMode->RequestRespawn(this, Controller);
	}
	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

void ABasePlayerCharacter::ServerLeaveGame_Implementation()
{
	SlipstreamGameMode = SlipstreamGameMode == nullptr ? GetWorld()->GetAuthGameMode<ASlipstreamGameMode>() : SlipstreamGameMode;
	BasePlayerState = BasePlayerState == nullptr ? GetPlayerState<ABasePlayerState>() : BasePlayerState;
	if (SlipstreamGameMode && BasePlayerState)
	{
		SlipstreamGameMode->PlayerLeftGame(BasePlayerState);
	}
}

void ABasePlayerCharacter::PlayHitReactMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr || CombatComponent->CombatState != ECombatState::ECS_Unoccupied) return;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		
		FName SectionName = FName("FromFront");
		/*SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");*/
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABasePlayerCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && GrenadeThrowMontage)
	{
		AnimInstance->Montage_Play(GrenadeThrowMontage);
	}
}

void ABasePlayerCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                         AController* InstigatorController, AActor* DamageCauser)
{
	SlipstreamGameMode = SlipstreamGameMode == nullptr ? GetWorld()->GetAuthGameMode<ASlipstreamGameMode>() : SlipstreamGameMode;
	
	if (bIsDead || SlipstreamGameMode == nullptr) return;
	Damage = SlipstreamGameMode->CalculateDamage(InstigatorController, Controller, Damage);
	
	// Subtract damage from shield first
	float DamageToHealth = Damage;
	if (Shield > 0.f)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}
		else if (Shield < Damage)
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
			Shield = 0.f;
		}
	}
	
	Health = FMath::Clamp(Health - DamageToHealth, 0.0f, MaxHealth);
	
	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMontage();

	if (Health == 0.f)
	{
		if (SlipstreamGameMode)
		{
			PlayerController = PlayerController == nullptr ? Cast<ABasePlayerController>(Controller) : PlayerController;
			ABasePlayerController* AttackerController = Cast<ABasePlayerController>(InstigatorController);
			SlipstreamGameMode->PlayerEliminated(this, PlayerController, AttackerController);
		}
	}
}


void ABasePlayerCharacter::UpdateHUDHealth()
{
	PlayerController = PlayerController == nullptr ? Cast<ABasePlayerController>(Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABasePlayerCharacter::UpdateHUDShield()
{
	PlayerController = PlayerController == nullptr ? Cast<ABasePlayerController>(Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void ABasePlayerCharacter::UpdateHUDAmmo()
{
	PlayerController = PlayerController == nullptr ? Cast<ABasePlayerController>(Controller) : PlayerController;
	if (PlayerController && CombatComponent && CombatComponent->EquippedWeapon)
	{
		PlayerController->SetHUDWeaponAmmo(CombatComponent->EquippedWeapon->GetMagAmmo());
		PlayerController->SetHUDStoredAmmo(CombatComponent->EquippedWeapon->GetStoredAmmo());
	}
}

void ABasePlayerCharacter::PollInit()
{
	if (BasePlayerState == nullptr)
	{
		BasePlayerState = GetPlayerState<ABasePlayerState>();
		if (BasePlayerState)
		{
			BasePlayerState->AddToScore(0.f);
			BasePlayerState->AddToDefeat(0);
		}
		
		ASlipstreamGameState* GameState = Cast<ASlipstreamGameState>(UGameplayStatics::GetGameState(this));
		if (GameState && GameState->TopScoringPlayers.Contains(BasePlayerState))
		{
			MulticastGainTheLead();
		}
	}

	if (!bShowCharacterOutline && !IsLocallyControlled())
	{
		if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Activating Outline")));
		SetCustomDepthStencilForNonLocalPlayers(true);
	}
}

void ABasePlayerCharacter::InitializeMaterials()
{
	if (DissolveHeadSkin)
	{
		DissolveMaterials.Add(DissolveHeadSkin);
		MaterialSlots.Add(FName("MI_HeadSkin"));
	}

	if (DissolveGloves) 
	{
		DissolveMaterials.Add(DissolveGloves);
		MaterialSlots.Add(FName("MI_Gloves"));
	}
	if (DissolveBodySkin) 
	{
		DissolveMaterials.Add(DissolveBodySkin);
		MaterialSlots.Add(FName("MI_BodySkin"));
	}
	if (DissolveBikini) 
	{
		DissolveMaterials.Add(DissolveBikini);
		MaterialSlots.Add(FName("MI_Bikini"));
	}
	if (DissolveShirts) 
	{
		DissolveMaterials.Add(DissolveShirts);
		MaterialSlots.Add(FName("MI_Shirts"));
	}
	if (DissolvePants) 
	{
		DissolveMaterials.Add(DissolvePants);
		MaterialSlots.Add(FName("MI_Pants"));

	}
	if (DissolvePants) 
	{
		DissolveMaterials.Add(DissolvePants);
		MaterialSlots.Add(FName("MI_Pants_Cloth"));
	}
	if (DissolveShoes) 
	{
		DissolveMaterials.Add(DissolveShoes);
		MaterialSlots.Add(FName("MI_Shoes"));
	}
}


void ABasePlayerCharacter::MulticastGainTheLead_Implementation()
{
	
	if (CrownSystem == nullptr) return;
	if (CrownComponent == nullptr)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(CrownSystem, GetMesh(), FName("CrownSocket"),  FVector::ZeroVector,FRotator::ZeroRotator, EAttachLocation::SnapToTarget, false);
	}
	if (CrownComponent)
	{
		CrownComponent->Activate();
	}
}

void ABasePlayerCharacter::MulticastLostTheLead_Implementation()
{
	if (CrownComponent) CrownComponent->DestroyComponent();
}

void ABasePlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	UpdateHUDHealth();
	UpdateHUDShield();

	ASlipstreamGameState* CombatGameState = Cast<ASlipstreamGameState>(UGameplayStatics::GetGameState(this));

	if (CombatGameState)
	{
		PlayerController = PlayerController == nullptr ? Cast<ABasePlayerController>(Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDRedTeamScore(CombatGameState->RedTeamScore, CombatGameState->TeamDeathMatchMaxScore);
			PlayerController->SetHUDBlueTeamScore(CombatGameState->BlueTeamScore, CombatGameState->TeamDeathMatchMaxScore);
		}
	}
	
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABasePlayerCharacter::ReceiveDamage);
	}

	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}

void ABasePlayerCharacter::SetCustomDepthStencilForNonLocalPlayers(bool bShowOutline)
{
	if (!bShowOutline)
	{
		GetMesh()->SetRenderCustomDepth(false);
		bShowCharacterOutline = false;
		return;
	}
	if(GEngine) GEngine->AddOnScreenDebugMessage(5, 20.f, FColor::Red, FString::Printf(TEXT("Enable Custom Depth")));
	
	if(!IsLocallyControlled())
	{
		ABasePlayerCharacter* LocalPlayerCharacter = Cast<ABasePlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
		BasePlayerState = BasePlayerState == nullptr ? GetPlayerState<ABasePlayerState>() : BasePlayerState;
		if(GEngine) GEngine->AddOnScreenDebugMessage(6, 20.f, FColor::Red, FString::Printf(TEXT("Is not locally controlled")));
		
		if (LocalPlayerCharacter)
		{
			ABasePlayerState* LocalPlayerState = LocalPlayerCharacter->BasePlayerState;
			ASlipstreamGameState* GameState = Cast<ASlipstreamGameState>(UGameplayStatics::GetGameState(this));
		
			if (GameState && GameState->bTeamsMatch && LocalPlayerState) //This is Team Death Match
			{
				if(GEngine) GEngine->AddOnScreenDebugMessage(7, 20.f, FColor::Red, FString::Printf(TEXT("Is Team Death Match")));
				if (BasePlayerState && BasePlayerState->GetTeam() == LocalPlayerState->GetTeam())
				{
					if(GEngine) GEngine->AddOnScreenDebugMessage(8, 20.f, FColor::Red, FString::Printf(TEXT("Is on Own Team")));
					GetMesh()->SetRenderCustomDepth(true);
					GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
					GetMesh()->MarkRenderStateDirty();
					bShowCharacterOutline = true;
				}
				else if (BasePlayerState && BasePlayerState->GetTeam() != LocalPlayerState->GetTeam())
				{
					if(GEngine) GEngine->AddOnScreenDebugMessage(9, 20.f, FColor::Red, FString::Printf(TEXT("Is on Enemy Team")));
					GetMesh()->SetRenderCustomDepth(true);
					GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
					GetMesh()->MarkRenderStateDirty();
					bShowCharacterOutline = true;
				}
			}
			else if (GameState && !GameState->bTeamsMatch) //This is Free for All
			{
				if(GEngine) GEngine->AddOnScreenDebugMessage(10, 20.f, FColor::Red, FString::Printf(TEXT("Is Free For All")));
				GetMesh()->SetRenderCustomDepth(true);
				GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
				GetMesh()->MarkRenderStateDirty();
				bShowCharacterOutline = true;
			}
		}
	}
}

void ABasePlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	RotateInPlace(DeltaTime);
	HideCameraIfCharacterClose();
	PollInit();
}

void ABasePlayerCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	if (GetLocalRole() > ROLE_SimulatedProxy && IsLocallyControlled()) AimOffset(DeltaTime);
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f) OnRep_ReplicatedMovement();
		CalculateAO_Pitch();
	}
}

void ABasePlayerCharacter::Restart()
{
	Super::Restart();
	InitializeMappingContext();
}

void ABasePlayerCharacter::InitializeMappingContext()
{
	/*TObjectPtr<APlayerController> PlayerControllerInput = Cast<APlayerController>(GetController());
	if (PlayerControllerInput)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerControllerInput->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}*/
	
	if (!bIsMappingContextAdded)
	{
		if (TObjectPtr<APlayerController> PlayerControllerInput = Cast<APlayerController>(GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerControllerInput->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
				bIsMappingContextAdded = true;
			}
		}
	}
}

void ABasePlayerCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;
	if ((CameraComponent->GetComponentLocation() - GetActorLocation()).Size() < HideCameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void ABasePlayerCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		/* Correct pitch for multiplayer replication, as during compression the server doesn't use negative numbers for pitch below 0. */
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABasePlayerCharacter::AimOffset(float DeltaTime)
{
	if (CombatComponent && CombatComponent->EquippedWeapon == nullptr) return;
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir)
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir)
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
	
	CalculateAO_Pitch();
}

void ABasePlayerCharacter::SimProxiesTurn()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;
	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;		
	}
	
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_TurningRight;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_TurningLeft;
		}
		else TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

float ABasePlayerCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	return Speed;
}

void ABasePlayerCharacter::Jump()
{
	if (bIsCrouched) UnCrouch();
	else Super::Jump();
}

void ABasePlayerCharacter::Destroyed()
{
	Super::Destroyed();
	if (EliminationBotParticlesComponent)
	{
		EliminationBotParticlesComponent->DestroyComponent();
	}
	
	SlipstreamGameMode = SlipstreamGameMode == nullptr ? GetWorld()->GetAuthGameMode<ASlipstreamGameMode>() : SlipstreamGameMode;
	bool bMatchNotInProgress = SlipstreamGameMode && SlipstreamGameMode->GetMatchState() != MatchState::InProgress;
	if (CombatComponent && CombatComponent->EquippedWeapon && bMatchNotInProgress)
	{
		CombatComponent->EquippedWeapon->Destroy();
	}
}

void ABasePlayerCharacter::Move(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;
	const FVector2D MovementVector = Value.Get<FVector2D>();

	//Get rotation from controller instead of player root
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
	
	//Determine forward direction from Rotation Matrix X, add it to movement input
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MovementVector.Y);
	
	//Determine right direction from Rotation Matrix Y, add it to movement input
	const FVector RightDirection = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ABasePlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisValue = Value.Get<FVector2D>();
	if(GetController())
	{
		AddControllerYawInput(LookAxisValue.X);
		AddControllerPitchInput(LookAxisValue.Y*-1.f);
	}
}

void ABasePlayerCharacter::JumpKeyPressed(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;
	Jump();
}

void ABasePlayerCharacter::EquipKeyPressed(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;
	if (CombatComponent)
	{
		if (CombatComponent->CombatState == ECombatState::ECS_Unoccupied) ServerEquipKeyPressed();
		if (CombatComponent->ShouldSwapWeapons() && !HasAuthority() && CombatComponent->CombatState == ECombatState::ECS_Unoccupied && OverlappingWeapon == nullptr)
		{
			PlaySwapWeaponMontage();
			CombatComponent->CombatState = ECombatState::ECS_SwappingWeapons;
			bFinishedSwapping = false;
		}
	}
}

void ABasePlayerCharacter::ServerEquipKeyPressed_Implementation()
{
	if (bDisableGameplay) return;
	if (CombatComponent)
	{
		if (OverlappingWeapon) CombatComponent->EquipWeapon(OverlappingWeapon);
		else if (CombatComponent->ShouldSwapWeapons())
		{
			CombatComponent->SwapWeapon();
		}
	}
}

void ABasePlayerCharacter::CrouchKeyPressed(const FInputActionValue& Value)
{
	if (bIsCrouched) UnCrouch();
	else Crouch();
}


void ABasePlayerCharacter::AimKeyPressed(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;
	if (CombatComponent)
	{
		CombatComponent->SetAiming(Value.Get<bool>());
	}
}

void ABasePlayerCharacter::TriggerKeyPressed(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->TriggerKeyPressed(Value.Get<bool>());
	}
}

void ABasePlayerCharacter::ReloadKeyPressed(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;
	if (CombatComponent)
	{
		CombatComponent->Reload();
	}
}

void ABasePlayerCharacter::GrenadeKeyPressed(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->ThrowGrenade();
	}
}

void ABasePlayerCharacter::PauseKeyPressed(const FInputActionValue& Value)
{
	if (ReturnToMenuWidget == nullptr) return;
	if (ReturnToMenu == nullptr)
	{
		PlayerController = PlayerController == nullptr ? Cast<ABasePlayerController>(Controller) : PlayerController;
		if (PlayerController) ReturnToMenu = CreateWidget<UReturnToMainMenu>(PlayerController, ReturnToMenuWidget);
	}
	if (ReturnToMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			ReturnToMenu->MenuSetup();
		}
		else ReturnToMenu->MenuTeardown();
	}
	
}


void ABasePlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if(UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::JumpKeyPressed);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &ABasePlayerCharacter::EquipKeyPressed);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::CrouchKeyPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::AimKeyPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ABasePlayerCharacter::AimKeyPressed);
		EnhancedInputComponent->BindAction(TriggerAction, ETriggerEvent::Started, this, &ABasePlayerCharacter::TriggerKeyPressed);
		EnhancedInputComponent->BindAction(TriggerAction, ETriggerEvent::Completed, this, &ABasePlayerCharacter::TriggerKeyPressed);
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::ReloadKeyPressed);
		EnhancedInputComponent->BindAction(GrenadeThrowAction, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::GrenadeKeyPressed);
		EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &ABasePlayerCharacter::PauseKeyPressed);
	}
}

void ABasePlayerCharacter::OnRep_OverlappingWeapon(AWeaponBase* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickUpWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickUpWidget(false);
	}
}

void ABasePlayerCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_TurningRight;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_TurningLeft;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath:: FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw)<15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ABasePlayerCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth) PlayHitReactMontage();
}

void ABasePlayerCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield) PlayHitReactMontage();
}

void ABasePlayerCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	for (int32 Selection = 0; Selection < DynamicDissolveMaterials.Num(); Selection++)
	{
		if (DynamicDissolveMaterials[Selection])
		{
			DynamicDissolveMaterials[Selection]->SetScalarParameterValue("DissolveAmount", DissolveValue);

		}
	}
}

void ABasePlayerCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABasePlayerCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void ABasePlayerCharacter::SetOverlappingWeapon(AWeaponBase* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickUpWidget(false);
	}
	OverlappingWeapon = Weapon;

	/* Ensure the widget is displayed if the server is also a player*/
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickUpWidget(true);
		}
	}
}

bool ABasePlayerCharacter::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent->EquippedWeapon);
}

bool ABasePlayerCharacter::IsAiming()
{
	return (CombatComponent && CombatComponent->bAiming);
}

AWeaponBase* ABasePlayerCharacter::GetEquippedWeapon()
{
	if (CombatComponent == nullptr) return nullptr;

	return CombatComponent->EquippedWeapon;
	
}

FVector ABasePlayerCharacter::GetHitTarget() const
{
	if (CombatComponent) return CombatComponent->HitTarget;
	return FVector();
}

ECombatState ABasePlayerCharacter::GetCombatState() const
{
	if (CombatComponent == nullptr) return ECombatState::ECS_Max;
		
	return CombatComponent->CombatState;
}

bool ABasePlayerCharacter::IsLocallyRelaoding()
{
	if (CombatComponent == nullptr) return false;
	return CombatComponent->bLocallyReloading;
}
