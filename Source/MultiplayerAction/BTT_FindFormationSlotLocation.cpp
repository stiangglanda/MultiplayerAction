#include "BTT_FindFormationSlotLocation.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTT_FindFormationSlotLocation::UBTT_FindFormationSlotLocation()
{
    NodeName = "Find Formation Slot Location";
}

EBTNodeResult::Type UBTT_FindFormationSlotLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        UE_LOG(LogTemp, Error, TEXT("BTT_FindFormationSlotLocation: Blackboard Component not found!"));
        return EBTNodeResult::Failed;
    }

    AActor* LeaderActor = Cast<AActor>(BlackboardComp->GetValueAsObject(PatrolLeaderKey));
    if (!LeaderActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("BTT_FindFormationSlotLocation: Leader Actor not found for key '%s'."), *PatrolLeaderKey.ToString());
        return EBTNodeResult::Failed;
    }

    FVector FormationOffset = BlackboardComp->GetValueAsVector(FormationOffsetKey);
    if (FormationOffset.IsNearlyZero() && !BlackboardComp->IsVectorValueSet(FormationOffsetKey))
    {
        UE_LOG(LogTemp, Warning, TEXT("BTT_FindFormationSlotLocation: Formation Offset not found or invalid for key '%s'."), *FormationOffsetKey.ToString());
        return EBTNodeResult::Failed;
    }

    FVector TargetLocation = LeaderActor->GetActorTransform().TransformPosition(FormationOffset);

    BlackboardComp->SetValueAsVector(GetSelectedBlackboardKey(), TargetLocation);

    return EBTNodeResult::Succeeded;
}