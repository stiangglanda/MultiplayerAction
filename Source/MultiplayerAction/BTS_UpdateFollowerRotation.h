#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTS_UpdateFollowerRotation.generated.h"

UCLASS()
class MULTIPLAYERACTION_API UBTS_UpdateFollowerRotation : public UBTService_BlackboardBase
{
	GENERATED_BODY()
public:
    UBTS_UpdateFollowerRotation();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Blackboard)
    FName PatrolLeaderKey = "PatrolLeader";

    // How quickly the follower should turn to face the leader's direction.
    // Higher values mean faster turning.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float InterpSpeed = 1.0f;
};