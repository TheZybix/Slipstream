// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Slipstream/Characters/BasePlayerCharacter.h"
#include "Slipstream/Weapon/WeaponBase.h"
#include "Slipstream/PlayerController/BasePlayerController.h"
#include "Slipstream/HUD/BasePlayerHUD.h"
#include "TimerManager.h"
#include "Slipstream/GameModes/SlipstreamGameMode.h"
#include "Slipstream/Types/WeaponTypes.h"
#include "Slipstream/Weapon/Projectile.h"
#include "Slipstream/Weapon/Shotgun.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Grenades);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		if (Character->GetCameraComponent())
		{
			DefaultFOV = Character->GetCameraComponent()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		SpawnDefaultWeapon();
		Character->UpdateHUDAmmo();
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		InterpolateFOV(DeltaTime);
	}
}

void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon) EquippedWeapon->Dropped();
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr || !EquippedWeapon || !EquippedWeapon->RightHandSocket.IsValid()) return;
	const USkeletalMeshSocket* HandSocket =  Character->GetMesh()->GetSocketByName(EquippedWeapon->RightHandSocket);
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr || !EquippedWeapon || !EquippedWeapon->LeftHandSocket.IsValid()) return;
	const USkeletalMeshSocket* HandSocket =  Character->GetMesh()->GetSocketByName(EquippedWeapon->LeftHandSocket);
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToBack(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr || !SecondaryWeapon || !SecondaryWeapon->BackSocket.IsValid()) return;
	const USkeletalMeshSocket* BackSocket =  Character->GetMesh()->GetSocketByName(SecondaryWeapon->BackSocket);
	if (BackSocket)
	{
		BackSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::PlayEquipWeaponSound(AWeaponBase* WeaponToEquip)
{
	if (WeaponToEquip && WeaponToEquip->EquipSound) UGameplayStatics::PlaySoundAtLocation(this, WeaponToEquip->EquipSound, Character->GetActorLocation());
}

void UCombatComponent::EquipWeapon(AWeaponBase* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr || CombatState != ECombatState::ECS_Unoccupied) return;

	if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
	{
		EquipSecondaryWeapon(WeaponToEquip);
	}
	else
	{
		EquipPrimaryWeapon(WeaponToEquip);
	}
	
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::EquipPrimaryWeapon(AWeaponBase* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	DropEquippedWeapon();
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);

	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();
	EquippedWeapon->SetHUDStoredAmmo();
	
	if (EquippedWeapon->IsEmpty())
	{
		Reload();
	}
	PlayEquipWeaponSound(WeaponToEquip);
	EquippedWeapon->EnableCustomDepth(false);
}

void UCombatComponent::EquipSecondaryWeapon(AWeaponBase* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_Secondary);
	AttachActorToBack(SecondaryWeapon);

	SecondaryWeapon->SetOwner(Character);
	PlayEquipWeaponSound(SecondaryWeapon);
}

void UCombatComponent::OnRep_Aiming()
{
	if (Character && Character->IsLocallyControlled())
	{
		bAiming = bAimKeyPressed;
	}
}

void UCombatComponent::SwapWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied || Character == nullptr || !Character->HasAuthority()) return;

	Character->PlaySwapWeaponMontage();
	Character->bFinishedSwapping = false;
	CombatState = ECombatState::ECS_SwappingWeapons;
	if (SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(false);
}

bool UCombatComponent::ShouldSwapWeapons()
{
	return (EquippedWeapon != nullptr && SecondaryWeapon != nullptr);
}

void UCombatComponent::Reload()
{
	if (EquippedWeapon && EquippedWeapon->GetStoredAmmo() > 0 && CombatState == ECombatState::ECS_Unoccupied && !EquippedWeapon->IsFull() && !bLocallyReloading)
	{
		ServerReload();
		HandleReload();
		bLocallyReloading = true;
	}
}

