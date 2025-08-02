// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletSleeve.h"

#include "Kismet/GameplayStatics.h"

ABulletSleeve::ABulletSleeve()
{
	PrimaryActorTick.bCanEverTick = false;

	SleeveMesh = CreateDefaultSubobject<USkeletalMeshComponent>("SleeveMesh");
	SetRootComponent(SleeveMesh);
	SleeveMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	SleeveMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	SleeveMesh->SetSimulatePhysics(true);
	SleeveMesh->SetEnableGravity(true);
	SleeveMesh->SetNotifyRigidBodyCollision(true);
	SleeveMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	EjectionImpulse = 5.f;
}


void ABulletSleeve::BeginPlay()
{
	Super::BeginPlay();
	SleeveMesh->AddImpulse(GetActorRightVector() * EjectionImpulse);
	SleeveMesh->OnComponentHit.AddDynamic(this, &ABulletSleeve::OnHit);
}

void ABulletSleeve::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShellSound)	UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	SleeveMesh->SetNotifyRigidBodyCollision(false);
	SetLifeSpan(5.f);
}

