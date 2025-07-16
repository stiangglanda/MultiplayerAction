#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_ChooseAttack.generated.h"

UCLASS()
class MULTIPLAYERACTION_API UBTTask_ChooseAttack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_ChooseAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

	/** The blackboard key to write the chosen attack type to. This should be an Enum key. */
	UPROPERTY(Category = "Blackboard", EditAnywhere, meta = (AllowPrivateAccess = "true"))
	FBlackboardKeySelector ChosenAttackTypeKey;

	/** The chance (from 0.0 to 1.0) that the AI will choose a heavy attack. */
	UPROPERTY(Category = "AI|Attack Logic", EditAnywhere, meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float HeavyAttackChance = 0.3f;
};
