#pragma once

#include "CoreMinimal.h"
#include "MultiplayerActionCharacter.h"
#include "BossEnemyCharacter.generated.h"

UCLASS()
class MULTIPLAYERACTION_API ABossEnemyCharacter : public AMultiplayerActionCharacter
{
	GENERATED_BODY()

protected:
	virtual void PerformWeaponTrace() override;

	void ApplyKnockbackToPlayer(ACharacter* PlayerToLaunch, const FHitResult& HitResult);

	UPROPERTY(Category = "Combat|Knockback", EditAnywhere, BlueprintReadWrite)
	float KnockbackStrength = 1000.0f;

	UPROPERTY(Category = "Combat|Knockback", EditAnywhere, BlueprintReadWrite)
	float KnockbackUpwardForce = 500.0f;
};
