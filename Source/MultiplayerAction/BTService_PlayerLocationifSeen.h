#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BTService_PlayerLocationifSeen.generated.h"

UCLASS()
class MULTIPLAYERACTION_API UBTService_PlayerLocationifSeen : public UBTService_BlackboardBase
{
    GENERATED_BODY()

public:
    UBTService_PlayerLocationifSeen();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, Category = "AI")
    float SightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "AI")
    float LoseSightRadius = 2000.0f;

    UPROPERTY(EditAnywhere, Category = "AI")
    float PeripheralVisionAngleDegrees = 90.0f;
};