void UCombatComponent::SetCombatState(ECombatState NewCombatState)
{
	if (CombatState == ECombatState::ECS_Reloading) bLocallyReloading = false;
	if (Character == nullptr) return;
	if (CombatState == ECombatState::ECS_Reloading && Character->HasAuthority())
	{
		UpdateAmmoValues();
	}
	if (CombatState == ECombatState::ECS_SwappingWeapons)
	{
		if (SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(true);
	}
	if (Character) Character->bFinishedSwapping = true;
	if (Character->HasAuthority()) CombatState = NewCombatState;
	if (bTriggerKeyPressed) Fire();
}

void UCombatComponent::FinishSwapAttachWeapons()
{
	AWeaponBase* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;
	
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetHUDAmmo();
	EquippedWeapon->SetHUDStoredAmmo();
	PlayEquipWeaponSound(EquippedWeapon);
	
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_Secondary);
	AttachActorToBack(SecondaryWeapon);
}

void UCombatComponent::HandleReload()
{
	if (Character) Character->PlayReloadMontage();
}

void UCombatComponent::UpdateAmmoValues()
{
	if (EquippedWeapon == nullptr) return;
	int32 ReloadAmount = AmountToReload();
	int32 NewStoredAmmo = FMath::Clamp(EquippedWeapon->GetStoredAmmo() - ReloadAmount, 0,EquippedWeapon->GetStoredAmmo());
	EquippedWeapon->SetStoredAmmo(NewStoredAmmo);
	EquippedWeapon->AddAmmo(-ReloadAmount);
	PlayerController = PlayerController == nullptr ? Cast<ABasePlayerController>(Character->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDStoredAmmo(NewStoredAmmo);
		PlayerController->SetHUDWeaponAmmo(EquippedWeapon->GetMagAmmo());
	}
}

void UCombatComponent::UpdateShotgunAmmoValues()
{
	if (EquippedWeapon == nullptr) return;
	int32 NewStoredAmmo = FMath::Clamp(EquippedWeapon->GetStoredAmmo() - 1, 0,EquippedWeapon->GetStoredAmmo());

	EquippedWeapon->SetStoredAmmo(NewStoredAmmo);
	EquippedWeapon->AddAmmo(-1);
	bCanFire = true;
	
	PlayerController = PlayerController == nullptr ? Cast<ABasePlayerController>(Character->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDStoredAmmo(NewStoredAmmo);
		PlayerController->SetHUDWeaponAmmo(EquippedWeapon->GetMagAmmo());
	}
	if (EquippedWeapon->IsFull() || EquippedWeapon->GetStoredAmmo() == 0)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::ThrowGrenade()
{
	if (Grenades <= 0) return;
	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	if (Character && !Character->HasAuthority()) ServerThrowGrenade();
	if (Character && Character->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateHUDGrenades();
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if (Grenades <= 0) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;
	
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
	UpdateHUDGrenades();
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	
	CombatState = ECombatState::ECS_Reloading;
	if (!Character->IsLocallyControlled()) HandleReload();
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
		case ECombatState::ECS_Reloading:
			if (Character &&!Character->IsLocallyControlled()) HandleReload();
			break;
		case ECombatState::ECS_Unoccupied:
			if (bTriggerKeyPressed)
			{
				Fire();
			}
		case ECombatState::ECS_ThrowingGrenade:
			{
				if (Character && !Character->IsLocallyControlled())
				{
					Character->PlayThrowGrenadeMontage();
					AttachActorToLeftHand(EquippedWeapon);
					ShowAttachedGrenade(true);
				}
			}
			break;
	case ECombatState::ECS_SwappingWeapons:
		if (Character && !Character->IsLocallyControlled())
		{
			Character->PlaySwapWeaponMontage();
		}
		break;
	}
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetMagAmmo();
	int32 AmountCarried = EquippedWeapon->GetStoredAmmo();
	int32 Least = FMath::Min(RoomInMag, AmountCarried);
	return FMath::Clamp(RoomInMag, 0, Least);
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character && Character->GetGrenadeMesh()) Character->GetGrenadeMesh()->SetVisibility(bShowGrenade);
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenades();
}

