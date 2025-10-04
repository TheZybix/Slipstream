// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "WeaponMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Slipstream/Characters/BasePlayerCharacter.h"
#include "Slipstream/Components/LagCompensationComponent.h"
#include "Slipstream/PlayerController/BasePlayerController.h"

AProjectileBullet::AProjectileBullet()
{
	WeaponMovementComponent = CreateDefaultSubobject<UWeaponMovementComponent>(FName("WeaponMovementComponent"));
	WeaponMovementComponent->bRotationFollowsVelocity = true;
	WeaponMovementComponent->SetIsReplicated(true);
	WeaponMovementComponent->InitialSpeed = InitialSpeed;
	WeaponMovementComponent->MaxSpeed = InitialSpeed;
}

#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	FName PropertyName = PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))
	{
		if (WeaponMovementComponent)
		{
			WeaponMovementComponent->InitialSpeed = InitialSpeed;
			WeaponMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}
#endif

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	/* FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithChannel = true;
	PathParams.bTraceWithCollision = true;
	PathParams.DrawDebugTime = 5.f;
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	PathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
	PathParams.MaxSimTime = 4.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.SimFrequency = 30.f;
	PathParams.StartLocation = GetActorLocation();
	PathParams.TraceChannel = ECC_Visibility;
	PathParams.ActorsToIgnore.Add(this);
	
	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult); */
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner())
	{
		return;
	}
	
	ABasePlayerCharacter* OwnerCharacter = Cast<ABasePlayerCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		ABasePlayerController* OwnerController = Cast<ABasePlayerController>(OwnerCharacter->Controller);
		if (OwnerController)
		{
			if (OwnerCharacter->HasAuthority() && !bUseServerSideRewind)
			{
				float DamageToCause = Damage;
				if (Hit.BoneName.ToString() == FString("head"))
				{
					DamageToCause = HeadshotDamage;
				}
				UGameplayStatics::ApplyDamage(OtherActor, DamageToCause, OwnerController, this, UDamageType::StaticClass());
				Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
				return;
			}

			ABasePlayerCharacter* HitCharacter = Cast<ABasePlayerCharacter>(OtherActor);
			if (bUseServerSideRewind && OwnerCharacter->GetLagCompensation() && OwnerCharacter->IsLocallyControlled() && HitCharacter)
			{
				OwnerCharacter->GetLagCompensation()->ServerProjectileScoreRequest(HitCharacter, TraceStart, InitialVelocity, OwnerController->GetServerTime() - OwnerController->SingleTripTime);
			}
		}
	}
	
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
