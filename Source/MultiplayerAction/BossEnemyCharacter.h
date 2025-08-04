#pragma once

#include "CoreMinimal.h"
#include "MultiplayerActionCharacter.h"
#include "BossEnemyCharacter.generated.h"

UCLASS()
class MULTIPLAYERACTION_API ABossEnemyCharacter : public AMultiplayerActionCharacter
{
	GENERATED_BODY()
public:
	ABossEnemyCharacter()
	{
		bAlwaysRelevant = true;
		NetUpdateFrequency = 100.0f;
	}

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PerformWeaponTrace() override;

	void ApplyKnockbackToPlayer(ACharacter* PlayerToLaunch, const FHitResult& HitResult);

	UFUNCTION()
	void OnChestUnlockingStarted();

	UPROPERTY(Category = "Combat|Knockback", EditAnywhere, BlueprintReadWrite)
	float KnockbackStrength = 1000.0f;

	UPROPERTY(Category = "Combat|Knockback", EditAnywhere, BlueprintReadWrite)
	float KnockbackUpwardForce = 500.0f;

	UPROPERTY(EditAnywhere)
	float BossDamage = 50;

	UPROPERTY(EditAnywhere)
	float BossHeavyDamage = 60;
};
