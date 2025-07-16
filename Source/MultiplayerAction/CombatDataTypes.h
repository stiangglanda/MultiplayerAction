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