void UCombatComponent::UpdateHUDGrenades()
{
	PlayerController = PlayerController == nullptr ? Cast<ABasePlayerController>(Character->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDGrenades(Grenades);
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if (Character) Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle) Character->ShowSniperScopeWidget(bAiming);
	if (Character->IsLocallyControlled()) bAimKeyPressed = bIsAiming;
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character) Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && EquippedWeapon->RightHandSocket.IsValid() && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		PlayEquipWeaponSound(EquippedWeapon);
		EquippedWeapon->EnableCustomDepth(false);
		EquippedWeapon->SetHUDAmmo();
		EquippedWeapon->SetHUDStoredAmmo();
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_Secondary);
		AttachActorToBack(SecondaryWeapon);
		PlayEquipWeaponSound(SecondaryWeapon);
	}
}

void UCombatComponent::TriggerKeyPressed(bool bPressed)
{
	bTriggerKeyPressed = bPressed;
	if (bTriggerKeyPressed)
	{
		Fire();
	}
}

void UCombatComponent::ShotgunShellReload()
{
	if (Character && Character->HasAuthority())	UpdateShotgunAmmoValues();
}

void UCombatComponent::ThrowGrenadeFinished()
{
	if (!EquippedWeapon) return;
	AttachActorToRightHand(EquippedWeapon);
	ShowAttachedGrenade(false);

	if (Character && Character->IsLocallyControlled()) ServerThrowGrenadeFinished(HitTarget);
}

void UCombatComponent::ServerThrowGrenadeFinished_Implementation(const FVector_NetQuantize& Target)
{
	if (Character && Character->GetGrenadeMesh() && GrenadeClass)
	{
		const FVector StartingLocation = Character->GetGrenadeMesh()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		if (UWorld* World = GetWorld())
		{
			World->SpawnActor<AProjectile>(GrenadeClass, StartingLocation, ToTarget.Rotation(), SpawnParams);
		}
	}
}

void UCombatComponent::JumpToShotgunEnd()
{
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance && Character->GetShotgunMontage())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}

void UCombatComponent::PickupAmmo(TMap<EWeaponType, int32> Ammo)
{
	if (EquippedWeapon)
	{
		int32 AmmoAmount = Ammo[EquippedWeapon->GetWeaponType()];
		EquippedWeapon->SetStoredAmmo(FMath::Clamp(EquippedWeapon->GetStoredAmmo() + AmmoAmount, 0, EquippedWeapon->GetStoredMaxAmmo()));
		EquippedWeapon->SetHUDStoredAmmo();
		PlayerController = PlayerController == nullptr ? Cast<ABasePlayerController>(Character->Controller) : PlayerController;
		
		if (EquippedWeapon && EquippedWeapon->IsEmpty())
		{
			Reload();
		}
	}
}

void UCombatComponent::SpawnDefaultWeapon()
{
	ASlipstreamGameMode* SlipstreamGameMode = Cast<ASlipstreamGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	if (SlipstreamGameMode && World && Character && !Character->IsDead() && DefaultWeapon)
	{
		AWeaponBase* StartingWeapon = World->SpawnActor<AWeaponBase>(DefaultWeapon);
		StartingWeapon->bDestroyWeapon = true;
		EquipWeapon(StartingWeapon);
	}
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(FireTimer, this, &UCombatComponent::FireTimerFinished, EquippedWeapon->FireDelay);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;
	bCanFire = true;
	if (bTriggerKeyPressed && EquippedWeapon->bAutomatic) Fire();
	if (EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::Fire()
{
	if (CanFire())
	{
		bCanFire = false;
		
		if (EquippedWeapon)
		{
			CrosshairShootingFactor += EquippedWeapon->GetWeaponBulletSpread();
			CrosshairShootingFactor = FMath::Clamp(CrosshairShootingFactor, 0.f, 20.f);

			switch (EquippedWeapon->FireType)
			{
				case EFireType::EFT_Projectile:
					FireProjectileWeapon();
					break;

				case EFireType::EFT_HitScan:
					FireHitscanWeapon();
					break;
				
				case EFireType::EFT_Shotgun:
					FireShotgun();
					break;
			}
		}
		StartFireTimer();
	}
}

void UCombatComponent::FireProjectileWeapon()
{
	if (EquippedWeapon && Character)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if (!Character->HasAuthority()) LocalFire(HitTarget);
		ServerFire(HitTarget, EquippedWeapon->FireDelay);
	}
}

void UCombatComponent::FireHitscanWeapon()
{
	if (EquippedWeapon && Character)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if (!Character->HasAuthority()) LocalFire(HitTarget);
		ServerFire(HitTarget, EquippedWeapon->FireDelay);
	}
}

