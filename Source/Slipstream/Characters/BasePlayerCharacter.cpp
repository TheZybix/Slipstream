// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Slipstream/Components/CombatComponent.h"
#include "Slipstream/Weapon/WeaponBase.h"


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
}

void ABasePlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABasePlayerCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void ABasePlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComponent) CombatComponent->Character = this;
}


void ABasePlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitializeMappingContext();
}

void ABasePlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABasePlayerCharacter::InitializeMappingContext()
{
	TObjectPtr<APlayerController> PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
}

void ABasePlayerCharacter::Move(const FInputActionValue& Value)
{
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

void ABasePlayerCharacter::Jump(const FInputActionValue& Value)
{
	Super::Jump();
}

void ABasePlayerCharacter::EquipKeyPressed(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		if (HasAuthority())	CombatComponent->EquipWeapon(OverlappingWeapon);
		else ServerEquipKeyPressed();
	}
}

void ABasePlayerCharacter::ServerEquipKeyPressed_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}

void ABasePlayerCharacter::Crouch(const FInputActionValue& Value)
{
	
}

void ABasePlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if(UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::Jump);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::EquipKeyPressed);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::Crouch);
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


