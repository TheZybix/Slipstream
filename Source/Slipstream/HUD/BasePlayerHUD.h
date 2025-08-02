// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BasePlayerHUD.generated.h"

/**
 * 
 */
UCLASS()
class SLIPSTREAM_API ABasePlayerHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
	
};
