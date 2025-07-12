#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTT_FindFormationSlotLocation.generated.h"

UCLASS()
class MULTIPLAYERACTION_API UBTT_FindFormationSlotLocation : public UBTTask_BlackboardBase
{
    GENERATED_BODY()

public:
    UBTT_FindFormationSlotLocation();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Blackboard)
    FName PatrolLeaderKey = "PatrolLeader";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Blackboard)
    FName FormationOffsetKey = "FormationOffset";
};