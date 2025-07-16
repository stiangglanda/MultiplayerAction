#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_FindFleeLocation.generated.h"

UCLASS()
class MULTIPLAYERACTION_API UBTTask_FindFleeLocation : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_FindFleeLocation();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(Category = "AI|Flee", EditAnywhere)
	float FleeDistance = 1500.0f;

	UPROPERTY(Category = "Blackboard", EditAnywhere)
	FBlackboardKeySelector TargetToFleeFromKey;

	UPROPERTY(Category = "Blackboard", EditAnywhere)
	FBlackboardKeySelector FleeLocationKey;
};