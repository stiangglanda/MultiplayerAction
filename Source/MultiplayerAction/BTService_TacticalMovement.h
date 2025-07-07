#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_TacticalMovement.generated.h"

UCLASS()
class MULTIPLAYERACTION_API UBTService_TacticalMovement : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_TacticalMovement();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, Category = "AI")
    float CirclingRadius = 300.0f; // Distance to maintain while circling

    UPROPERTY(EditAnywhere, Category = "AI")
    float CirclingSpeed = 100.0f; // How fast to move while circling

    UPROPERTY(EditAnywhere, Category = "AI")
    float MinApproachDistance = 200.0f; // Minimum distance to keep from player

    UPROPERTY(EditAnywhere, Category = "AI")
    float MaxApproachDistance = 400.0f; // Maximum distance before approaching player

    UPROPERTY(EditAnywhere, Category = "AI")
    FBlackboardKeySelector TargetKey; // Key for the target actor (player)

private:
    float CurrentCirclingAngle = 0.0f;
};