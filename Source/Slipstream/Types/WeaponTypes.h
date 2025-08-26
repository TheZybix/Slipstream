#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),
	EWT_SMG UMETA(DisplayName = "SMG"),
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	EWT_Max UMETA(DisplayName = "DefaultMax")
};