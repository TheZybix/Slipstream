// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponEquipped, AActor*, Actor);

enum class EWeaponType : uint8;
class ABasePlayerCharacter;
class ABasePlayerController;
class USkeletalMeshComponent;
class USphereComponent;
class UWidgetComponent;
class UAnimationAsset;
class ABulletSleeve;
class UTexture2D;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Secondary UMETA(DisplayName = "Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_Max UMETA(DisplayName = "DefaultMax")
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "HitScan"),
	EFT_Projectile UMETA(DisplayName = "Projectile"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun"),
	EFT_Max UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class SLIPSTREAM_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeaponBase();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	void ShowPickUpWidget(bool bShowWidget);
	virtual void Fire(const FVector& HitTarget);
	void Dropped();
	void SetHUDAmmo();
	void SetHUDStoredAmmo();
	void AddAmmo(int32 AmmoToAdd);
	FVector TraceEndWithScatter(const FVector& HitTarget);

	/* Textures for Weaponcrosshairs */
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairsCenter;
	
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairsRight;
	
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairsBottom;

	/* Automatic fire */
	UPROPERTY(EditAnywhere, Category = "Combat")
	float FireDelay = .15f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bAutomatic = true;

	UPROPERTY(EditAnywhere, Category = "Combat")
	USoundBase* EquipSound;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName RightHandSocket;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName LeftHandSocket;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName BackSocket;

	/* Enable or disable custom depth for weapon outlines */
	void EnableCustomDepth(bool bEnable);

	bool bDestroyWeapon = false;

	UPROPERTY()
	FOnWeaponEquipped WeaponEquipped;

	UPROPERTY(EditAnywhere)
	EFireType FireType;
	
	UPROPERTY(EditAnywhere, Category = "Scatter")
	bool bUseScatter = false;
	
protected:
	virtual void BeginPlay() override;

	virtual void OnWeaponStateSet();
	virtual void HandleOnEquipped();
	virtual void HandleOnDropped();
	virtual void HandleOnEquippedSecondary();

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,	UPrimitiveComponent* OtherComp,	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,	UPrimitiveComponent* OtherComp,	int32 OtherBodyIndex);

	UPROPERTY()
	ABasePlayerCharacter* OwnerCharacter;

	UPROPERTY()
	ABasePlayerController* OwnerController;

	UPROPERTY(EditAnywhere)
	float Damage = 15.f;
	
	UPROPERTY(EditAnywhere)
	float HeadshotDamage = 15.f;

	/* Trace end with scatter */
	UPROPERTY(EditAnywhere, Category = "Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Scatter")
	float SphereRadius = 800.f;

	UPROPERTY(EditAnywhere, Replicated)
	bool bUseServerSideRewind = false;

	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);

private:
	UPROPERTY(VisibleAnywhere, Category = "WeaponProperties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "WeaponProperties")
	USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "WeaponProperties")
	EWeaponState WeaponState = EWeaponState::EWS_Initial;

	UFUNCTION()
	void OnRep_WeaponState();
	void ClearDestroyTimer();

	UPROPERTY(VisibleAnywhere, Category = "WeaponProperties")
	UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties")
	UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties")
	float WeaponSpread = 0.75f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABulletSleeve> BulletSleeveClass;

	/* Zoomed FOV while aiming */
	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere)
	int32 Ammo;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_StoredAmmo)
	int32 StoredAmmo = 10;

	UPROPERTY(EditAnywhere)
	int32 StoredMaxAmmo = 10;

	UFUNCTION()
	void OnRep_StoredAmmo();

	void SpendRound();

	UFUNCTION(Reliable, Client)
	void ClientUpdateAmmo(int32 ServerAmmo);

	//Number of unprocessed server requests for Ammo
	//Incremented in spend round, decremented in client update ammo
	int32 Sequence = 0;

	UFUNCTION(Reliable, Client)
	void ClientAddAmmo(int32 AmmoToAdd);

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

	/* Destroy weapon after it was dropped and not picked up again */
	void StartDestroyTimer();
	void DestroyTimerFinished();
	
	FTimerHandle DestroyTimer;
	
	UPROPERTY(EditAnywhere)
	float DestroyTime;
	
public:
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() {return AreaSphere;}
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const {return WeaponMesh;}
	FORCEINLINE float GetZoomedFOV() const {return ZoomedFOV;}
	FORCEINLINE float GetZoomInterpSpeed() const {return ZoomInterpSpeed;}
	FORCEINLINE float GetWeaponBulletSpread() const { return WeaponSpread;}
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetStoredAmmo() const { return StoredAmmo; }
	FORCEINLINE void SetStoredAmmo(int32 NewStoredAmmo) { StoredAmmo = NewStoredAmmo; }
	FORCEINLINE int32 GetMagAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE int32 GetStoredMaxAmmo() const { return StoredMaxAmmo; }
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadshotDamage() const { return HeadshotDamage; }
	bool IsEmpty();
	bool IsFull();
};