#pragma once

#include "CoreMinimal.h"
#include "MultiplayerActionCharacter.h"
#include "DefaultAIController.h"
#include "BossEnemyCharacter.generated.h"

UCLASS()
class MULTIPLAYERACTION_API ABossEnemyCharacter : public AMultiplayerActionCharacter
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	virtual void PerformWeaponTrace() override;

	void ApplyKnockbackToPlayer(ACharacter* PlayerToLaunch, const FHitResult& HitResult);

	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult) override;

	UPROPERTY(Category = "Combat|Knockback", EditAnywhere, BlueprintReadWrite)
	float KnockbackStrength = 1000.0f;

	UPROPERTY(Category = "Combat|Knockback", EditAnywhere, BlueprintReadWrite)
	float KnockbackUpwardForce = 500.0f;
	
	bool bIsAwake = false;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Wake Up")
	TObjectPtr<UAnimMontage> WakeUpMontage;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Wake Up")
	TSubclassOf<ADefaultAIController> DefaultAIControllerClass;
};