void UCombatComponent::FireShotgun()
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun && Character)
	{
		TArray<FVector_NetQuantize> HitTargets;
		Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);
		if (!Character->HasAuthority()) LocalFireShotgun(HitTargets);
		ServerShotgunFire(HitTargets, EquippedWeapon->FireDelay);
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	MulticastFire(TraceHitTarget);
}

bool UCombatComponent::ServerFire_Validate(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	if (EquippedWeapon)
	{
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	MulticastShotgunFire(TraceHitTargets);	
}

bool UCombatComponent::ServerShotgunFire_Validate(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	if (EquippedWeapon)
	{
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalFireShotgun(TraceHitTargets);
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::LocalFireShotgun(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun == nullptr || Character == nullptr) return;
	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied)
	{
		bLocallyReloading = false;
		Character->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(TraceHitTargets);
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalFire(TraceHitTarget);
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);
	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;
		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation()-Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 25.f);
			//DrawDebugSphere(GetWorld(), Start, 50.f, 12, FColor::Red, false);
		}
		
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;
		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility);
		if (!TraceHitResult.bBlockingHit) TraceHitResult.ImpactPoint = End;

		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairColor = FLinearColor::Red;
			bIsTargeting = true;
		}
		else
		{
			HUDPackage.CrosshairColor = FLinearColor::White;
			bIsTargeting = false;
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;
	PlayerController = PlayerController == nullptr ? Cast<ABasePlayerController>(Character->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerHUD = PlayerHUD == nullptr ? Cast<ABasePlayerHUD>(PlayerController->GetHUD()) : PlayerHUD;
		if (PlayerHUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}

			/* Calculate Crosshair spread */
			FVector2D WalkingSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplier(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkingSpeedRange, VelocityMultiplier, Velocity.Size());

			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 20.f);
			}

			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, -1.25f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}
			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 5.f);

			if (bIsTargeting)
			{
				CrosshairTargetFactor = FMath::FInterpTo(CrosshairTargetFactor, -1.f, DeltaTime, 10.f);
			}
			else CrosshairTargetFactor = FMath::FInterpTo(CrosshairTargetFactor, 0.f, DeltaTime, 15.f);
			
			float CrosshairSpread = 0.25f + CrosshairVelocityFactor + CrosshairInAirFactor + CrosshairAimFactor + CrosshairShootingFactor + CrosshairTargetFactor;
			float CrosshairSpreadClamped = FMath::Clamp(CrosshairSpread, 0.f, CrosshairSpread);
			HUDPackage.CrosshairSpread = CrosshairSpreadClamped;
			PlayerHUD->SetHUDPackage(HUDPackage);
		}
	}
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;
	if (!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun) return true;
	if (bLocallyReloading) return false;
	return bCanFire && !EquippedWeapon->IsEmpty() && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::InterpolateFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;

	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomedInterpSpeed);
	}
	if (Character && Character->GetCameraComponent())
	{
		Character->GetCameraComponent()->SetFieldOfView(CurrentFOV);
	}
}