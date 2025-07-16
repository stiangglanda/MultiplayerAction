#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "BTDecorator_IsHealthLow.generated.h"

UCLASS()
class MULTIPLAYERACTION_API UBTDecorator_IsHealthLow : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTDecorator_IsHealthLow();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

	UPROPERTY(Category = "AI|Condition", EditAnywhere)
	float HealthThreshold = 0.3f;
};
