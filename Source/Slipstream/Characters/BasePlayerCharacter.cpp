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
#include "Slipstream/Slipstream.h"
#include "Slipstream/GameModes/SlipstreamGameMode.h"
#include "Slipstream/PlayerController/BasePlayerController.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
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

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	CombatComponent->SetIsReplicated(true);

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
}

void ABasePlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABasePlayerCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABasePlayerCharacter, Health);
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
		MontageToPlay = AssaultRifleMontage;
		break;
	case EWeaponType::EWT_Pistol:
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

void ABasePlayerCharacter::Elim()
{
	MulticastElim();
	GetWorldTimerManager().SetTimer(ElimTimer, this, &ABasePlayerCharacter::ElimTimerFinished, ElimDelay);
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->EquippedWeapon->Dropped();
	}
}

void ABasePlayerCharacter::MulticastElim_Implementation()
{
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
}

void ABasePlayerCharacter::ElimTimerFinished()
{
	ASlipstreamGameMode* GameMode = GetWorld()->GetAuthGameMode<ASlipstreamGameMode>();
	if (GameMode)
	{
		GameMode->RequestRespawn(this, Controller);
	}
}

void ABasePlayerCharacter::PlayHitReactMontage()
{
	if (!CombatComponent || !CombatComponent->EquippedWeapon) return;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		
		FName SectionName = FName("FromFront");
		/*SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");*/
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABasePlayerCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();

	if (Health == 0.f)
	{
		ASlipstreamGameMode* SlipstreamGameMode = GetWorld()->GetAuthGameMode<ASlipstreamGameMode>();
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


void ABasePlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	UpdateHUDHealth();
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABasePlayerCharacter::ReceiveDamage);
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

	ASlipstreamGameMode* SlipstreamGame = Cast<ASlipstreamGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = SlipstreamGame && SlipstreamGame->GetMatchState() != MatchState::InProgress;
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
		if (HasAuthority())	CombatComponent->EquipWeapon(OverlappingWeapon);
		else ServerEquipKeyPressed();
	}
}

void ABasePlayerCharacter::ServerEquipKeyPressed_Implementation()
{
	if (bDisableGameplay) return;
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
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


void ABasePlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if(UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::JumpKeyPressed);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::EquipKeyPressed);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::CrouchKeyPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::AimKeyPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ABasePlayerCharacter::AimKeyPressed);
		EnhancedInputComponent->BindAction(TriggerAction, ETriggerEvent::Started, this, &ABasePlayerCharacter::TriggerKeyPressed);
		EnhancedInputComponent->BindAction(TriggerAction, ETriggerEvent::Completed, this, &ABasePlayerCharacter::TriggerKeyPressed);
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::ReloadKeyPressed);
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

void ABasePlayerCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();
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