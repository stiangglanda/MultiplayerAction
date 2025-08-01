#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTT_GetPatrolPointLocation.generated.h"

UCLASS()
class MULTIPLAYERACTION_API UBTT_GetPatrolPointLocation : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTT_GetPatrolPointLocation();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
