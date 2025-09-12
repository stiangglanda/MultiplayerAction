#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "CombatDataTypes.generated.h"

UENUM(BlueprintType)
enum class EAttackType : uint8
{
	EAT_None		UMETA(DisplayName = "None"),
	EAT_Regular		UMETA(DisplayName = "Regular Attack"),
	EAT_Heavy		UMETA(DisplayName = "Heavy Attack")
};

UENUM(BlueprintType)
enum class EImpactType : uint8
{
	EIT_Flesh		UMETA(DisplayName = "Flesh"),
	EIT_Stone		UMETA(DisplayName = "Stone"),
	EIT_Metal		UMETA(DisplayName = "Metal")
};