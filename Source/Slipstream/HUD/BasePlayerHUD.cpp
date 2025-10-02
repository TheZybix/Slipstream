// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlayerHUD.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlay.h"
#include "Announcement.h"
#include "ElimAnnouncement.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/CanvasPanelSlot.h"

void ABasePlayerHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ABasePlayerHUD::AddCharacterOverlay()
{
	APlayerController* PlayerContoller = GetOwningPlayerController();
	if (PlayerContoller && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerContoller, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void ABasePlayerHUD::AddAnnouncement()
{
	APlayerController* PlayerContoller = GetOwningPlayerController();
	if (PlayerContoller && AnnouncementClass && Announcement == nullptr)
	{
		Announcement = CreateWidget<UAnnouncement>(PlayerContoller, AnnouncementClass);
		Announcement->AddToViewport();
	}
}

void ABasePlayerHUD::AddElimAnnouncement(FString EliminationText)
{
	OwningPlayer = OwningPlayer == nullptr ? GetOwningPlayerController() : OwningPlayer;
	
	if (OwningPlayer && ElimAnnouncementClass)
	{
		UElimAnnouncement* ElimAnnouncement = CreateWidget<UElimAnnouncement>(OwningPlayer, ElimAnnouncementClass);
		
		if (ElimAnnouncement)
		{
			ElimAnnouncement->ElimText->SetText(FText::FromString(EliminationText));
			ElimAnnouncement->AddToViewport();
			for (UElimAnnouncement* Msg : ElimMessages)
			{
				if (Msg && Msg->AnnouncementBox)
				{
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox);
					if (CanvasSlot)
					{
						FVector2D Position = CanvasSlot->GetPosition();
						FVector2D NewPosition(CanvasSlot->GetPosition().X, Position.Y + CanvasSlot->GetSize().Y * 1.25);
						CanvasSlot->SetPosition(NewPosition);
					}
				}
			}
			ElimMessages.Add(ElimAnnouncement);
			FTimerHandle ElimMsgTimer;
			FTimerDelegate ElimMsgDelegate;
			ElimMsgDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"),ElimAnnouncement);
			GetWorldTimerManager().SetTimer( ElimMsgTimer, ElimMsgDelegate,ElimAnnouncementTime,false	);
		}
	}
}

void ABasePlayerHUD::ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove)
{
	if (MsgToRemove)
	{
		MsgToRemove->RemoveFromParent();
	}
}

void ABasePlayerHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter = ViewportSize * 0.5f;

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairsCenter)
		{
			FVector2D Spread (0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsLeft)
		{
			FVector2D Spread (-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsRight)
		{
			FVector2D Spread (SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsTop)
		{
			FVector2D Spread (0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsBottom)
		{
			FVector2D Spread (0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
	}
}

void ABasePlayerHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(ViewportCenter.X - (TextureWidth * 0.5f) + Spread.X, ViewportCenter.Y - (TextureHeight * 0.5f) + Spread.Y);

	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight, 0.f, 0.f, 1.f, 1.f, CrosshairColor);
}
