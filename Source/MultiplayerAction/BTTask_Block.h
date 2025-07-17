#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Block.generated.h"

UCLASS()
class MULTIPLAYERACTION_API UBTTask_Block : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_Block();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};

