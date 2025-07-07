#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_IncrementPatrolIndex.generated.h"


UCLASS()
class MULTIPLAYERACTION_API UBTT_IncrementPatrolIndex : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTT_IncrementPatrolIndex();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
