#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_SetMovementSpeed.generated.h"

UCLASS()
class MULTIPLAYERACTION_API UBTT_SetMovementSpeed : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UBTT_SetMovementSpeed();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float NewSpeed = 150.0f;
};
