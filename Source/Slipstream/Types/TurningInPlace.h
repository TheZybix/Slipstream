#pragma once


UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
	ETIP_TurningRight UMETA(DisplayName = "TurningRight"),
	ETIP_TurningLeft UMETA(DisplayName = "TurningLeft"),
	ETIP_NotTurning UMETA(DisplayName = "NotTurning"),
	ETIP_MAX UMETA(DisplayName = "Max")
};