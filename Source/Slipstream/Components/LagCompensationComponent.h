// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

class ABasePlayerController;
class ABasePlayerCharacter;
class AWeaponBase;

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;
	
	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	ABasePlayerCharacter* Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()
	
	UPROPERTY()
	bool bHitConfirmed;
	
	UPROPERTY()
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<ABasePlayerCharacter*, uint32> HeadShots;
	
	UPROPERTY()
	TMap<ABasePlayerCharacter*, uint32> BodyShots;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SLIPSTREAM_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend class ABasePlayerCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(FFramePackage& FramePackage, const FColor& Color);

	/* Hitscan */
	FServerSideRewindResult ServerSideRewind(ABasePlayerCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);

	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(ABasePlayerCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);

	/* Shotgun */
	FShotgunServerSideRewindResult ShotgunServerSideRewind(const TArray<ABasePlayerCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);

	UFUNCTION(Server, Reliable)
	void ServerShotgunScoreRequest(const TArray<ABasePlayerCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);

	/* Projectile */
	FServerSideRewindResult ProjectileServerSideRewind(ABasePlayerCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,const FVector_NetQuantize100& InitialVelocity, float HitTime);

	UFUNCTION(Server, Reliable)
	void ServerProjectileScoreRequest(ABasePlayerCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);


protected:
	virtual void BeginPlay() override;
	void SaveFramePackage();
	void SaveFramePackage(FFramePackage& FramePackage);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
	void CacheBoxPositions(ABasePlayerCharacter* HitCharacter, FFramePackage& Package);
	void MoveBoxes(ABasePlayerCharacter* HitCharacter, const FFramePackage& Package);
	void ResetBoxes(ABasePlayerCharacter* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(ABasePlayerCharacter* HitCharacter, ECollisionEnabled::Type NewCollisionEnabled);
	FFramePackage GetFrameToCheck(ABasePlayerCharacter* HitCharacter, float HitTime);

	/* Hitscan */
	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, ABasePlayerCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);
	
	/* Shotgun */
	FShotgunServerSideRewindResult ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations);

	/* Projectile */
	FServerSideRewindResult ProjectileConfirmHit(const FFramePackage& Package, ABasePlayerCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,const FVector_NetQuantize100& InitialVelocity, float HitTime);
	
private:
	UPROPERTY()
	ABasePlayerCharacter* Character;

	UPROPERTY()
	ABasePlayerController* PlayerController;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;
	
public:	
};
