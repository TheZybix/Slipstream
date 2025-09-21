// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

class USphereComponent;
class USoundBase;
class UStaticMeshComponent;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class SLIPSTREAM_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	APickup();

	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,	UPrimitiveComponent* OtherComp,	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;

	UPROPERTY(EditAnywhere)
	UNiagaraComponent* PickupEffectComponent;
	
	UPROPERTY(EditAnywhere)
	UNiagaraSystem* PickupEffectSystem;

private:
	UPROPERTY(EditAnywhere)
	USphereComponent* OverlapSphere;

	UPROPERTY(EditAnywhere)
	USoundBase* PickupSound;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PickupMesh;
	
public:	
	virtual void Tick(float DeltaTime) override;

};
