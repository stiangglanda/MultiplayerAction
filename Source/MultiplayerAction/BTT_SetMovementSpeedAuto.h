#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTT_SetMovementSpeedAuto.generated.h"

UCLASS()
class MULTIPLAYERACTION_API UBTT_SetMovementSpeedAuto : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
    UBTT_SetMovementSpeedAuto();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float MaxSpeed = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float MinSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float thresholdDistance = 900.0f;

    UPROPERTY(Category = "Blackboard", EditAnywhere)
    FBlackboardKeySelector PatrolLeaderKey;
};
